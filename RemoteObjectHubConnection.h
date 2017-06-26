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

class RemoteObjectHubConnection {
public:

	RemoteObjectHubConnection(NetworkConnection* connection, unsigned long pollInterval, const char* securityTokenPSTR, const char* localRappInstanceName, uint16_t localServerPort);

	bool processCommand();

	void loop();

	virtual ~RemoteObjectHubConnection() { };

protected:

	NetworkConnection* connection;

	unsigned long pollInterval;

	unsigned long lastPollTime = 0;

	const char* securityToken;

	const char* localRappInstanceName;

	bool registered = false;

	uint16_t localServerPort = 0;

	void registerToHub();

};

RemoteObjectHubConnection::RemoteObjectHubConnection(NetworkConnection* connection, unsigned long pollInterval, const char* securityTokenPSTR, const char* localRappInstanceName, uint16_t localServerPort) {
	this->connection = connection;
	this->pollInterval = pollInterval;
	this->securityToken = securityTokenPSTR;
	this->localRappInstanceName = localRappInstanceName;
	this->localServerPort = localServerPort;
}

bool RemoteObjectHubConnection::processCommand() {
	// skip http headers; break if headers not found
	if (!connection->in->find((char*) "\r\n\r\n")) {
		return false;
	}

	size_t size = 0;
	int cmd = fprp_readCommand(connection->in, securityToken);
	if (cmd < 0) {
		Serial.print("Error reading command: "); Serial.println(cmd);
		registered = false;
		return false;
	}

	char callbackIdStr[8];

	char rbuf[RECV_BUFFER_SIZE];

	switch (cmd) {
	case 'I': {  // INVOKE
//		size = connection->in->readBytesUntil('\0', rbuf, RECV_BUFFER_SIZE); // hasNext
//		hasNext = (*rbuf == '1');
		connection->in->readBytesUntil('\0', rbuf, RECV_BUFFER_SIZE); // rappInstanceId (ignored)
		size = connection->in->readBytesUntil('\0', rbuf, RECV_BUFFER_SIZE); // callbackId
		strncpy(callbackIdStr, rbuf, size);
		callbackIdStr[size] = '\0';
		size = connection->in->readBytesUntil('\x04', rbuf, RECV_BUFFER_SIZE); // function call
		rbuf[size] = '\0';

		//buffer command specific packet fields and response
		BufferedPrint<64> tbuf(connection->out);
//		tbuf.print('0'); tbuf.print(TERM); // hasNext = false
		tbuf.print(callbackIdStr); tbuf.print(TERM); // callbackId
		dispatchFunctionCall(rbuf, &tbuf);

		// start HTTP request; send headers
		connection->startHttpRequest("/hub", FPRP_FIXED_PACKET_SIZE + tbuf.getSize());
		// send response packet
		fprp_startPacket(connection->out, 'R', securityToken); // command = RESULT
		tbuf.flush();
		fprp_endPacket(connection->out);
		connection->flush();
		return true; }
	case 'R': { // result
//		size = connection->in->readBytesUntil(TERM, rbuf, RECV_BUFFER_SIZE); // hasNext
//		hasNext = (*rbuf == '1');
		size = connection->in->readBytesUntil(TERM, rbuf, RECV_BUFFER_SIZE); // callbackId
		strncpy(callbackIdStr, rbuf, size);
		callbackIdStr[size] = '\0';

		uint16_t callbackId = (uint16_t) atol(callbackIdStr);
//		Serial.print("Response received; callbackId="); Serial.println(callbackId);
		executeCallback(callbackId, connection->in);
		return true; }
	}

	return false;
}

void RemoteObjectHubConnection::registerToHub() {
	Serial.println("Registering...");
	BufferedPrint<32> buf(connection->out);
	write_P(&buf, localRappInstanceName); buf.print(TERM);
	buf.print(localServerPort);	buf.print(TERM);
	connection->startHttpRequest("/hub", FPRP_FIXED_PACKET_SIZE + buf.getSize());
	fprp_startPacket(connection->out, 'A', securityToken); // register
	buf.flush();
	fprp_endPacket(connection->out);
	connection->flush();
	processCommand();
	registered = true;
}

void RemoteObjectHubConnection::loop() {
	if (millis() - lastPollTime < pollInterval) {
		return;
	}
	lastPollTime = millis();

	if (!connection->connect()) {
		registered = false;
		return;
	}
	if (!registered) {
		registerToHub();
	}

	// get pending invocations
	connection->startHttpRequest("/hub", FPRP_FIXED_PACKET_SIZE);
	fprp_startPacket(connection->out, 'J', securityToken);
	fprp_endPacket(connection->out);
	connection->flush();
	while(processCommand());

	// get pending responses
	connection->startHttpRequest("/hub", FPRP_FIXED_PACKET_SIZE);
	fprp_startPacket(connection->out, 'S', securityToken);
	fprp_endPacket(connection->out);
	connection->flush();
	while(processCommand());

	connection->disconnect();
	lastPollTime = millis();
}

#endif /* REMOTEOBJECTHUBCONNECTION_H_ */
