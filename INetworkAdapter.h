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

	virtual void setup() { }

	virtual void loop() { }

	virtual ~INetworkAdapter() { }

};
#endif
