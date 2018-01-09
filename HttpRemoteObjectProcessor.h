/**
 * @author Claudiu Matei
 */

#ifndef HTTPREMOTEOBJECTPROCESSOR_H_
#define HTTPREMOTEOBJECTPROCESSOR_H_

#include <Arduino.h>
#include <Client.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>
#include <RemoteObjectProcessor.h>
#include <RemoteObjectProtocol.h>
#include <stddef.h>

class HttpRemoteObjectProcessor : public RemoteObjectProcessor {
public:

	HttpRemoteObjectProcessor(const char* securityTokenPSTR) : RemoteObjectProcessor(securityTokenPSTR) { }

	virtual void sendPacketHeader(Print* out, char command, const char* securityTokenPSTR, size_t payloadSize);

	void processClient(Client *client);

};


void HttpRemoteObjectProcessor::sendPacketHeader(Print* out, char command, const char* securityTokenPSTR, size_t payloadSize) {
	// send HTTP header
	write_P(out, PSTR("HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\n")); // HTTP headers
	write_P(out, PSTR("Content-Length: ")); out->println(FPRP_PACKET_OVERHEAD_SIZE + payloadSize);
	out->println();

	// send packet header
	fprp_startPacket(out, command, securityTokenPSTR); // command = RESULT
}

void HttpRemoteObjectProcessor::processClient(Client *client) {
	Serial.println("processing client");

	// skip http headers
	if (!client->find((char*) "\r\n\r\n")) {
		// http headers not found or incomplete
		client->stop();
		return;
	}

	if (!processCommand(client, client)) {
		client->stop();
		return;
	}
	client->flush();
	delay(100);
	client->stop();
}

#endif /* HTTPREMOTEOBJECTPROCESSOR_H_ */