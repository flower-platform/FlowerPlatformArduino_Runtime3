/*
 *  Author: Claudiu Matei
 */

#ifndef IProtocolHandler_h
#define IProtocolHandler_h

#include <Client.h>

class IProtocolHandler {

public:

	int port;

	IProtocolHandler(int port);

	virtual void processClientRequest(Client* client) = 0;

	virtual ~IProtocolHandler() { };

};

IProtocolHandler::IProtocolHandler(int port) {
	this->port = port;
}

#endif
