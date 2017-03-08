/*
 *  Author: Claudiu Matei
 */

#ifndef TesterComponent_h
#define TesterComponent_h

#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>

class TesterComponent;

class TesterEvent {
public:

	TesterComponent* tester;

};

class TesterComponent {
protected:

	void logEvent(const char* event, const char* eventProperties);

//	unsigned long deadline;

public:

	const char* name;

//	unsigned long delay;

	TesterComponent(const char* name);

	Callback<TesterEvent>* onTesterEvent = NULL;

	void setup();

	void loop();

};

TesterComponent::TesterComponent(const char* name) {
	this->name = name;
//	delay = 500;
//	deadline = millis() + delay;
}

void TesterComponent::loop() {
	Serial.print("{\"eventType\":\"functionCall\",\"functionName\":\"loop\",\"objectClass\":\"TesterComponent\"");
	Serial.print(",\"objectProperties\":{\"name\":\""); Serial.print(name); Serial.print("\"}");
	Serial.println("}");

//	unsigned long currentTime = millis();
//	if (onTesterEvent && (currentTime > deadline)) {
		TesterEvent event;
		event.tester = this;
		(*onTesterEvent)(&event);
//		deadline = currentTime + delay;
//	}
}

void TesterComponent::setup() {
	Serial.print("{\"eventType\":\"functionCall\",\"functionName\":\"setup\",\"objectClass\":\"TesterComponent\"");
	Serial.print(",\"objectProperties\":{\"name\":\""); Serial.print(name); Serial.print("\"}");
	Serial.println("}");
}

void logFunctionCallEvent(const char* functionName, const char* objectClass = NULL, const char* customProperties = NULL) {
	Serial.print("{\"eventType\":\"functionCall\",\"functionName\":\""); Serial.print(functionName); Serial.print("\"");
	if (objectClass) {
		Serial.print(",\"objectClass\":\""); Serial.print(objectClass); Serial.print("\"");
	}
	if (customProperties) {
		Serial.print(",");
		Serial.print(customProperties);
	}
	Serial.println("}");
}



#endif
