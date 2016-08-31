/*
 *  Author: Claudiu Matei
 */

#ifndef Output_h
#define Output_h

#include <Arduino.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <Print.h>


class Output {
protected:

	int lastValue;

	uint8_t pin;

	bool isPwm;

public:
	// TODO CS: TEMP
	bool contributesToState;

	Callback<ValueChangedEvent>* onValueChanged = NULL;

	/*
	 * @flower { constructorVariant="Default" }
	 */
	Output(int pin, bool isPwm = false, uint8_t initialValue = LOW);

	void printStateAsJson(const __FlashStringHelper* instanceName, Print* print);

	void setHigh();

	void setLow();

	void setValue(int value);

	int getValue();

	void toggleHighLow();

};

Output::Output(int pin, bool isPwm, uint8_t initialValue) {
	this->pin = pin;
    this->isPwm = isPwm;
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
