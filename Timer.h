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
	void (*onTimer)(TimerEvent*) = NULL;
	void (*onTimerComplete)(TimerEvent*) = NULL;

	unsigned int currentCount;

	unsigned long delay;

	unsigned int repeatCount;

	bool autoStart;

	void setup() {
	}

	void loop() {
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

	void reset() {
		autoStart = false;
		currentCount = 0;
	}

	void start() {
		lastTimestamp = millis();
		autoStart = true;
	}

	void stop() {
		autoStart = false;
	}

};

#endif
