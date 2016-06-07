/*
 *  Author: Claudiu Matei
 */

#ifndef INetworkAdapter_h
#define INetworkAdapter_h

#include <IProtocolHandler.h>

class INetworkAdapter {
public:

	IProtocolHandler* protocolHandler;

	uint8_t ipAddress[4];

	// TOCO CM: temporary; we will use setters for ip and mac
	const char* ipAddressStr = NULL;

	virtual void setup() {
		if (ipAddressStr != NULL) {
			parseBytes(ipAddressStr, '.', ipAddress, 4, 10);
		}
	}

	virtual void loop() { }

	virtual ~INetworkAdapter() { }

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

};
#endif
