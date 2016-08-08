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

	INetworkAdapter(String ipAddress);

	virtual void setup() { }

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

INetworkAdapter::INetworkAdapter(String ipAddress) {
	parseBytes(ipAddress.c_str(), '.', this->ipAddress, 4, 10);
}

#endif
