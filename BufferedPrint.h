/**
 * @author Claudiu Matei
 *
 */

#ifndef BUFFEREDPRINT_H_
#define BUFFEREDPRINT_H_

#include <Arduino.h>
#include <Print.h>

template <size_t BUFFER_SIZE = 64>
class BufferedPrint : public Print {
private:

    uint8_t STACK_BUF[BUFFER_SIZE];

    uint8_t* buf;

    size_t bufferSize;

    size_t bufIndex = 0;

    size_t write(const uint8_t* buffer, size_t size, bool isProgMem);

public:

    Print* out;

	BufferedPrint() : BufferedPrint(NULL, NULL, 0) { };

	BufferedPrint(Print* print) : BufferedPrint(print, NULL, 0) { }

	BufferedPrint(uint8_t* buf, size_t bufferSize) : BufferedPrint(NULL, buf, bufferSize) { }

	BufferedPrint(Print* print, uint8_t* buf, size_t bufferSize) {
    	this->out = print;
    	if (buf) {
    		this->buf = buf;
    		this->bufferSize = bufferSize;
    	} else {
    		this->buf = STACK_BUF;
    		this->bufferSize = BUFFER_SIZE;
    	}
    };

	size_t write(uint8_t b) {
		return write(&b, 1, false);
	}

	size_t write(const uint8_t* buffer, size_t size) {
		return write(buffer, size, false);
	}

	size_t write_P(const char* s);

	size_t write_P(const uint8_t* buffer, size_t size) {
		return write(buffer, size, true);
	}

	size_t getSize() {
		return bufIndex;
	}

    void flush();

    virtual ~BufferedPrint() { }

};

template <size_t BUFFER_SIZE> size_t BufferedPrint<BUFFER_SIZE>::write_P(const char* s) {
	return write((uint8_t*) s, strlen_P(s), true);
}

template <size_t BUFFER_SIZE> size_t BufferedPrint<BUFFER_SIZE>::write(const uint8_t* buffer, size_t size, bool isProgMem) {
	size_t n = size;
	size_t writableBytes;
	do {
		writableBytes = bufIndex + n > bufferSize ? bufferSize - bufIndex : n;
		if (isProgMem) {
    		memcpy_P(buf + bufIndex, buffer, writableBytes);
		} else {
    		memcpy(buf + bufIndex, buffer, writableBytes);
		}
		bufIndex += writableBytes;
		buffer += writableBytes;
		n -= writableBytes;
		if (bufIndex == BUFFER_SIZE) {
			flush();
		}
	} while (n > 0);
	return size;
}

template <size_t BUFFER_SIZE> void BufferedPrint<BUFFER_SIZE>::flush() {
	if (bufIndex == 0) {
		return;
	}
	out->write(buf, bufIndex);
	bufIndex = 0;
}




#endif /* BUFFEREDPRINT_H_ */
