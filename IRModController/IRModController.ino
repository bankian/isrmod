// Ian R Dec 17
// Note: in the control chain library if you want to be able to read the label values, in config.h comment-out the lines #define CC_STRING_NOT_SUPPORTED - see my forum post - https://forum.moddevices.com/t/error-reading-control-labels-from-arduino-shield/1945/2
// V0.1 - Basic control from a single knob
// V0.2 - Display value of a single label (after enabling strings in config.h)
// V0.3 - After much frustration discovered that to make it work with more thean 4 actuators you need to edit #define CC_MAX_ACTUATORS in config.h

#include <Wire.h> 
#include <ControlChain.h>
#include <LiquidCrystal_I2C.h>

#define amountOfPorts 8	//amount of actuators connected 
#define amountOfPotentiometers 4
#define amountOfButtons 4

//LCDs used
#define lines 4
#define characters 20
#define buttonOffset 3

float	buttonValue[amountOfButtons];
int		buttonPin = 10;
int		buttonState;             // the current reading from the input pin
int		lastButtonState[amountOfButtons];  // the previous reading from the input pin

								 // the following variables are unsigned long's because the time, measured in miliseconds,
								 // will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime[amountOfButtons];  // the last time the output pin was toggled
unsigned long debounceDelay = 20; // the debounce time; increase if the output flickers 

float potValues[amountOfPotentiometers], actuatorValues[amountOfPorts], maxValues[amountOfPorts], minValues[amountOfPorts];
char  actuatorNames[amountOfPorts][20];

ControlChain cc;

// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x3F, characters, lines); // set the LCD address to 0x3F for a 20 chars and 4 line display

void setup() {

	SetupPins();	// Setup the pins for button input										 
	cc.begin();		// initialize control chain
					
	const char *uri = "https://www.rogersons.net/ModControl"; // define device name (1st parameter) and its URI (2nd parameter).  The URI must be an unique identifier for your device. A good practice is to use a URL pointing to your project's code or documentation
	cc_device_t *device = cc.newDevice("ISR", uri);

//	cc_actuator_config_t actuator_config;

//	cc_actuator_t *actuator;
	cc_actuator_t *buttonactuator;
	String actuatorlabel;

	char charBuf[20];

	for (int i = 0; i < amountOfPotentiometers; i++) {
		cc_actuator_config_t actuator_config;		
//		Setup continuous controllers here
		if (i < 2) {
			actuatorlabel = "Knob ";
			actuatorlabel += i + 1;
		}
		else {
			actuatorlabel = "Pedal ";
			actuatorlabel += i - 1;
		}
		actuatorlabel.toCharArray(charBuf, 20);
		actuator_config.name = charBuf;

		actuator_config.type = CC_ACTUATOR_CONTINUOUS;						
		actuator_config.value = &potValues[i];
		actuator_config.min = 0.0;
		actuator_config.max = 1023.0;
		actuator_config.supported_modes = CC_MODE_REAL | CC_MODE_INTEGER;
		actuator_config.max_assignments = 1;
		cc_actuator_t *actuator;
		actuator = cc.newActuator(&actuator_config);
		cc.addActuator(device, actuator);		
	}
	
	for (int i = 0; i < amountOfButtons; i++) {
		cc_actuator_config_t button_config;
		// Setup buttons here
		actuatorlabel = "Button ";
		actuatorlabel += i + 1;
		actuatorlabel.toCharArray(charBuf, 20);
		button_config.name = charBuf;

		button_config.type = CC_ACTUATOR_MOMENTARY;
		button_config.value = &buttonValue[i]; //&buttonValue;
		button_config.min = 0.0;
		button_config.max = 1.0;
		button_config.supported_modes = CC_MODE_TOGGLE | CC_MODE_TRIGGER;
		button_config.max_assignments = 1;
		buttonactuator = cc.newActuator(&button_config);
		cc.addActuator(device, buttonactuator);
	}

	lcd.init();			//initialize the lcd
	lcd.backlight();	//open the backlight

			//static_cast<cc_assignment_t*>(updateNames));
			//static_cast<FilterAuthenticate*>(eventData) 

			//startupmessage();

	//set event callbacks
	//cc.setEventCallback(CC_EV_UPDATE, updateValues);
	//cc.setEventCallback(CC_EV_ASSIGNMENT, updateNames);
	//cc.setEventCallback(CC_EV_UNASSIGNMENT, clearlcd);
}


String val2;

void loop() {
	//lcd.clear();

	readButtons();
	readpots();
	//displayInfo();

	cc.run();
	//delay(50);
}

void readButtons()
{
	// read button state
	// state =  1 -> button pressed
	// state = -1 -> button released
	// state =  0 -> same state as before (no change)
	for (int i = 0; i < amountOfButtons; i++)
	{
		readButton(i);
		int state = lastButtonState[i];
		if (state == 1) {
			buttonValue[i] = 1.0;
		}
		else //if (state == -1) 
		{
			buttonValue[i] = 0.0;
		}
	}
}

