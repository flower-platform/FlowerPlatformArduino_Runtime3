#ifndef ROLLMANLINEFOLLOWER_H_
#define ROLLMANLINEFOLLOWER_H_

#include <HardwareSerial.h>
#include "LineFollower.h"
#include "RobotControlService.h"

#define BASE_SPEED 120
//#define SPEED_CORRECTION 70

class RollmanLineFollower : public LineFollower {
public:

	void begin();

	virtual void lineFollow();

	void stop();

	void rotateClockwise();

	void rotateCounterClockwise();

};

void RollmanLineFollower::begin() {
	LineFollower::begin("rollman");
}

void RollmanLineFollower::lineFollow() {
	robotControlService->readSensors(this, [](void* self, int sensorBits) {
		Serial.print("sensorBits: "); Serial.println(sensorBits);
		RollmanLineFollower* lf = (RollmanLineFollower*) self;
		if (sensorBits == 0) {
			return;
		} else if (sensorBits == 8 || sensorBits == 16) { // centered
			lf->robotControlService->setMotorSpeeds(BASE_SPEED, BASE_SPEED);
		} else if (sensorBits > 16) {
			lf->robotControlService->setMotorSpeeds(BASE_SPEED + 50, BASE_SPEED - 50);
		} else if (sensorBits < 8) {
			lf->robotControlService->setMotorSpeeds(BASE_SPEED - 50, BASE_SPEED + 50);
		}
	});
}

void RollmanLineFollower::stop() {
	for (int i = 100; i >= 0; i -= 10) {
		robotControlService->setMotorSpeeds(i, i);
	}
	robotControlService->setMotorSpeeds(0, 0);
}

void RollmanLineFollower::rotateClockwise() {
	Serial.println("Rotating CW...");
	robotControlService->setMotorSpeeds(3 * BASE_SPEED / 4, -3 * BASE_SPEED / 4);
	keepLoop = true;
	while (keepLoop) {
		robotControlService->readSensors(this, [](void* self, int sensorBits) {
			RollmanLineFollower* lf = (RollmanLineFollower*) self;
			lf->keepLoop = (sensorBits != 128 );
		});
		yield();
	}
	keepLoop = true;
	while (keepLoop) {
		robotControlService->readSensors(this, [](void* self, int sensorBits) {
			RollmanLineFollower* lf = (RollmanLineFollower*) self;
			lf->keepLoop = (sensorBits != 8 && sensorBits != 16);
		});
		yield();
	}
	robotControlService->setMotorSpeeds(0, 0);
}

void RollmanLineFollower::rotateCounterClockwise() {

}

#endif /* ROLLMANLINEFOLLOWER_H_ */
