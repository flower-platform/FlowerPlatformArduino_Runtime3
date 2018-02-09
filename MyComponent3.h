#include <Arduino.h>

class MyComponent3 : public Input, public MyComponent2, public MyComponent {
public:
	int attribute1_3;
	int attribute2_3;

	void (*event1_3)(OtherEventType*) = NULL;
	void (*event2_3)(ValueChangedEvent*) = NULL;
	
	int attribute3_3;

	void (*event3_3)(ValueChangedEvent*) = NULL;
};
