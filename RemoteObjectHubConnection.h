/**
 *	@author: Claudiu Matei
 *
 */

#ifndef REMOTEOBJECTHUBCONNECTION_H_
#define REMOTEOBJECTHUBCONNECTION_H_

#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>
#include <NetworkConnection.h>
#include <RemoteObject.h>
#include <RemoteObjectProtocol.h>

#ifdef ESP8266
#define PACKET_BUFFER_SIZE 6210
#define RESULT_BUFFER_SIZE 2048
#else
#define PACKET_BUFFER_SIZE 128
#define RESULT_BUFFER_SIZE 64
#endif

class RemoteObjectHubConnection {
public:

	RemoteObjectHubConnection(const char* remoteAddressPSTR, int endpointPort, unsigned long pollInterval, const char* securityTokenPSTR, const char* localRappInstanceName, uint16_t localServerPort);

	bool processCommand(Stream* in, Print* out);

	void loop();

	virtual ~RemoteObjectHubConnection() { };

protected:

	const char* remoteAddress;

	int endpointPort = 80;

	unsigned long pollInterval;

	unsigned long lastPollTime = 0;

	const char* securityToken;

	const char* localRappInstanceName;

	bool registered = false;

	uint16_t localServerPort = 0;

	void registerToHub(Stream* in, Print* out);

	void startHttpRequest(Print* out, const char* url, int contentLength = -1);

};



RemoteObjectHubConnection::RemoteObjectHubConnection(const char* remoteAddressPSTR, int endpointPort, unsigned long pollInterval, const char* securityTokenPSTR, const char* localRappInstanceName, uint16_t localServerPort) {
	this->remoteAddress = remoteAddressPSTR;
	this->endpointPort = endpointPort;
	this->pollInterval = pollInterval;
	this->securityToken = securityTokenPSTR;
	this->localRappInstanceName = localRappInstanceName;
	this->localServerPort = localServerPort;
}

bool RemoteObjectHubConnection::processCommand(Stream* in, Print* out) {
	// skip http headers; break if headers not found
	if (!in->find((char*) "\r\n\r\n")) {
		return false;
	}

	size_t size = 0;
	int cmd = fprp_readCommand(in, securityToken);
	if (cmd < 0) {
		Serial.print("Error reading command: "); Serial.println(cmd);
		registered = false;
		return false;
	}

	char callbackIdStr[8];

	char rbuf[PACKET_BUFFER_SIZE];

	switch (cmd) {
	case 'I': {  // INVOKE
//		size = connection->in->readBytesUntil('\0', rbuf, RECV_BUFFER_SIZE); // hasNext
//		hasNext = (*rbuf == '1');
		in->readBytesUntil('\0', rbuf, PACKET_BUFFER_SIZE); // rappInstanceId (ignored)
		size = in->readBytesUntil('\0', rbuf, PACKET_BUFFER_SIZE); // callbackId
		strncpy(callbackIdStr, rbuf, size);
		callbackIdStr[size] = '\0';
		size = in->readBytesUntil('\x04', rbuf, PACKET_BUFFER_SIZE); // function call
		rbuf[size] = '\0';

		//buffer command specific packet fields and response
		uint8_t buf[64];
		SmartBuffer tbuf(out, buf, 64);
		tbuf.print(callbackIdStr); tbuf.print(TERM); // callbackId
		dispatchFunctionCall(rbuf, &tbuf);

		// start HTTP request; send headers
		startHttpRequest(out, "/hub", FPRP_PACKET_OVERHEAD_SIZE + tbuf.available());
		// send response packet
		fprp_startPacket(out, 'R', securityToken); // command = RESULT
		tbuf.flush();
		fprp_endPacket(out);
//		connection->flush();
		return true; }
	case 'R': { // result
		size = in->readBytesUntil(TERM, rbuf, PACKET_BUFFER_SIZE); // callbackId
		strncpy(callbackIdStr, rbuf, size);
		callbackIdStr[size] = '\0';

		uint16_t callbackId = (uint16_t) atol(callbackIdStr);
//		Serial.print("Response received; callbackId="); Serial.println(callbackId);
		executeCallback(callbackId, in);
		return true; }
	}

	return false;
}

void RemoteObjectHubConnection::registerToHub(Stream* in, Print* out) {
	Serial.println("Registering...");
	uint8_t bufArray[32];
	SmartBuffer buf(out, bufArray, 43);
	write_P(&buf, localRappInstanceName); buf.print(TERM);
	buf.print(localServerPort);	buf.print(TERM);
	startHttpRequest(out, "/hub", FPRP_PACKET_OVERHEAD_SIZE + buf.available());
	fprp_startPacket(out, 'A', securityToken); // register
	buf.flush();
	fprp_endPacket(out);
//	connection->flush();
	processCommand(in, out);
	registered = true;
}

void RemoteObjectHubConnection::loop() {
		if (millis() - lastPollTime < pollInterval) {
		return;
	}
	lastPollTime = millis();

	Client* client = connect();

	if (!client) {
		registered = false;
		return;
	}
	if (!registered) {
		registerToHub(client, client);
	}

	// get pending invocations
	startHttpRequest(client, "/hub", FPRP_PACKET_OVERHEAD_SIZE);
	fprp_startPacket(client, 'J', securityToken);
	fprp_endPacket(client);
	client->flush();

	while(processCommand(client, client));

	// get pending responses
	startHttpRequest(client, "/hub", FPRP_PACKET_OVERHEAD_SIZE);
	fprp_startPacket(client, 'S', securityToken);
	fprp_endPacket(client);
	client->flush();
	while(processCommand(client, client));

	client->stop();
	lastPollTime = millis();
}

void RemoteObjectHubConnection::startHttpRequest(Print* out, const char* url, int contentLength = -1) {
	write_P(out, PSTR("POST ")); out->print(url); write_P(out, PSTR(" HTTP/1.1\r\n"));
	if (contentLength >= 0) {
		write_P(out, PSTR("Content-Length: ")); out->println(contentLength);
	}
	write_P(out, PSTR("Host: ")); write_P(out, remoteAddress); out->println();
	out->println();
}


#endif /* REMOTEOBJECTHUBCONNECTION_H_ */
