#ifndef MBOTLINEFOLLOWER_H_
#define MBOTLINEFOLLOWER_H_

#include <Arduino.h>
#include <HardwareSerial.h>
#include <LineFollower.h>
#include <RobotControlService.h>

class MbotLineFollower : public LineFollower {
protected:

	int lastSensorBits = -1;

	bool running = false;

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
	BASE_SPEED = 120;
}

void MbotLineFollower::lineFollow() {
	robotControlService->readSensors(this, [](int, void* self, int sensorBits) {
		MbotLineFollower* lf = (MbotLineFollower*) self;
		if (sensorBits == lf->lastSensorBits && lf->running) {
			return;
		}
		Serial.print("sensorBits: "); Serial.println(sensorBits);
		int SPEED_CORRECTION = lf->BASE_SPEED * 50 / 100;

		if (sensorBits == 0) {
			if (lf->lastSensorBits == 3) {
				lf->stop();
			} else if (lf->lastSensorBits == 1) {
				lf->robotControlService->setMotorSpeeds(lf->BASE_SPEED * 80 / 100, 0);
			} else if (lf->lastSensorBits == 2) {
				lf->robotControlService->setMotorSpeeds(0, lf->BASE_SPEED  * 80 / 100);
			}
		} else if (sensorBits == 3) { // both sensors over the line
			lf->robotControlService->setMotorSpeeds(lf->BASE_SPEED, lf->BASE_SPEED);
		} else if (sensorBits == 1) {
			lf->robotControlService->setMotorSpeeds(lf->BASE_SPEED, lf->BASE_SPEED - SPEED_CORRECTION);
		} else if (sensorBits == 2) {
			lf->robotControlService->setMotorSpeeds(lf->BASE_SPEED - SPEED_CORRECTION, lf->BASE_SPEED);
		}
		if (lf->running) {
			lf->lastSensorBits = sensorBits;
		} else {
			lf->running = true;
		}
	});
}

/*
void MbotLineFollower::lineFollow() {
	robotControlService->readSensors(this, [](void* self, int sensorBits) {
//		Serial.print("sensorBits: "); Serial.println(sensorBits);
		MbotLineFollower* lf = (MbotLineFollower*) self;
		int left = 0, right = 0;
		if (sensorBits == 0) {
			if (lf->lastCorrection == 3) {  // 3 => both sensors over the line
				lf->stop();
				lf->lastCorrection = 0;
			} else if (lf->lastCorrection == 1) {
				left = lf->BASE_SPEED;
				right = 0;
			} else if (lf->lastCorrection == 2) {
				left = 0;
				right = lf->BASE_SPEED;
			}
		} else {
			left = lf->BASE_SPEED;
			right = lf->BASE_SPEED;
			lf->lastCorrection = sensorBits;
		}
		if (left != lf->lastLeft || right != lf->lastRight) {
			lf->robotControlService->setMotorSpeeds(left, right);
			lf->lastLeft = left;
			lf->lastRight = right;
		}
	});
}
*/

/*
void MbotLineFollower::lineFollow1() {
	robotControlService->readSensors(this, [](void* self, int sensorBits) {
//		Serial.print("sensorBits: "); Serial.println(sensorBits);
		MbotLineFollower* lf = (MbotLineFollower*) self;
		if (sensorBits == 0 && lf->lastSensorBits == 3) {
			lf->stop();
		} else if (sensorBits == 3) { // both sensors over the line
			lf->robotControlService->setMotorSpeeds(lf->BASE_SPEED, lf->BASE_SPEED);
		} else if (sensorBits == 1) {
//			lf->robotControlService->setMotorSpeeds(BASE_SPEED + SPEED_CORRECTION, BASE_SPEED - SPEED_CORRECTION);
//			lf->robotControlService->setMotorSpeeds(BASE_SPEED + SPEED_CORRECTION/2, BASE_SPEED - SPEED_CORRECTION);
//			lf->robotControlService->setMotorSpeeds(BASE_SPEED, BASE_SPEED - SPEED_CORRECTION);
			lf->robotControlService->setMotorSpeeds(0.90 * lf->BASE_SPEED, 0);
//			lf->robotControlService->setMotorSpeeds(lf->BASE_SPEED, 0);
		} else if (sensorBits == 2) {
//			lf->robotControlService->setMotorSpeeds(BASE_SPEED - SPEED_CORRECTION, BASE_SPEED + SPEED_CORRECTION);
//			lf->robotControlService->setMotorSpeeds(BASE_SPEED - SPEED_CORRECTION, BASE_SPEED + SPEED_CORRECTION/2);
			lf->robotControlService->setMotorSpeeds(0, 0.90 * lf->BASE_SPEED);
//			lf->robotControlService->setMotorSpeeds(0, lf->BASE_SPEED);
		}
		lf->lastSensorBits = sensorBits;
	});
}
*/

void MbotLineFollower::forward() {
	robotControlService->setMotorSpeeds(BASE_SPEED, BASE_SPEED);
}

void MbotLineFollower::stop() {
	robotControlService->setMotorSpeeds(0, 0);
	running = false;
}

void MbotLineFollower::rotateClockwise() {
	Serial.println("Rotating CW...");
	robotControlService->setMotorSpeeds(0.75 * BASE_SPEED, -0.75 * BASE_SPEED);
}

void MbotLineFollower::rotateCounterClockwise() {
	Serial.println("Rotating CCW...");
	robotControlService->setMotorSpeeds(-0.75 * BASE_SPEED, 0.75 * BASE_SPEED);
}

void MbotLineFollower::searchRoad(int roadNumber) {
    while (roadNumber > 0) {
		keepLoop = true;
		while (keepLoop) {
			robotControlService->readSensors(this, [](int, void* self, int sensorBits) {
				MbotLineFollower* lf = (MbotLineFollower*) self;
				lf->keepLoop = (sensorBits != 0);
			});
			yield();
		}
		keepLoop = true;
		while (keepLoop) {
			robotControlService->readSensors(this, [](int, void* self, int sensorBits) {
				MbotLineFollower* lf = (MbotLineFollower*) self;
//				lf->keepLoop = (sensorBits != 3);
				lf->keepLoop = (sensorBits == 0);
			});
			yield();
		}
		roadNumber--;
	}
	robotControlService->setMotorSpeeds(0, 0);
}

#endif /* MBOTLINEFOLLOWER_H_ */
