/*
 *  Author: Claudiu Matei
 */

#ifndef DHTSensor_h
#define DHTSensor_h

#include <Arduino.h>
#include <DHT.h>
#include <Print.h>


class DHTSensor {
public:

	Callback<ValueChangedEvent>* onTemperatureChanged = NULL;
	Callback<ValueChangedEvent>* onHumidityChanged = NULL;

	uint8_t pin;
	unsigned int pollInterval;

	void setup() {
		lastTime = 0;
		lastHumidity = -1000;
		lastTemperature = -1000;
		pinMode(pin, INPUT);

		if (pollInterval <= 0) {
			pollInterval = 1000;
		}
	}

	void loop() {
		if ((unsigned long int)(millis() - lastTime) < pollInterval) {
			return;
		}

		lastTime = millis();

		DHT dhtSensor(pin, DHT11);

		double temperature = dhtSensor.readTemperature();
		if (temperature != lastTemperature && onTemperatureChanged != NULL) {
			ValueChangedEvent event;
			event.currentValue = temperature;
			event.previousValue = lastTemperature;
			(*onTemperatureChanged)(&event);
		}
		lastTemperature = temperature;

		double humidity = dhtSensor.readHumidity();
		if (humidity != lastHumidity && onHumidityChanged != NULL) {
			ValueChangedEvent event;
			event.currentValue = humidity;
			event.previousValue = lastHumidity;
			(*onHumidityChanged)(&event);
		}
		lastHumidity = humidity;
	}

	void printStateAsJson(const __FlashStringHelper* instanceName, Print* print) {

		print->print(F("\""));
		print->print(instanceName);
		print->print(F("_temperature"));
		print->print(F("\":"));
		print->print(lastTemperature);

		print->print(F(","));

		print->print(F("\""));
		print->print(instanceName);
		print->print(F("_humidity"));
		print->print(F("\":"));
		print->print(lastHumidity);
	}


protected:
	unsigned long lastTime;
	double lastTemperature;
	double lastHumidity;
};

#endif
