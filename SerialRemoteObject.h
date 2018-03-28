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
	// clear input buffer
	while (rs485->available()) { rs485->read(); }

	// send payload
	size_t size = buf->available();
	rs485->setBatchWriteMode(true);
	rs485->write((size >> 8) & 0xFF);
	rs485->write(size & 0xFF);
	buf->flush(rs485);
	rs485->setBatchWriteMode(false);

	// read response
	buf->clear();
	size = ((rs485->timedRead() & 0xFF) << 8) | (rs485->timedRead() & 0xFF);
	size_t n = rs485->readBytesUntil(EOT, buf->getBuffer(), buf->capacity());
	if (n > 0) {
		buf->setSize(n);
		buf->write(EOT);
	}

//	debug_printBuffer(buf->getBuffer(), buf->available());

	return buf;
}

#endif /* SPIREMOTEOBJECT_H_ */
