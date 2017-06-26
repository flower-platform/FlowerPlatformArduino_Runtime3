/**
 * @author Claudiu Matei
 *
 */

#ifndef RS485SERIAL_H_
#define RS485SERIAL_H_

#include <Arduino.h>
#include <stddef.h>
#include <Stream.h>
#include <cstdint>

class RS485Serial: public Stream {
private:
	Stream* serial;
	uint8_t writeEnablePin;
	bool batchWriteMode = false;

public:

	RS485Serial(Stream &serial, uint8_t writeEnablePin) {
		this->serial = &serial;
		this->writeEnablePin = writeEnablePin;
		pinMode(writeEnablePin, OUTPUT);
		digitalWrite(writeEnablePin, LOW);
	}

	virtual ~RS485Serial() { }

	// Stream impl.
	int available() { return serial->available(); }
    int read() { return serial->read(); }
    int peek() { return serial->peek(); }
    void flush() { serial->flush(); }

    // Print impl.
    size_t write(uint8_t b);
    size_t write(const uint8_t *buffer, size_t size);

    int timedRead() { return Stream::timedRead(); }

    void setBatchWriteMode(bool batchWriteMode);

};

void RS485Serial::setBatchWriteMode(bool batchWriteMode) {
	this->batchWriteMode = batchWriteMode;
 	if (batchWriteMode) {
     	digitalWrite(writeEnablePin, HIGH);
 	} else {
		serial->flush();
		delay(1);
		digitalWrite(writeEnablePin, LOW);
 	}
}

size_t RS485Serial::write(uint8_t b) {
 	bool bw = this->batchWriteMode;
	if (!bw) {
     	setBatchWriteMode(true);
 	}
 	size_t n = serial->write(b);
 	if (!bw) {
 		setBatchWriteMode(false);
 	}
 	return n;
 }

size_t RS485Serial::write(const uint8_t *buffer, size_t size) {
 	bool bw = this->batchWriteMode;
	if (!bw) {
     	setBatchWriteMode(true);
 	}
	size_t n = serial->write(buffer, size);
 	if (!bw) {
 		setBatchWriteMode(false);
 	}
	return n;
}


#endif /* RS485SERIAL_H_ */
