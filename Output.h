/*
 *  Author: Claudiu Matei
 */

#ifndef Output_h
#define Output_h

#include <FlowerPlatformArduinoRuntime.h>

class Output {
protected:

	int lastValue;

public:
	// TODO CS: TEMP
	bool contributesToState;

	uint8_t pin;
	uint8_t initialValue = LOW;

	Callback<ValueChangedEvent>* onValueChanged = NULL;

	bool isPwm = false;

	void loop();

	void setup();

	void printStateAsJson(const __FlashStringHelper* instanceName, Print* print);

	void setHigh();

	void setLow();

	void setValue(int value);

	int getValue();

	void toggleHighLow();

};

void Output::loop() {
}

void Output::setup() {
    pinMode(pin, OUTPUT);
	digitalWrite(pin, initialValue);
    lastValue = initialValue;
}

void Output::printStateAsJson(const __FlashStringHelper* instanceName, Print* print) {
	print->print(F("\""));
	print->print(instanceName);
	print->print(F("\": "));
	print->print(lastValue);
}

void Output::setHigh() {
	setValue(HIGH);
}

void Output::setLow() {
	setValue(LOW);
}

void Output::setValue(int value) {
	if (isPwm) {
		analogWrite(pin, value);
	} else {
		digitalWrite(pin, value);
	}

	if (onValueChanged != NULL) {
		ValueChangedEvent event;
		event.previousValue = lastValue;
		event.currentValue = value;
		(*onValueChanged)(&event);
	}

	lastValue = value;
}

int Output::getValue() {
	return lastValue;
}

void Output::toggleHighLow() {
	if (lastValue) {
		setValue(LOW);
	} else {
		setValue(HIGH);
	}
}

#endif
