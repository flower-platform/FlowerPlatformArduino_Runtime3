/*
 *  Author: Claudiu Matei
 */

#ifndef FlowerPlatformArduinoRuntime_h
#define FlowerPlatformArduinoRuntime_h

#include <Arduino.h>
#include <Print.h>
//#include <FlowerPlatformStringManip.h>

#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#define DEBUG_FP 0

template <class E> class Callback {
public:

	virtual void operator()(E* event) const = 0;

	virtual ~Callback() { }

};

template <class T, class E> class DelegatingCallback : public Callback<E> {
protected:

	T* instance;

	void (T::*functionPointer)(E* event);

public:

	DelegatingCallback(T* _instance, void (T::*_functionPointer)(E* event)) {
		instance = _instance;
		functionPointer = _functionPointer;
	}

	void operator()(E* event) const {
		(*instance.*functionPointer)(event);
	}

	virtual ~DelegatingCallback() { }

};

class ValueChangedEvent {
public:

	int previousValue;

	int currentValue;

};

int freeRam() {
#ifdef ESP8266
	return system_get_free_heap_size();
#elif SAMD21_SERIES
	extern int __HeapLimit;
	int v;
	return (int) &v - (int) &__HeapLimit;
#elif __AVR_ARCH__
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
#else
	return -1;
#endif
}

size_t write_P(Print* p, const char* s) {
	size_t n = strlen_P(s);
	char buf[n];
	memcpy_P(buf, s, n);
	p->write((uint8_t*) buf, n);
	return n;
}

#define BUFFERED_PRINT_BUFFER_SIZE 64

class BufferedPrint : public Print {
private:

	Print* out;

    uint8_t buf[BUFFERED_PRINT_BUFFER_SIZE];

    size_t bufIndex;

public:

	BufferedPrint(Print* print);

	size_t write(uint8_t b);

	size_t write(const uint8_t* buffer, size_t size);

	size_t write_P(const char* s);

	size_t write_P(const uint8_t* buffer, size_t size);

    size_t write(const uint8_t* buffer, size_t size, bool isProgMem);

    void flush();

    virtual ~BufferedPrint() { }

};

BufferedPrint::BufferedPrint(Print* print) {
	this->out = print;
	this->bufIndex = 0;
}

size_t BufferedPrint::write(uint8_t b) {
	return write(&b, 1, false);
}

size_t BufferedPrint::write(const uint8_t* buffer, size_t size) {
	return write(buffer, size, false);
}

size_t BufferedPrint::write_P(const char* s) {
	return write((uint8_t*) s, strlen_P(s), true);
}

size_t BufferedPrint::write_P(const uint8_t* buffer, size_t size) {
	return write(buffer, size, true);
}

size_t BufferedPrint::write(const uint8_t* buffer, size_t size, bool isProgMem) {
	size_t n = size;
	size_t writableBytes;
	do {
		writableBytes = bufIndex + n > BUFFERED_PRINT_BUFFER_SIZE ? BUFFERED_PRINT_BUFFER_SIZE - bufIndex : n;
		if (isProgMem) {
    		memcpy_P(buf + bufIndex, buffer, writableBytes);
		} else {
    		memcpy(buf + bufIndex, buffer, writableBytes);
		}
		bufIndex += writableBytes;
		buffer += writableBytes;
		n -= writableBytes;
		if (bufIndex == BUFFERED_PRINT_BUFFER_SIZE) {
			flush();
		}
	} while (n > 0);
	return size;
}

void BufferedPrint::flush() {
	if (bufIndex == 0) {
		return;
	}
	out->write(buf, bufIndex);
	bufIndex = 0;
}

#endif
