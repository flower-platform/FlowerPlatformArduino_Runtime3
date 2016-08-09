/*
 *  Author: Claudiu Matei
 */

 #ifndef Timer_h
#define Timer_h

#include <Arduino.h>
#include <FlowerPlatformArduinoRuntime.h>

class Timer;

class TimerEvent {
public:

	Timer* timer;

};

class Timer {
protected:

	unsigned long lastTimestamp = 0;

	unsigned int currentCount = 0;

	bool started;

public:

	/*
	 * @flower { constructorVariant="Default" }
	 */
	Timer(bool autoStart);

	Callback<TimerEvent>* onTimer = NULL;

	Callback<TimerEvent>* onTimerComplete = NULL;

	unsigned long delay = 1000;

	unsigned int repeatCount = 0;

	void loop();

	void reset();

	void start();

	void stop();

};

Timer::Timer(bool autoStart) {
	started = autoStart;
}

void Timer::loop() {
	if (!started) {
		return;
	}
	if (repeatCount > 0 && currentCount > repeatCount) {
		started = false;
		return;
	}
	if (millis() > lastTimestamp + delay) {
		currentCount++;

		TimerEvent event;
		if (onTimer != NULL) {
			(*onTimer)(&event);
		}

		if (repeatCount > 0 && currentCount == repeatCount) {
			if (onTimerComplete != NULL) {
				(*onTimerComplete)(&event);
			}
			started = false;
		}
		lastTimestamp = millis();
	}

}

void Timer::reset() {
	started = false;
	currentCount = 0;
}

void Timer::start() {
	lastTimestamp = millis();
	started = true;
}

void Timer::stop() {
	started = false;
}

#endif
