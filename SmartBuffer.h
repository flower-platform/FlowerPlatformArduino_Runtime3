/**
 * @author Claudiu Matei
 *
 */

#ifndef SMARTBUFFER_H_
#define SMARTBUFFER_H_

#include <Stream.h>

template <size_t BUFFER_CAPACITY = 64>
class SmartBuffer : public Stream {
private:

    uint8_t buf[BUFFER_CAPACITY];

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

	uint8_t* getBuffer() {
		return buf;
	}

	void setSize(size_t n) {
		bufTail = n;
	}

	size_t capacity() {
		return BUFFER_CAPACITY;
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

template <size_t BUFFER_CAPACITY> size_t SmartBuffer<BUFFER_CAPACITY>::write(const uint8_t* buffer, size_t size, bool isProgMem) {
	size_t n = size;
	size_t writableBytes;
	do {
		writableBytes = bufTail + n > BUFFER_CAPACITY ? BUFFER_CAPACITY - bufTail : n;
		if (isProgMem) {
    		memcpy_P(buf + bufTail, buffer, writableBytes);
		} else {
    		memcpy(buf + bufTail, buffer, writableBytes);
		}
		bufTail += writableBytes;
		buffer += writableBytes;
		n -= writableBytes;
		if (bufTail == BUFFER_CAPACITY) {
			if (out != NULL) {
				flush(out);
			} else {
				return size - n;
			}
		}
	} while (n > 0);
	return size;
}

template <size_t BUFFER_CAPACITY> size_t SmartBuffer<BUFFER_CAPACITY>::write_P(const char* s) {
	return write((uint8_t*) s, strlen_P(s), true);
}

template <size_t BUFFER_CAPACITY> void SmartBuffer<BUFFER_CAPACITY>::flush(Print* out) {
	if (bufTail == 0 || out == NULL) {
		return;
	}
	out->write(buf + bufHead, bufTail - bufHead);
	bufTail = 0;
	bufHead = 0;
}

template <size_t BUFFER_CAPACITY> void SmartBuffer<BUFFER_CAPACITY>::clear() {
	bufTail = 0;
	bufHead = 0;
}

template <size_t BUFFER_CAPACITY> int SmartBuffer<BUFFER_CAPACITY>::available() {
	return bufTail - bufHead;
}

template <size_t BUFFER_CAPACITY> int SmartBuffer<BUFFER_CAPACITY>::read() {
	return bufTail > bufHead ? buf[bufHead++] : -1;
}

template <size_t BUFFER_CAPACITY> int SmartBuffer<BUFFER_CAPACITY>::peek() {
	return bufTail > bufHead ? buf[bufHead] : -1;
}

#endif /* SMARTBUFFER_H_ */
