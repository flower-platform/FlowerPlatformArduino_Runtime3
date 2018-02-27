#ifndef ROLLMANPIDLINEFOLLOWER_H_
#define ROLLMANPIDLINEFOLLOWER_H_

#include <Arduino.h>
#include <HardwareSerial.h>
#include "RobotControlService.h"
#include "RollmanLineFollower.h"

#define BASE_SPEED 120
#define MAX_SPEED 255
#define MIN_SPEED 0
#define Kp 4
#define Kd 3
#define Ki 2

class RollmanPIDLineFollower : public RollmanLineFollower {
protected:

	int lastError = 0, sum = 0;

public:

	void lineFollow();

};

void RollmanPIDLineFollower::lineFollow() {
	robotControlService->readSensors(this, [](void* self, int sensorBits) {
		Serial.print("sensorBits: "); Serial.println(sensorBits);
		RollmanPIDLineFollower* lf = (RollmanPIDLineFollower*) self;

		if (sensorBits == 0) {
			return;
		} else if ((sensorBits & _BV(3)) || (sensorBits & _BV(4))) {
			lf->robotControlService->setMotorSpeeds(BASE_SPEED, BASE_SPEED);
			lf->sum = 0;
		} else {
			int activeSensor;
			for (activeSensor = 0; activeSensor < 8 && !(sensorBits & _BV(activeSensor)); activeSensor++);
			Serial.print("activeSensor:"); Serial.println(activeSensor);

			int error = 3.5 - activeSensor;
			lf->sum += error;
			if (lf->sum > 1000) {
				lf->sum = 1000;
			}

			Serial.print(" error:"); Serial.print(error);
			int correction = Kp * error + Kd * (error - lf->lastError) + Ki * lf->sum;
			Serial.print(" correction:"); Serial.print(correction);
			lf->lastError = error;

			int left = BASE_SPEED - correction;
			if (left > MAX_SPEED) {
				left = MAX_SPEED;
			} else if (left < MIN_SPEED) {
				left = MIN_SPEED;
			}

			int right = BASE_SPEED + correction;
			if (right > MAX_SPEED) {
				right = MAX_SPEED;
			} else if (right < MIN_SPEED) {
				right = MIN_SPEED;
			}

			Serial.print(" left: "); Serial.print(left);
			Serial.print(" right: "); Serial.print(right);
			Serial.println();

			lf->robotControlService->setMotorSpeeds(left, right);
		}
	});
}

#endif /* ROLLMANPIDLINEFOLLOWER_H_ */
