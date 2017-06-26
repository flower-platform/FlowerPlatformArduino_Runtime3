/**
 * @author Claudiu Matei
 *
 */

#ifndef REMOTEOBJECT_H_
#define REMOTEOBJECT_H_

#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>
#include <NetworkConnection.h>
#include <RemoteObjectProtocol.h>
#include <Stream.h>

#define MAX_CALLBACKS 4

#define TYPE_VOID 0
#define TYPE_STRING 1
#define TYPE_INT 2
#define TYPE_BOOL 3

#define RECV_BUFFER_SIZE 128

extern void dispatchFunctionCall(const char* functionCall, Print* response);
extern void registerCallback(uint16_t callbackId, void* callback, uint8_t returnTypeId);
extern bool executeCallback(uint16_t callbackId, Stream *response);
extern bool executeCallback(void* callback, uint8_t returnType, Stream *response);

class RemoteObject {
public:

	RemoteObject(NetworkConnection* connection, const char* rappInstancePSTR, const char* instanceNamePSTR, const char* securityTokenPSTR) {
		this->connection = connection;
		this->rappInstance = rappInstancePSTR;
		this->instanceName = instanceNamePSTR;
		this->securityToken = securityTokenPSTR;
	};

	virtual ~RemoteObject() { }

protected:

	NetworkConnection* connection;

	const char* rappInstance;

	const char* instanceName;

	const char* securityToken;

	bool callFunction(const char* functionNamePSTR, BufferedPrint<>* argsBuf, void *callback, uint8_t returnTypeId);

};

bool RemoteObject::callFunction(const char* functionNamePSTR, BufferedPrint<>* argsBuf, void *callback, uint8_t returnTypeId) {
	if (!connection->connect()) {
		return false;
	}

	BufferedPrint<64> buf(connection->out);
	if (rappInstance) {
		write_P(&buf, rappInstance);
	}
	buf.print(TERM); // rappInstance
	buf.print(TERM); // callbackId = null
	write_P(&buf, instanceName); buf.print('.');
	write_P(&buf, functionNamePSTR); buf.print(TERM);

	size_t contentLength = buf.getSize() + FPRP_FIXED_PACKET_SIZE;
	if (argsBuf) {
		contentLength += argsBuf->getSize();
	}
	connection->startHttpRequest(rappInstance ? "/hub" : "/remoteObject", contentLength);
	fprp_startPacket(connection->out, 'I', securityToken);
	buf.flush();
	if (argsBuf) {
		argsBuf->out = connection->out;
		argsBuf->flush();
	}
	connection->out->print(EOT);
	connection->flush();

	// read response

	// skip http headers; exit if headers not found
	if (!connection->in->find((char*) "\r\n\r\n")) {
		return false;
	}
	int cmd = fprp_readCommand(connection->in, securityToken);
	if (cmd < 0) {
		Serial.print("Error reading command: "); Serial.println(cmd);
		connection->disconnect();
		return false;
	}

	switch(cmd) {
	case 'R': // result
		connection->in->find(TERM); // callbackId (ignored)
		executeCallback(callback, returnTypeId, connection->in);
		break;
	case 'P': // pending
		char callbackIdStr[8];
		int size = connection->in->readBytesUntil(TERM, callbackIdStr, 8); // callbackId
		callbackIdStr[size] = TERM;
		registerCallback((uint16_t) atol(callbackIdStr), callback, returnTypeId);
		break;
	}

	connection->disconnect();
	return true;
}

#endif /* REMOTEOBJECT_H_ */
