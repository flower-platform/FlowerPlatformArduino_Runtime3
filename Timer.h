#ifndef Timer_h
#define Timer_h

#include <Arduino.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <stdbool.h>
#include <stddef.h>

class Timer;

class TimerEvent {
public:

	Timer* timer;

};

class Timer {
protected:

	unsigned long lastTimestamp;

public:

	Callback<TimerEvent>* onTimer = NULL;

	Callback<TimerEvent>* onTimerComplete = NULL;

	unsigned int currentCount;

	unsigned long delay;

	unsigned int repeatCount;

	bool autoStart;

	void setup();

	void loop();

	void reset();

	void start();

	void stop();

};

void Timer::setup() { }

void Timer::loop() {
	if (!autoStart) {
		return;
	}
	if (repeatCount > 0 && currentCount > repeatCount) {
		autoStart = false;
		return;
	}
	if (millis() > lastTimestamp + delay) {
		currentCount++;

		TimerEvent event;
		if (onTimer != NULL) {
			onTimer(&event);
		}

		if (repeatCount > 0 && currentCount == repeatCount) {
			if (onTimerComplete != NULL) {
				onTimerComplete(&event);
			}
			autoStart = false;
		}
		lastTimestamp = millis();
	}

}

void Timer::reset() {
	autoStart = false;
	currentCount = 0;
}

void Timer::start() {
	lastTimestamp = millis();
	autoStart = true;
}

void Timer::stop() {
	autoStart = false;
}

#endif
