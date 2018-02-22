/**
 * @author Claudiu Matei
 *
 */

#ifndef HTTPREMOTEOBJECT_H_
#define HTTPREMOTEOBJECT_H_

#include <Arduino.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>
#include <RemoteObject.h>
#include <RemoteObjectProtocol.h>
#include <SmartBuffer.h>
#include <Stream.h>
#include <WiFiClient.h>

class HttpRemoteObject : public RemoteObject {
public:

	HttpRemoteObject(const char* nodeIdPSTR, const char* objectNamePSTR, const char* securityTokenPSTR, const char* remoteAddressPSTR, uint16_t remotePort, Client* client) : RemoteObject(nodeIdPSTR, objectNamePSTR, securityTokenPSTR) {
		this->remoteAddress = remoteAddressPSTR;
		this->remotePort = remotePort;
		this->client = client;
	}

protected:

	const char* remoteAddress;

	uint16_t remotePort;

	Client* client;

	void disconnect();

	Stream* sendRequest(SmartBuffer* buf);

};

Stream* HttpRemoteObject::sendRequest(SmartBuffer* buf) {
	// connect
	char address[strlen_P(remoteAddress) + 1];
	strcpy_P(address, remoteAddress);
	Serial.print(address); Serial.println(" connecting...");
//	WiFiClient client;
	int connStatus = client->connect(address, remotePort);
	Serial.print(address); Serial.print(":"); Serial.print(remotePort); Serial.print(" connect status: "); Serial.println(connStatus);

	// compute payload size
	size_t contentLength = buf->available();

	// buffer http headers
	uint8_t reqBufArray[128 + contentLength]; SmartBuffer reqBuf(reqBufArray, 128 + contentLength);
	reqBuf.write_P(PSTR("POST ")); reqBuf.print(remoteNodeIdPSTR ? "/hub" : "/remoteObject"); reqBuf.write_P(PSTR(" HTTP/1.1\r\n"));
	reqBuf.write_P(PSTR("Content-Length: ")); reqBuf.println(contentLength);
	reqBuf.write_P(PSTR("Host: ")); reqBuf.write_P(remoteAddress); reqBuf.println();
	reqBuf.println();

	// buffer payload
	buf->flush(&reqBuf);

	// send request
	reqBuf.flush(client);

	// skip http headers; exit if headers not found
	if (!client->find((char*) "\r\n\r\n")) {
		return NULL;
	}

	// read response
	buf->clear();
	size_t n = client->readBytesUntil(EOT, buf->getBuffer(), buf->capacity());
	buf->setSize(n);
	Serial.print("Received "); Serial.print(n); Serial.println(" bytes");

	// disconnect
	delay(10);
	client->stop();
	Serial.println("Disconnected");

	return buf;
}


#endif /* HTTPREMOTEOBJECT_H_ */
