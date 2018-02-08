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

	bool processClient(Client *client);

};


void HttpRemoteObjectProcessor::sendPacketHeader(Print* out, char command, const char* securityTokenPSTR, size_t payloadSize) {
	// send HTTP header
	write_P(out, PSTR("HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\n")); // HTTP headers
	write_P(out, PSTR("Content-Length: ")); out->println(FPRP_PACKET_OVERHEAD_SIZE + payloadSize);
	out->println();

	// send packet header
	fprp_startPacket(out, command, securityTokenPSTR); // command = RESULT
}

bool HttpRemoteObjectProcessor::processClient(Client *client) {
	Serial.print(millis()); Serial.println(": processing http client");

	// skip http headers
	if (!client->find((char*) "\r\n\r\n")) {
		// http headers not found or incomplete
		client->stop();
		return false;
	}

	if (!processCommand(client, client)) {
		client->stop();
		return false;
	}
	client->flush();
	client->stop();
	Serial.print(millis()); Serial.println(": process http client end");
	return true;
}

#endif /* HTTPREMOTEOBJECTPROCESSOR_H_ */