int readButton(int buttonid) {
	// read the state of the switch into a local variable:
	int reading = digitalRead(buttonid+buttonOffset);

	//lcd.setCursor( buttonid * 2,1);
	//lcd.print(reading);

	 //check to see if you just pressed the button
	 //(i.e. the input went from LOW to HIGH),  and you've waited
	 //long enough since the last press to ignore any noise:

	// If the switch changed, due to noise or pressing:
	if (reading != lastButtonState[buttonid]) {
		// reset the debouncing timer
		lastDebounceTime[0] = millis();
	}

	if ((millis() - lastDebounceTime[buttonid]) > debounceDelay) {
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:

		// if the button state has changed:
		if (reading != buttonState) {
			buttonState = reading;

			// save button last state
			lastButtonState[buttonid] = reading;

			// button pressed
			if (buttonState == LOW) {
				return 1;
				// button released
			}
			else {
				return -1;
			}
		}
	}

	lastButtonState[buttonid] = buttonState;
	return 0;
}

//reads all available potentiometers
void  readpots() {
	for (int i; i<amountOfPotentiometers; i++) {
		potValues[i] = analogRead(i);
	}
}

//updates actuator value and calls the write function
void updateValues(cc_assignment_t *assignment) {
	if (assignment->value<4)
	{
		actuatorValues[assignment->actuator_id] = assignment->value;
		//	  writeValues(assignment->actuator_id);
	}
}

void writeValues(cc_assignment_t *assignment)
{

}

void clearlcd(cc_assignment_t *assignment)
{
	//lcd.clear();
	//for (int i=0;i < assignment->label.size;i++)
	//{ 
	//	actuatorNames[assignment->actuator_id][i] = (char)"";
	//}
}

void displayInfo()
{
	lcd.setCursor(0, 0); // set the cursor to column 15, line 1
	lcd.print(val2);
	lcd.setCursor(0, 0);
	lcd.print("k1:" + (String)actuatorNames[0] + " " + (String)actuatorValues[0]);
	lcd.setCursor(0, 1);
	lcd.print("k2:" + (String)actuatorNames[1] + " " + (String)actuatorValues[1]);
	lcd.setCursor(0, 2);
	lcd.print("p1:" + (String)actuatorNames[2] + " " + (String)actuatorValues[2]);
	lcd.setCursor(0, 3);
	lcd.print("p2:" + (String)actuatorNames[3] + " " + (String)actuatorValues[3]);
}

//when assigned to a actuator, gets the needed information and calls the write names function
void updateNames(cc_assignment_t *assignment) {
	minValues[assignment->actuator_id] = assignment->min;
	maxValues[assignment->actuator_id] = assignment->max;

	int len = assignment->label.size;
	if (len >= characters) len = characters - 1;

	//testactname = (String)assignment->label.text;

	for (int i = 0; i < assignment->label.size; i++) {
		for (int i = 0; i < len; i++) {
			actuatorNames[assignment->actuator_id][i] = assignment->label.text[i];
		}
	}
	//writeNames(assignment->actuator_id, assignment->label.size, 0);
	//displayInfo();
}

//write the name of the selected potentiometer
void writeNames(int num, int labelsize, int clr) {
	int len = labelsize;
	if (len >= characters) len = characters - 1;

	switch (num) {
	case 0:
		lcd.setCursor(0, 0);
		lcd.print("#Pot 1:              ");
		if (clr != 1) {
			lcd.setCursor(0, 3);
			lcd.print("x");//(val3);
			lcd.setCursor(10, 0);
			for (int i = 0; i < len; i++) {
				lcd.print(actuatorNames[num][i]);
			}
		}
		break;
	case 1:
		lcd.setCursor(0, 1);
		lcd.print("#Pot 2:              ");
		if (clr != 1) {
			//u8g.setPrintPos(0, 48);
			lcd.setCursor(3, 1);
			lcd.print("x");//(val3);
			lcd.setCursor(10, 0);
			for (int i = 0; i < num; i++) {
				lcd.print(actuatorNames[num][i]);
			}
		}
		break;
	}
}

void SetupPins()
{
	// configure led
	int ledPin = 13;
	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, LOW);

	for (int i = 0; i < amountOfButtons; i++)
	{
		lastDebounceTime[i] = 0;
		lastButtonState[i] = HIGH;
		pinMode(i+ buttonOffset, INPUT);
		digitalWrite(i + buttonOffset, HIGH);
	}	

	// configure button pin as input and enable internal pullup
	//pinMode(buttonPin, INPUT);
	//digitalWrite(buttonPin, HIGH);

	pinMode(3, INPUT);
	digitalWrite(3, HIGH);
	pinMode(4, INPUT);
	digitalWrite(4, HIGH);
	pinMode(5, INPUT);
	digitalWrite(5, HIGH);
	pinMode(6, INPUT);
	digitalWrite(6, HIGH);
	pinMode(7, INPUT);
	digitalWrite(7, HIGH);
}

void startupmessage() {
	lcd.setCursor(0, 0);
	lcd.print("ISRMOD");
	lcd.setCursor(0, 2);
	lcd.print("Version 0.1");
	delay(2000);
	lcd.clear();
}