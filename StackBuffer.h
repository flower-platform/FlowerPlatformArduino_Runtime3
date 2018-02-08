/**
 * @author Claudiu Matei
 *
 */

#ifndef STACKBUFFER_H_
#define STACKBUFFER_H_

#include <Stream.h>

template <size_t STACK_BUFFER_CAPACITY = 64>
class StackBuffer : public Stream {
private:

    uint8_t STACK_BUF[STACK_BUFFER_CAPACITY];

    uint8_t* buf;

    size_t bufferCapacity;

    size_t bufTail = 0;

    size_t bufHead = 0;

    size_t write(const uint8_t* buffer, size_t size, bool isProgMem);

public:

    Print* out;

    StackBuffer() : StackBuffer(NULL, NULL, 0) { };

    StackBuffer(Print* print) : StackBuffer(print, NULL, 0) { }

	StackBuffer(uint8_t* buf, size_t bufferSize) : StackBuffer(NULL, buf, bufferSize) { }

	StackBuffer(Print* print, uint8_t* buf, size_t bufferSize) {
    	this->out = print;
    	if (buf) {
    		this->buf = buf;
    		this->bufferCapacity = bufferSize;
    	} else {
    		this->buf = STACK_BUF;
    		this->bufferCapacity = STACK_BUFFER_CAPACITY;
    	}
    };

	uint8_t* getBuffer() {
		return buf;
	}

	void setSize(size_t n) {
		bufTail = n;
	}

	size_t capacity() {
		return bufferCapacity;
	}

    virtual ~StackBuffer() { }

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

template <size_t STACK_BUFFER_CAPACITY> size_t StackBuffer<STACK_BUFFER_CAPACITY>::write(const uint8_t* buffer, size_t size, bool isProgMem) {
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

template <size_t STACK_BUFFER_CAPACITY> size_t StackBuffer<STACK_BUFFER_CAPACITY>::write_P(const char* s) {
	return write((uint8_t*) s, strlen_P(s), true);
}

template <size_t STACK_BUFFER_CAPACITY> void StackBuffer<STACK_BUFFER_CAPACITY>::flush(Print* out) {
	if (bufTail == 0 || out == NULL) {
		return;
	}
	out->write(buf + bufHead, bufTail - bufHead);
	bufTail = 0;
	bufHead = 0;
}

template <size_t STACK_BUFFER_CAPACITY> void StackBuffer<STACK_BUFFER_CAPACITY>::clear() {
	bufTail = 0;
	bufHead = 0;
}

template <size_t STACK_BUFFER_CAPACITY> int StackBuffer<STACK_BUFFER_CAPACITY>::available() {
	return bufTail - bufHead;
}

template <size_t STACK_BUFFER_CAPACITY> int StackBuffer<STACK_BUFFER_CAPACITY>::read() {
	return bufTail > bufHead ? buf[bufHead++] : -1;
}

template <size_t STACK_BUFFER_CAPACITY> int StackBuffer<STACK_BUFFER_CAPACITY>::peek() {
	return bufTail > bufHead ? buf[bufHead] : -1;
}

#endif /* STACKBUFFER_H_ */
