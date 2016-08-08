#ifndef Input_h
#define Input_h

#include <Arduino.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <Print.h>
#include <WString.h>




class Input {
protected:

	int lastValue = 0;

	unsigned long lastTime = 0;

	uint8_t pin;

public:
	// TODO CS: TEMP
	bool contributesToState;

	Callback<ValueChangedEvent>* onValueChanged = NULL;

	unsigned int pollInterval = 50;

	bool internalPullUp = false;

	bool isAnalog = false;

	/*
	 * @flower { constructorVariant="Default" }
	 */
	Input(int pin);

	void setup();

	void loop();

	void printStateAsJson(const __FlashStringHelper* instanceName, Print* print);

};

Input::Input(int pin) {
	this->pin = pin;
}

void Input::setup() {
	pinMode(pin, INPUT);
	if (internalPullUp) {
		digitalWrite(pin, HIGH);
		lastValue = HIGH;
	} else {
		lastValue = LOW;
	}
}

void Input::loop() {
	int value;
	if (isAnalog) {
		value = analogRead(pin);
	} else {
		value = digitalRead(pin);
	}
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
