/**
 * @author Claudiu Matei
 */

#ifndef WEBSOCKETREMOTEOBJECT_H_
#define WEBSOCKETREMOTEOBJECT_H_

#include <Arduino.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <RemoteObject.h>
#include <RemoteObjectProtocol.h>
#include <SmartBuffer.h>
#include <WebSocketsServer.h>

class WebSocketRemoteObject : public RemoteObject {
public:

	WebSocketRemoteObject(const char* nodeIdPSTR, const char* objectNamePSTR, const char* securityTokenPSTR, uint16_t port) : RemoteObject(nodeIdPSTR, objectNamePSTR, securityTokenPSTR) {
		this->webSocket = new WebSocketsServer(port, "*");
	}

	virtual void loop() {
		this->webSocket->loop();
	}

	void start();

	void stop();

	~WebSocketRemoteObject();

protected:

	WebSocketsServer* webSocket;

	Stream* sendRequest(SmartBuffer* buf);

};

Stream* WebSocketRemoteObject::sendRequest(SmartBuffer* buf) {
	if (!webSocket->connectedClients(true)) {
		return NULL;
	}

	// send request
	webSocket->broadcastTXT(buf->getBuffer(), buf->available());
	debug_println("Data flushed");

	// not expecting response; WSRemoteObject is used only for notifications / void functions.

	return NULL;
}

void WebSocketRemoteObject::start() {
	this->webSocket->begin();
}

void WebSocketRemoteObject::stop() {
	webSocket->close();
}

WebSocketRemoteObject::~WebSocketRemoteObject() {
	delete webSocket;
	webSocket = NULL;
}

#endif /* WEBSOCKETREMOTEOBJECT_H_ */
