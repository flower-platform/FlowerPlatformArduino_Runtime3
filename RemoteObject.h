/**
 * @author Claudiu Matei
 *
 */

#ifndef REMOTEOBJECT_H_
#define REMOTEOBJECT_H_

#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>
#include <NetworkConnection.h>
#include <RemoteObjectUtils.h>
#include <Stream.h>

#define MAX_CALLBACKS 4
#define TYPE_VOID 0
#define TYPE_STRING 1
#define TYPE_INT 2
#define TYPE_BOOL 3

#define RECV_BUFFER_SIZE 128

extern void registerCallback(uint16_t callbackId, void* callback, uint8_t returnTypeId);
extern uint16_t executeCallback(uint16_t callbackId, Stream *response);
extern uint16_t executeCallback(void* callback, uint8_t returnType, Stream *response);
extern void dispatchFunctionCall(const char* functionCall, Print* response);

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
	buf.print('0'); buf.print(TERM); // hasNext = false
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
	fprp_startCommand(connection->out, 'I', securityToken);
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
	char cmd = fprp_readCommand(connection->in, securityToken);
	if ((int) cmd < 0) {
		Serial.print("Error reading command: "); Serial.println((int) cmd);
		connection->disconnect();
		return false;
	}

	switch(cmd) {
	case 'R': // result
		connection->in->find(TERM); // hasNext (ignored)
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

/***************************************** RemoteObjectServer ********************************************************/

class RemoteObjectServer {
public:

	RemoteObjectServer(const char* securityTokenPSTR) {
		this->securityToken = securityTokenPSTR;
	}

	bool processCommand(Stream* in, Print* out);

protected:

	const char* securityToken;

};

bool RemoteObjectServer::processCommand(Stream* in, Print* out) {
	char cmd = fprp_readCommand(in, securityToken);
	if ((int) cmd < 0) {
		Serial.print("Error reading command: "); Serial.println((int) cmd);
		return false;
	}

	size_t size = 0;
	switch (cmd) {
	case 'I': // INVOKE
		// ---------- read request ------------
		char rbuf[RECV_BUFFER_SIZE];
		size = in->readBytesUntil(TERM, rbuf, RECV_BUFFER_SIZE); // hasNext (ignored)
		in->readBytesUntil(TERM, rbuf, RECV_BUFFER_SIZE); // rappInstanceId (ignored)
		size = in->readBytesUntil(TERM, rbuf, RECV_BUFFER_SIZE); // callbackId
		char callbackIdStr[size + 1];
		strncpy(callbackIdStr, rbuf, size);
		callbackIdStr[size] = TERM;
		size = in->readBytesUntil(EOT, rbuf, RECV_BUFFER_SIZE); // function call
		rbuf[size] = TERM;

		// send initial headers
		write_P(out, PSTR("HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\n")); // HTTP headers

		//buffer command specific packet fields and response
		BufferedPrint<64> tbuf(out);
		tbuf.print('0'); tbuf.print(TERM); // hasNext = false
		tbuf.print(callbackIdStr); tbuf.print(TERM); // callbackId
		dispatchFunctionCall(rbuf, &tbuf);

		// send content length; end headers
		write_P(out, PSTR("Content-Length: ")); out->println(FPRP_FIXED_PACKET_SIZE + tbuf.getSize());
		out->println();

		// send response packet
		fprp_startCommand(out, 'R', securityToken); // command = RESULT
		tbuf.flush();
		fprp_endCommand(out);

		break;
	}
	return true;
}

#endif /* REMOTEOBJECT_H_ */
