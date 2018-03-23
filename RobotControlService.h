#ifndef ROBOT_CONTROL_SERVICE_H_
#define ROBOT_CONTROL_SERVICE_H_

#include <FlowerPlatformArduinoRuntime.h>
#include <RemoteObject.h>
#include <RemoteObjectProtocol.h>
#include <stddef.h>
#include <SerialRemoteObject.h>
#include <SmartBuffer.h>
#include <functional>

class RobotControlService : public SerialRemoteObject {
public:

	RobotControlService(const char* nodeIdPSTR, const char* objectNamePSTR, const char* securityTokenPSTR, RS485Serial* rs485)
		: SerialRemoteObject(nodeIdPSTR, objectNamePSTR, securityTokenPSTR, rs485)  { }

	void readSensors(void* self, void (*callback)(int errorCode, void* self, int)) {
		callFunction(PSTR("readSensors"), NULL, (void*) callback, TYPE_INT, self);
	}

	void getVersion(void* self, void (*callback)(int errorCode, void* self, int)) {
		callFunction(PSTR("getVersion"), NULL, (void*) callback, TYPE_INT, self);
	}

	void init(const char* robotModel, void* self, void (*callback)(int errorCode, void* self, bool)) {
		uint8_t argsBufArray[64]; SmartBuffer argsBuf(argsBufArray, 64);
		argsBuf.print(robotModel); argsBuf.print(TERM);
		callFunction(PSTR("init"), &argsBuf, (void*) callback, TYPE_BOOL, self);
	}

	void setMotorSpeeds(int left, int right) {
		uint8_t argsBufArray[64]; SmartBuffer argsBuf(argsBufArray, 64);
		argsBuf.print(left); argsBuf.print(TERM);
		argsBuf.print(right); argsBuf.print(TERM);
		callFunction(PSTR("setMotorSpeeds"), &argsBuf);
	}

};

#endif /* ROBOT_CONTROL_SERVICE_H_ */
