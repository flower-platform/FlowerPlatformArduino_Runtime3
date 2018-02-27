#ifndef LINEFOLLOWER_H_
#define LINEFOLLOWER_H_

#include <Arduino.h>
#include <pins_arduino.h>
#include <SoftwareSerial.h>
#include <RS485Serial.h>
#include "RobotControlService.h"

class LineFollower {
protected:

	SoftwareSerial* serial;

	RS485Serial* rs485;

	RobotControlService* robotControlService;

	bool keepLoop = false;

	void begin(const char* robotModel);

public:

	LineFollower() {
		serial = new SoftwareSerial(D3, D4);
		rs485 = new RS485Serial(serial);
		serial->begin(57600);
		robotControlService = new RobotControlService(NULL, NULL, PSTR("44444444"), rs485);
	}

	virtual void begin() = 0;

	virtual void lineFollow() = 0;

	virtual void forward() = 0;

	virtual void stop() = 0;

	virtual void rotateClockwise() = 0;

	virtual void rotateCounterClockwise() = 0;

	virtual void searchRoad(int roadNumber) = 0;

	virtual ~LineFollower() { }

	void rotateClockwise(int tillRoad)  {
		rotateClockwise();
		searchRoad(tillRoad);
	}

	void rotateCounterClockwise(int tillRoad)  {
		rotateCounterClockwise();
		searchRoad(tillRoad);
	}

};

void LineFollower::begin(const char* robotModel) {
	keepLoop = true;
	while (keepLoop) {
		robotControlService->getVersion(this, [](void* self, int version) {
			Serial.print("Control service version: "); Serial.println(version);
			((LineFollower*)self)->keepLoop = false;
		});
	}
	robotControlService->init(robotModel, this, [](void*, bool res) {
		Serial.print("Init: "); Serial.println(res);
	});
}

#endif /* LINEFOLLOWER_H_ */
