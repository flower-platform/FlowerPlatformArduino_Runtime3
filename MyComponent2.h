class MyComponent2 : public MyComponent {
public:
	int attribute1_2;
	int attribute2_2;

	void (*event1_2)(ValueChangedEvent*) = NULL;
	void (*event2_2)(ValueChangedEvent*) = NULL;
	
	int attribute3_2;

	void (*event3_2)(ValueChangedEvent*) = NULL;
}
