class MyComponent {
protected:
	int shouldntAppear1;
	int shouldntAppear2;

public:
	int attribute1;
	int attribute2;

	void (*event1)(ValueChangedEvent*) = NULL;
	void (*event2)(ValueChangedEvent*) = NULL;
	
	int attribute3;
	int attribute4;
	
	void (*event3)(ValueChangedEvent*) = NULL;
	void (*event4)(ValueChangedEvent*) = NULL;
}