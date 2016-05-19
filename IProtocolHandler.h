/*
 *  Author: Claudiu Matei
 */

#ifndef IProtocolHandler_h
#define IProtocolHandler_h

#include <Client.h>

class IProtocolHandler {
public:

	int port;

	virtual void processClientRequest(Client* client);

	virtual void setup() { }

	virtual void loop() { }

	virtual ~IProtocolHandler() { }
};

#endif
