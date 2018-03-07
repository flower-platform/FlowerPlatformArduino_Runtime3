#ifndef MBOTLINEFOLLOWER_H_
#define MBOTLINEFOLLOWER_H_

#include <HardwareSerial.h>
#include "LineFollower.h"
#include "RobotControlService.h"

#define BASE_SPEED 160
#define SPEED_CORRECTION 70

class MbotLineFollower : public LineFollower {
protected:

	int lastSensorBits;

public:

	void begin();

	void lineFollow();

	void forward();

	void stop();

	void rotateClockwise();

	void rotateCounterClockwise();

	void searchRoad(int roadNumber);

};

void MbotLineFollower::begin() {
	LineFollower::begin("mbot");
}

void MbotLineFollower::lineFollow() {
	robotControlService->readSensors(this, [](void* self, int sensorBits) {
//		Serial.print("sensorBits: "); Serial.println(sensorBits);
		MbotLineFollower* lf = (MbotLineFollower*) self;
		if (sensorBits == 0 && lf->lastSensorBits == 3) {
			lf->stop();
		} else if (sensorBits == 3) { // both sensors over the line
			lf->robotControlService->setMotorSpeeds(BASE_SPEED, BASE_SPEED);
		} else if (sensorBits == 1) {
//			lf->robotControlService->setMotorSpeeds(BASE_SPEED + SPEED_CORRECTION, BASE_SPEED - SPEED_CORRECTION);
			lf->robotControlService->setMotorSpeeds(BASE_SPEED + SPEED_CORRECTION/2, BASE_SPEED - SPEED_CORRECTION);
//			lf->robotControlService->setMotorSpeeds(BASE_SPEED, BASE_SPEED - SPEED_CORRECTION);
		} else if (sensorBits == 2) {
//			lf->robotControlService->setMotorSpeeds(BASE_SPEED - SPEED_CORRECTION, BASE_SPEED + SPEED_CORRECTION);
			lf->robotControlService->setMotorSpeeds(BASE_SPEED - SPEED_CORRECTION, BASE_SPEED + SPEED_CORRECTION/2);
//			lf->robotControlService->setMotorSpeeds(BASE_SPEED - SPEED_CORRECTION, BASE_SPEED);
		}
		lf->lastSensorBits = sensorBits;
	});
}

void MbotLineFollower::forward() {
	robotControlService->setMotorSpeeds(BASE_SPEED, BASE_SPEED);
	delay(200);
}

void MbotLineFollower::stop() {
	robotControlService->setMotorSpeeds(0, 0);
}

void MbotLineFollower::rotateClockwise() {
	Serial.println("Rotating CW...");
	robotControlService->setMotorSpeeds(0.60 * BASE_SPEED, -0.60 * BASE_SPEED);
}

void MbotLineFollower::rotateCounterClockwise() {
	Serial.println("Rotating CCW...");
	robotControlService->setMotorSpeeds(-0.60 * BASE_SPEED, 0.60 * BASE_SPEED);
}

void MbotLineFollower::searchRoad(int roadNumber) {
    while (roadNumber > 0) {
		keepLoop = true;
		while (keepLoop) {
			robotControlService->readSensors(this, [](void* self, int sensorBits) {
				MbotLineFollower* lf = (MbotLineFollower*) self;
				lf->keepLoop = (sensorBits != 0);
			});
			yield();
		}
		keepLoop = true;
		while (keepLoop) {
			robotControlService->readSensors(this, [](void* self, int sensorBits) {
				MbotLineFollower* lf = (MbotLineFollower*) self;
				lf->keepLoop = (sensorBits != 3);
			});
			yield();
		}
		roadNumber--;
	}
	robotControlService->setMotorSpeeds(0, 0);
}

#endif /* MBOTLINEFOLLOWER_H_ */
