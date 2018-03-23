/**
 * @author Claudiu Matei
 *
 */

#ifndef HTTPREMOTEOBJECT_H_
#define HTTPREMOTEOBJECT_H_

#include <Arduino.h>
#include <Client.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>
#include <RemoteObject.h>
#include <RemoteObjectProtocol.h>
#include <stddef.h>
#include <SmartBuffer.h>
#include <Stream.h>

class HttpRemoteObject : public RemoteObject {
public:

	HttpRemoteObject(const char* nodeIdPSTR, const char* objectNamePSTR, const char* securityTokenPSTR, const char* remoteAddressPSTR, uint16_t remotePort, Client* client) : RemoteObject(nodeIdPSTR, objectNamePSTR, securityTokenPSTR) {
		strcpy_P(this->remoteAddress, remoteAddressPSTR);
		this->remotePort = remotePort;
		this->client = client;
	}

protected:

	unsigned long lastReadTimestamp = 0;

	char remoteAddress[16];

	uint16_t remotePort;

	Client* client;

	void disconnect();

	Stream* sendRequest(SmartBuffer* buf);

};

Stream* HttpRemoteObject::sendRequest(SmartBuffer* buf) {
	if (!client->connected() || millis() - lastReadTimestamp > 10000) {
		debug_println("connecting... previouslyConnected=", client->connected());

		client->stop();
		Serial.print(remoteAddress); Serial.print(":"); Serial.println(remotePort);

		int connStatus = client->connect(remoteAddress, remotePort);
		debug_println("connect status: ", connStatus);

		if (connStatus == 0) {
			client->stop();
			return NULL;
		}
	}

	// compute payload size
	size_t contentLength = buf->available();

	// buffer http headers
	uint8_t reqBufArray[128 + contentLength]; SmartBuffer reqBuf(reqBufArray, 128 + contentLength);
	reqBuf.write_P(PSTR("POST ")); reqBuf.print(strlen(remoteNodeId) > 0 ? "/hub" : "/remoteObject"); reqBuf.write_P(PSTR(" HTTP/1.1\r\n"));
	reqBuf.write_P(PSTR("Content-Length: ")); reqBuf.println(contentLength);
	reqBuf.write_P(PSTR("Host: ")); reqBuf.write_P(remoteAddress); reqBuf.println();
	reqBuf.println();

	// buffer payload
	buf->flush(&reqBuf);

	// send request
	reqBuf.flush(client);
//	client->flush();	// DO NOT USE client->flush() on ESP8266; it slows down or breaks communication
	debug_println("Data flushed");

	debug_println("Reading data...");
	// skip http headers; exit if headers not found
	client->setTimeout(5000);
	if (!client->find((char*) "\r\n\r\n")) {
		debug_println("Read timeout");
		client->stop();
		return NULL;
	}

	// read response
	buf->clear();
	size_t n = client->readBytesUntil(EOT, buf->getBuffer(), buf->capacity());
	buf->setSize(n);
	buf->write(EOT); // include EOT in packet
	debug_println("Received ", n, " bytes");
	lastReadTimestamp = millis();

	// disconnect if not using keep-alive
//	delay(10);
//	client->stop();
//	debug_println("Disconnected");

	return buf;
}

#endif /* HTTPREMOTEOBJECT_H_ */
