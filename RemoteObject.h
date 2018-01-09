/**
 * @author Claudiu Matei
 *
 */

#ifndef REMOTEOBJECT_H_
#define REMOTEOBJECT_H_

#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>
#include <RemoteObjectProtocol.h>
#include <SmartBuffer.h>
#include <Stream.h>

#define MAX_CALLBACKS 4

#define TYPE_VOID 0
#define TYPE_STRING 1
#define TYPE_INT 2
#define TYPE_BOOL 3

#define DEFAULT_BUFFER_SIZE 128

extern bool dispatchFunctionCall(char* functionCall, Print* response);
extern void registerCallback(uint16_t callbackId, void* callback, uint8_t returnTypeId);
extern bool executeCallback(uint16_t callbackId, Stream *response);
extern bool executeCallback(void* callback, uint8_t returnType, Stream *response);

class RemoteObject {
public:

	RemoteObject(const char* rappInstancePSTR, const char* instanceNamePSTR, const char* securityTokenPSTR) {
		this->rappInstance = rappInstancePSTR;
		this->instanceName = instanceNamePSTR;
		this->securityToken = securityTokenPSTR;
	};

	virtual ~RemoteObject() { }

protected:

	const char* rappInstance;

	const char* instanceName;

	const char* securityToken;

	bool callFunction(const char* functionNamePSTR, SmartBuffer<>* argsBuf, void *callback, uint8_t returnTypeId);

	virtual Stream* sendRequest(SmartBuffer<DEFAULT_BUFFER_SIZE>* buf, SmartBuffer<>* argsBuf) = 0;

};

bool RemoteObject::callFunction(const char* functionNamePSTR, SmartBuffer<>* argsBuf, void *callback, uint8_t returnTypeId) {
	SmartBuffer<DEFAULT_BUFFER_SIZE> buf;
	fprp_startPacket(&buf, 'I', securityToken);
	if (rappInstance) {
		write_P(&buf, rappInstance);
	}
	buf.print(TERM); // rappInstance
	buf.print(TERM); // callbackId = null
	if (instanceName != NULL) {
		write_P(&buf, instanceName); buf.print('.');
	}
	write_P(&buf, functionNamePSTR); buf.print(TERM);

	if (argsBuf) {
		argsBuf->print(EOT);
	} else {
		buf.print(EOT);
	}

	Stream* in = sendRequest(&buf, argsBuf);
	if (in == NULL) {
		return false;
	}

	// *** read response ***

	int cmd = fprp_readCommand(in, securityToken);
	if (cmd < 0) {
		Serial.print("Error reading command: "); Serial.println(cmd);
		return false;
	}
	char callbackIdStr[8];
	switch(cmd) {
	case 'R': // result
		in->readBytesUntil(TERM, callbackIdStr, 8); // callbackId (ignored)
		executeCallback(callback, returnTypeId, in);
		break;
	case 'P': // pending
		int size = in->readBytesUntil(TERM, callbackIdStr, 8); // callbackId
		callbackIdStr[size] = TERM;
		registerCallback((uint16_t) atol(callbackIdStr), callback, returnTypeId);
		break;
	}

	return true;
}

#endif /* REMOTEOBJECT_H_ */
