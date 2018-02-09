#ifndef SPIREMOTEOBJECT_H_
#define SPIREMOTEOBJECT_H_

#include <Arduino.h>
#include <RemoteObject.h>
#include <RemoteObjectProtocol.h>
#include <SmartBuffer.h>
#include <SPI.h>

#define ENQ '\5'
#define NAK '\21'

class SPIRemoteObject: public RemoteObject {
public:

	SPIRemoteObject(const char* nodeIdPSTR, const char* objectNamePSTR,	const char* securityTokenPSTR, uint8_t slaveSelectPin) :
			RemoteObject(nodeIdPSTR, objectNamePSTR, securityTokenPSTR) {

		this->slaveSelectPin = slaveSelectPin;
		pinMode(slaveSelectPin, OUTPUT);
		digitalWrite(slaveSelectPin, HIGH);
	}

	Stream* sendRequest(SmartBuffer* buf);

protected:

	uint8_t slaveSelectPin;

};

Stream* SPIRemoteObject::sendRequest(SmartBuffer* buf) {
	// enable slave
	digitalWrite(slaveSelectPin, LOW);

	// send payload
	while (buf->available()) {
		uint8_t c = buf->read();
		SPI.transfer(c);
	}

	uint8_t c;
	long t = millis();

	// keep querying slave
	delay(5);
//	Serial.println("Querying...");
	c = SPI.transfer(ENQ); // response for last (EOT) byte; must be NAK
	while ((c = SPI.transfer(ENQ)) == NAK && millis() - t < 1000) {
//		Serial.println("waiting...");
		delay(5);
	}
	if (c == NAK) {
		return NULL;
	}

	// read response
//	Serial.println("Reading response");
	buf->clear();
	buf->write(c);
	t = millis();
	do {
		c = SPI.transfer(ENQ);
		buf->write(c);
	} while (c != EOT && millis() - t < 1000);

	digitalWrite(slaveSelectPin, HIGH);

//	for (int i = 0; i < buf->available(); i++) {
//		Serial.print((char) (buf->getBuffer()[i]));
//	}

	return buf;
}

#endif /* SPIREMOTEOBJECT_H_ */
