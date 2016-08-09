/*
 *  Author: Claudiu Matei
 */

#ifndef Input_h
#define Input_h

#include <Arduino.h>
#include <Print.h>
#include <WString.h>
#include <FlowerPlatformArduinoRuntime.h>

class Input {
protected:

	int lastValue = 0;

	unsigned long lastTime = 0;

	uint8_t pin;

	unsigned int pollInterval = 50;

	bool isAnalog = false;


public:
	// TODO CS: TEMP
	bool contributesToState;

	Callback<ValueChangedEvent>* onValueChanged = NULL;

	/*
	 * @flower { constructorVariant="Default" }
	 */
	Input(int pin, bool isAnalog, bool internalPullUp, int pollInterval);

	void loop();

	void printStateAsJson(const __FlashStringHelper* instanceName, Print* print);

};

Input::Input(int pin, bool isAnalog = false, bool internalPullUp = false, int pollInterval = 50) {
	this->pin = pin;
	this->isAnalog = isAnalog;
	this->pollInterval = pollInterval;
	pinMode(pin, INPUT);
	if (internalPullUp) {
		digitalWrite(pin, HIGH);
		lastValue = HIGH;
	} else {
		lastValue = isAnalog ? analogRead(pin) : digitalRead(pin);
	}
}

void Input::loop() {
	int value = isAnalog ? analogRead(pin) : digitalRead(pin);
	if (value == lastValue) {
    	return;
    }
	if (!isAnalog && (millis() - lastTime < pollInterval)) {
		return;
	}

	if (onValueChanged != NULL) {
		ValueChangedEvent event;
		event.previousValue = lastValue;
		event.currentValue = value;
		(*onValueChanged)(&event);
	}

	lastValue = value;

	if (!isAnalog) {
		lastTime = millis();
	}

}

void Input::printStateAsJson(const __FlashStringHelper* instanceName, Print* print) {
	print->print(F("\""));
	print->print(instanceName);
	print->print(F("\":"));
	print->print(lastValue);
}

#endif
