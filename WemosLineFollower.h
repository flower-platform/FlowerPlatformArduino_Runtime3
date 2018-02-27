#ifndef WEMOSLINEFOLLOWER_H_
#define WEMOSLINEFOLLOWER_H_

#include <HardwareSerial.h>
#include "LineFollower.h"
#include "WEMOS_Motor.h"

#define BASE_SPEED 40
#define SPEED_CORRECTION 10

//Motor shiled I2C Address: 0x30
//PWM frequency: 1000Hz(1kHz)
Motor M1(0x30,_MOTOR_A, 100);//Motor A
Motor M2(0x30,_MOTOR_B, 100);//Motor B

class WemosLineFollower : public LineFollower {
protected:

	uint8_t readSensors();

	void setMotorSpeeds(int left, int right);

public:

	WemosLineFollower() { };

	void begin() {
		pinMode(A0, INPUT);
		pinMode(D0, INPUT);
//		while(true) {
//			Serial.print(digitalRead(D0)); Serial.print(" "); Serial.println(analogRead(A0));
//			forward();
//			delay(1000);
//			rotateClockwise();
//			delay(1000);
//			rotateCounterClockwise();
//			delay(1000);
//			stop();
//			delay(1000);
//		}
	};

	void lineFollow();

	void forward();

	void stop();

	void rotateClockwise();

	void rotateCounterClockwise();

	void searchRoad(int roadNumber);

};

uint8_t WemosLineFollower::readSensors() {
	uint8_t sensorBits = 0;
	sensorBits |= digitalRead(D0);
	if (analogRead((unsigned char)A0) > 500) {
		sensorBits |= 2;
	}
	return sensorBits;
}

void WemosLineFollower::setMotorSpeeds(int left, int right) {
	if (left >= 0) {
		M1.setmotor(_CW, left);
	} else {
		M1.setmotor(_CCW, left);
	}
	if (right >= 0) {
		M2.setmotor(_CW, right);
	} else {
		M2.setmotor(_CCW, right);
	}
}

void WemosLineFollower::lineFollow() {
	uint8_t sensorBits = readSensors();
	Serial.print("sensorBits: "); Serial.println(sensorBits);
	if (sensorBits == 0) {
		return;
	} else if (sensorBits == 3) { // both sensors over the line
		setMotorSpeeds(BASE_SPEED, BASE_SPEED);
	} else if (sensorBits == 1) {
		setMotorSpeeds(BASE_SPEED + SPEED_CORRECTION, BASE_SPEED - SPEED_CORRECTION);
	} else if (sensorBits == 2) {
		setMotorSpeeds(BASE_SPEED - SPEED_CORRECTION, BASE_SPEED + SPEED_CORRECTION);
	}
}

void WemosLineFollower::forward() {
	setMotorSpeeds(BASE_SPEED, BASE_SPEED);
	delay(200);
}

void WemosLineFollower::stop() {
	M1.setmotor(_STOP);
	M2.setmotor(_STOP);
}

void WemosLineFollower::rotateClockwise() {
	Serial.println("Rotating CW...");
	setMotorSpeeds(0.6 * BASE_SPEED, -0.6 * BASE_SPEED);
}

void WemosLineFollower::rotateCounterClockwise() {
	Serial.println("Rotating CCW...");
	setMotorSpeeds(-0.6 * BASE_SPEED, 0.6 * BASE_SPEED);
}

void WemosLineFollower::searchRoad(int roadNumber) {
    while (roadNumber > 0) {
		keepLoop = true;
		while (keepLoop) {
			robotControlService->readSensors(this, [](void* self, int sensorBits) {
				WemosLineFollower* lf = (WemosLineFollower*) self;
				lf->keepLoop = (sensorBits != 0);
			});
			yield();
		}
		keepLoop = true;
		while (keepLoop) {
			robotControlService->readSensors(this, [](void* self, int sensorBits) {
				WemosLineFollower* lf = (WemosLineFollower*) self;
				lf->keepLoop = (sensorBits != 3);
			});
			yield();
		}
		roadNumber--;
	}
	robotControlService->setMotorSpeeds(0, 0);
}

#endif /* WEMOSLINEFOLLOWER_H_ */
