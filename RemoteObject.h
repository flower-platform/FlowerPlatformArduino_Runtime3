/**
 * @author Claudiu Matei
 *
 */

#ifndef REMOTEOBJECT_H_
#define REMOTEOBJECT_H_

#ifdef ESP8266
#define PSTR(X) X
#define strcmp_P(str1, st2P) strcmp(str1, st2P)
#endif

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
void registerCallback(uint16_t callbackId, void* self, void* callback, uint8_t returnTypeId);
bool executeCallback(uint16_t callbackId, Stream *response);
bool executeCallback(void* self, void* callback, uint8_t returnType, Stream *response);

class RemoteObject {
public:

	RemoteObject(const char* remoteNodeIdPSTR, const char* objectNamePSTR, const char* securityTokenPSTR) {
		this->remoteNodeIdPSTR = remoteNodeIdPSTR;
		this->objectNamePSTR = objectNamePSTR;
		this->securityTokenPSTR = securityTokenPSTR;
	};

	virtual ~RemoteObject() { }

	bool callFunction(const char* functionNamePSTR, SmartBuffer* argsBuf, void *callback, uint8_t returnTypeId, void* self);

protected:

	const char* remoteNodeIdPSTR;

	const char* objectNamePSTR;

	const char* securityTokenPSTR;

	virtual Stream* sendRequest(SmartBuffer* buf) = 0;

};

bool RemoteObject::callFunction(const char* functionNamePSTR, SmartBuffer* argsBuf, void *callback = NULL, uint8_t returnTypeId = TYPE_VOID, void* self = NULL) {
	uint8_t bufArray[DEFAULT_BUFFER_SIZE];
	SmartBuffer buf(bufArray, DEFAULT_BUFFER_SIZE);
	fprp_startPacket(&buf, 'I', securityTokenPSTR);
	if (remoteNodeIdPSTR) {
		buf.write_P(remoteNodeIdPSTR);
	}
	buf.print(TERM); // remoteNodeId
	buf.print(TERM); // callbackId = null
	if (objectNamePSTR != NULL) {
		buf.write_P(objectNamePSTR); buf.print('.');
	}
	write_P(&buf, functionNamePSTR); buf.print(TERM);
	if (argsBuf) {
		argsBuf->flush(&buf);
	}
	buf.print(EOT);

	Stream* in = sendRequest(&buf);
	if (in == NULL) {
		return false;
	}

// *** read response ***

	int cmd = fprp_readCommand(in, securityTokenPSTR);
	if (cmd < 0) {
		Serial.print("Error reading command: "); Serial.println(cmd);
		return false;
	}

	if (callback == NULL) {
		return true;
	}

	char callbackIdStr[8];
	switch(cmd) {
	case 'R': // result
		in->readBytesUntil(TERM, callbackIdStr, 8); // callbackId (ignored)
		executeCallback(self, callback, returnTypeId, in);
		break;
	case 'P': // pending
		int size = in->readBytesUntil(TERM, callbackIdStr, 8); // callbackId
		callbackIdStr[size] = TERM;
		registerCallback((uint16_t) atol(callbackIdStr), self, callback, returnTypeId);
		break;
	}

	return true;
}

struct RemoteObjectCallback {
	uint16_t callbackId;
	void* callbackFunction;
	void* selfObject;
	uint8_t returnType;
} callbacks[MAX_CALLBACKS];

uint8_t callbackIndex = 0;

void registerCallback(uint16_t callbackId, void* self, void* callback, uint8_t returnTypeId) {
	callbacks[callbackIndex].callbackId = callbackId;
	callbacks[callbackIndex].callbackFunction = callback;
	callbacks[callbackIndex].selfObject = self;
	callbacks[callbackIndex].returnType = returnTypeId;
	callbackIndex++;
	if (callbackIndex >= MAX_CALLBACKS) {
		callbackIndex = 0;
	}
}

bool executeCallback(uint16_t callbackId, Stream *response) {
	uint8_t cbIndex = 0;
	while (cbIndex < MAX_CALLBACKS && (callbacks[cbIndex].callbackId != callbackId ) ) {
		cbIndex++;
	}
	if (cbIndex == MAX_CALLBACKS) {
		return false;
	}
	RemoteObjectCallback cb = callbacks[callbackIndex];
	void* callbackFunction = cb.callbackFunction;
	cb.callbackFunction = NULL;
	return executeCallback(cb.selfObject, callbackFunction, cb.returnType, response);
}

bool executeCallback(void* self, void* callback, uint8_t returnType, Stream *response) {
	int size;
	switch (returnType) {
	case TYPE_VOID:
		((void (*)(void*)) callback)(self);
		break;
	case TYPE_INT: {
		char valueStr[8];
		int size = response->readBytesUntil(EOT, valueStr, 8);
		valueStr[size] = TERM;
		int value = atoi(valueStr);
		((void (*)(void*, int)) callback)(self, value);
	} break;
	case TYPE_BOOL: {
		char valueStr[8];
		size = response->readBytesUntil(EOT, valueStr, 8);
		valueStr[size] = TERM;
		int value = atoi(valueStr);
		((void (*)(void*, bool)) callback)(self, value);
	} break;
	case TYPE_STRING: {
		char value[65];
		int size = response->readBytesUntil(EOT, value, 64);
		value[size] = TERM;
		((void (*)(void*, const char*)) callback)(self, value);
	} break;
	}
	return true;
}

#endif /* REMOTEOBJECT_H_ */
