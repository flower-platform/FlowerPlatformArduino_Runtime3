/**
 * @author Claudiu Matei
 *
 */

#ifndef SMARTBUFFER_H_
#define SMARTBUFFER_H_

#include <pgmspace.h>
#include <stddef.h>
#include <Stream.h>
#include <cstdint>
#include <cstring>

template <size_t BUFFER_SIZE = 64>
class SmartBuffer : public Stream {
private:

    uint8_t buf[BUFFER_SIZE];

    size_t bufTail = 0;

    size_t bufHead = 0;

    size_t write(const uint8_t* buffer, size_t size, bool isProgMem);

public:

    Print* out;

    SmartBuffer() {
    	this->out = NULL;
    };

	SmartBuffer(Print* print) {
		this->out = print;
	}

    virtual ~SmartBuffer() { }

    /******************************* Print methods *********************************/

	size_t write(uint8_t b) {
		return write(&b, 1, false);
	}

	size_t write(const uint8_t* buffer, size_t size) {
		return write(buffer, size, false);
	}

	size_t write_P(const uint8_t* buffer, size_t size) {
		return write(buffer, size, true);
	}

	size_t write_P(const char* s);

	/******************************* Stream methods *********************************/

    int available();

    int read();

    int peek();

    void flush() {
    	flush(out);
    }

    void flush(Print* out);

};

template <size_t BUFFER_SIZE> size_t SmartBuffer<BUFFER_SIZE>::write(const uint8_t* buffer, size_t size, bool isProgMem) {
	size_t n = size;
	size_t writableBytes;
	do {
		writableBytes = bufTail + n > BUFFER_SIZE ? BUFFER_SIZE - bufTail : n;
		if (isProgMem) {
    		memcpy_P(buf + bufTail, buffer, writableBytes);
		} else {
    		memcpy(buf + bufTail, buffer, writableBytes);
		}
		bufTail += writableBytes;
		buffer += writableBytes;
		n -= writableBytes;
		if (bufTail == BUFFER_SIZE) {
			if (out != NULL) {
				flush(out);
			} else {
				return size - n;
			}
		}
	} while (n > 0);
	return size;
}

template <size_t BUFFER_SIZE> size_t SmartBuffer<BUFFER_SIZE>::write_P(const char* s) {
	return write((uint8_t*) s, strlen_P(s), true);
}

template <size_t BUFFER_SIZE> void SmartBuffer<BUFFER_SIZE>::flush(Print* out) {
	if (bufTail == 0 || out == NULL) {
		return;
	}
	out->write(buf + bufHead, bufTail - bufHead);
	bufTail = 0;
	bufHead = 0;
}

template <size_t BUFFER_SIZE> int SmartBuffer<BUFFER_SIZE>::available() {
	return bufTail - bufHead;
}

template <size_t BUFFER_SIZE> int SmartBuffer<BUFFER_SIZE>::read() {
	return bufTail > bufHead ? buf[bufHead++] : -1;
}

template <size_t BUFFER_SIZE> int SmartBuffer<BUFFER_SIZE>::peek() {
	return bufTail > bufHead ? buf[bufHead] : -1;
}

#endif /* SMARTBUFFER_H_ */
