#ifndef SPIREMOTEOBJECT_H_
#define SPIREMOTEOBJECT_H_

#include <Arduino.h>
#include <HardwareSerial.h>
#include <RemoteObject.h>
#include <RemoteObjectProtocol.h>
#include <RS485Serial.h>
#include <SmartBuffer.h>

class RS485Serial;

class SerialRemoteObject: public RemoteObject {
public:

	SerialRemoteObject(const char* nodeIdPSTR, const char* objectNamePSTR, const char* securityTokenPSTR, RS485Serial* rs485) :
			RemoteObject(nodeIdPSTR, objectNamePSTR, securityTokenPSTR) {
		this->rs485 = rs485;
	}

	Stream* sendRequest(SmartBuffer* buf);

protected:

	RS485Serial* rs485;

};

Stream* SerialRemoteObject::sendRequest(SmartBuffer* buf) {
	// send payload
	size_t size = buf->available();
	rs485->setBatchWriteMode(true);
	rs485->write((size >> 8) & 0xFF);
	rs485->write(size & 0xFF);
	buf->flush(rs485);
	rs485->setBatchWriteMode(false);

	// read response
//	Serial.println("Reading response");
	uint8_t c;
	long t = millis();
	buf->clear();
	t = millis();

	size = ((rs485->timedRead() & 0xFF) << 8) | (rs485->timedRead() & 0xFF);
	do {
		c = rs485->timedRead();
		buf->write(c);
	} while (c != EOT && millis() - t < 1000);


//	for (int i = 0; i < buf->available(); i++) {
//		Serial.print((char) (buf->getBuffer()[i]));
//	}

	return buf;
}

#endif /* SPIREMOTEOBJECT_H_ */
