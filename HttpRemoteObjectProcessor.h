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
	write_P(out, PSTR("HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n")); // HTTP headers
	write_P(out, PSTR("Content-Length: ")); out->println(FPRP_PACKET_OVERHEAD_SIZE + payloadSize);
	out->println();

	// send packet header
	fprp_startPacket(out, command, securityTokenPSTR); // command = RESULT
}

bool HttpRemoteObjectProcessor::processClient(Client *client) {
	debug_println("processing http client");
	client->setTimeout(1000);
	// skip http headers
	if (!client->find((char*) "\r\n\r\n")) {
		// http headers not found or incomplete
		debug_println("http headers not found or incomplete");
//		client->stop();
		return false;
	}

	// read http headers
//	bool readHeaders = true;
//	char buf[128];
//	while (readHeaders) {
//		int n = client->readBytesUntil('\n', buf, 127);
//		if (n == 0) {
//			debug_println("http headers not found or incomplete");
//			client->stop();
//			return false;
//		} else if (n == 1) {
//			readHeaders = false;
//		}
//		buf[n-1] = '\0';
//		Serial.println(buf);
//	}

	bool res = processCommand(client, client);

//	client->flush();
	delay(10);
	client->stop();
	debug_println("process http client end");
	return res;
}

#endif /* HTTPREMOTEOBJECTPROCESSOR_H_ */
