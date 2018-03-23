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

	uint8_t BASE_SPEED = 0;

	uint8_t orientation = 0;

	LineFollower() {
		serial = new SoftwareSerial(D3, D4);
		rs485 = new RS485Serial(serial);
		serial->begin(19200);
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

	void rotate(uint8_t newOrientation) {
		int diff = newOrientation - orientation;
		if (abs(diff + 4) < abs(diff)) {
			diff += 4;
		} else if (abs(diff - 4) < abs(diff)) {
			diff -= 4;
		}

		if (diff > 0) {
			rotateClockwise(diff);
		} else if (diff < 0) {
			rotateCounterClockwise(-diff);
		}
		orientation = newOrientation;
	}

	void rotateClockwise(int tillRoad)  {
		rotateClockwise();
		searchRoad(tillRoad);
	}

	void rotateCounterClockwise(int tillRoad)  {
		rotateCounterClockwise();
		searchRoad(tillRoad);
	}

	void setBaseSpeed(uint8_t speed) {
		BASE_SPEED = speed;
	}

};

void LineFollower::begin(const char* robotModel) {
	keepLoop = true;
	while (keepLoop) {
		robotControlService->getVersion(this, [](int, void* self, int version) {
			Serial.print("Control service version: "); Serial.println(version);
			((LineFollower*)self)->keepLoop = false;
		});
	}
	robotControlService->init(robotModel, this, [](int, void*, bool res) {
		Serial.print("Init: "); Serial.println(res);
	});
}

#endif /* LINEFOLLOWER_H_ */
