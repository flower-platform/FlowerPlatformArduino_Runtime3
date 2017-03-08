/*
 *  Author: Claudiu Matei
 */

#ifndef FlowerPlatformArduinoRuntime_h
#define FlowerPlatformArduinoRuntime_h

#include <avr/pgmspace.h>
#include <Arduino.h>
#include <Print.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

typedef void (*TDispatchFunction)(const char*, Print*);

char* __nextStringStart;
char* nextString(const char* s) {
	if (s) {
		__nextStringStart = (char*) s;
	}
	char* res = __nextStringStart;
	__nextStringStart += strlen(__nextStringStart);
	__nextStringStart++;
	return res;
}

/**
 * base on code from http://arduino.stackexchange.com/questions/18007/simple-url-decoding
 */
const char* decodeAndBreakUrl(const char* url) {
	char *leader =(char*) url;
	char *follower = leader;

	// While we're not at the end of the string (current character not NULL)
	while (*leader) {
	    if (*leader == '%') { // Check to see if the current character is a %
	        // Grab the next two characters and move leader forwards
	        leader++;
	        char high = *leader;
	        leader++;
	        char low = *leader;

	        // Convert ASCII 0-9A-F to a value 0-15
	        if (high > 0x39) high -= 7;
	        high &= 0x0f;

	        // Same again for the low byte:
	        if (low > 0x39) low -= 7;
	        low &= 0x0f;

	        // Combine the two into a single byte and store in follower:
	        *follower = (high << 4) | low;
	    } else if (*leader == '/') { // replace slashes with string terminator
	    	*follower ='\0';
	    } else {
	        // All other characters copy verbatim
	        *follower = *leader;
	    }

	    // Move both pointers to the next character:
	    leader++;
	    follower++;
	}
	// Terminate the new string with a NULL character to trim it off
	*follower = 0;
	return url;
}

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

size_t write_P(Print* p, uint8_t* s, size_t size) {
	uint8_t buf[16];
	size_t k;
	do {
		k = size > 16 ? 16 : size;
		memcpy_P(buf, s, k);
		p->write(buf, k);
		size -= k;
		s += k;
	} while (size);
	return size;
}

size_t write_P(Print* p, const char* s) {
	return write_P(p, (uint8_t*) s, strlen_P(s));
}

void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
    for (int i = 0; i < maxBytes; i++) {
        bytes[i] = strtoul(str, NULL, base);  // Convert byte
        str = strchr(str, sep);               // Find next separator
        if (str == NULL || *str == '\0') {
            break;                            // No more separators, exit
        }
        str++;                                // Point to next character after separator
    }
}

template <size_t BUFFER_SIZE = 64>
class BufferedPrint : public Print {
private:

    uint8_t buf[BUFFER_SIZE];

    size_t bufIndex = 0;

    size_t write(const uint8_t* buffer, size_t size, bool isProgMem);

public:

    Print* out;

    BufferedPrint() {
    	this->out = NULL;
    };

	BufferedPrint(Print* print) {
		this->out = print;
	}

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
		writableBytes = bufIndex + n > BUFFER_SIZE ? BUFFER_SIZE - bufIndex : n;
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

#endif
