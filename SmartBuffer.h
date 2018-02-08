/**
 * @author Claudiu Matei
 *
 */

#ifndef SMARTBUFFER_H_
#define SMARTBUFFER_H_

#include <Stream.h>

class SmartBuffer : public Stream {
private:

    uint8_t* buf;

    size_t bufferCapacity;

    size_t bufTail = 0;

    size_t bufHead = 0;

    size_t write(const uint8_t* buffer, size_t size, bool isProgMem);

public:

    Print* out;

	SmartBuffer(uint8_t* buf, size_t bufferSize) : SmartBuffer(NULL, buf, bufferSize) { }

	SmartBuffer(Print* print, uint8_t* buf, size_t bufferSize);

	uint8_t* getBuffer() {
		return buf;
	}

	void setSize(size_t n) {
		bufTail = n;
	}

	size_t capacity() {
		return bufferCapacity;
	}

    virtual ~SmartBuffer() { }

    void clear();

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

SmartBuffer::SmartBuffer(Print* print, uint8_t* buf, size_t bufferSize) {
	this->out = print;
		this->buf = buf;
		this->bufferCapacity = bufferSize;
};

size_t SmartBuffer::write(const uint8_t* buffer, size_t size, bool isProgMem) {
	size_t n = size;
	size_t writableBytes;
	do {
		writableBytes = bufTail + n > bufferCapacity ? bufferCapacity - bufTail : n;
		if (isProgMem) {
    		memcpy_P(buf + bufTail, buffer, writableBytes);
		} else {
    		memcpy(buf + bufTail, buffer, writableBytes);
		}
		bufTail += writableBytes;
		buffer += writableBytes;
		n -= writableBytes;
		if (bufTail == bufferCapacity) {
			if (out != NULL) {
				flush(out);
			} else {
				return size - n;
			}
		}
	} while (n > 0);
	return size;
}

size_t SmartBuffer::write_P(const char* s) {
	return write((uint8_t*) s, strlen_P(s), true);
}

void SmartBuffer::flush(Print* out) {
	if (bufTail == 0 || out == NULL) {
		return;
	}
	out->write(buf + bufHead, bufTail - bufHead);
	bufTail = 0;
	bufHead = 0;
}

void SmartBuffer::clear() {
	bufTail = 0;
	bufHead = 0;
}

int SmartBuffer::available() {
	return bufTail - bufHead;
}

int SmartBuffer::read() {
	return bufTail > bufHead ? buf[bufHead++] : -1;
}

int SmartBuffer::peek() {
	return bufTail > bufHead ? buf[bufHead] : -1;
}

#endif /* SMARTBUFFER_H_ */
