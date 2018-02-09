#include <Arduino.h>

class MyComponent {
protected:
	int shouldntAppear1;
	int shouldntAppear2;

public:
	int attribute1;
	int attribute2;
	
	static int staticAttribute;
	static MyComponent staticAttribute2;

	char* attributePtr;
	MyComponent* attributePtr2;
	
	void (*event1)(ValueChangedEvent*) = NULL;
	void (*event2)(ValueChangedEvent*) = NULL;
	
	int attribute3;

	/*
	 * @flower { ignore = "true" }
	 */
	int attribute4;
	
	void (*event3)(ValueChangedEvent*) = NULL;

	/*
	 * @flower { ignore = "true" }
	 */
	void (*event4)(ValueChangedEvent*) = NULL;
};

/*
 * @flower { ignore = "true" }
 */
class ShouldntAppear {
};
