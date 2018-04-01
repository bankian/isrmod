// Ian R Dec 17
// Note: in the control chain library if you want to be able to read the label values, in config.h comment-out the lines #define CC_STRING_NOT_SUPPORTED - see my forum post - https://forum.moddevices.com/t/error-reading-control-labels-from-arduino-shield/1945/2
// V0.1 - Basic control from a single knob
// V0.2 - Display value of a single label (after enabling strings in config.h)

#include <Wire.h> 
#include <ControlChain.h>
#include <LiquidCrystal_I2C.h>

#define amountOfPorts 5		//amount of actuators connected 
#define amountOfPotentiometers 4

//LCDs used
#define lines 4
#define characters 20

// the following variables are unsigned long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 20; // the debounce time; increase if the output flickers

float	buttonValue;
int		buttonPin = 9;
int		buttonState;             // the current reading from the input pin
int		lastButtonState = HIGH;  // the previous reading from the input pin
 

float potValues[amountOfPotentiometers], actuatorValues[amountOfPorts], maxValues[amountOfPorts], minValues[amountOfPorts];
char  actuatorNames[amountOfPorts][20];

ControlChain cc;

// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x3F, characters, lines); // set the LCD address to 0x3F for a 20 chars and 4 line display

void setup() {

	SetupPins();	// Setup the pins for button input										 
	cc.begin();		// initialize control chain

	// define device name (1st parameter) and its URI (2nd parameter).  The URI must be an unique identifier for your device. A good practice is to use a URL pointing to your project's code or documentation
	const char *uri = "https://www.rogersons.net/ModControl";
	cc_device_t *device = cc.newDevice("ISR", uri);

	for (int i = 0; i < amountOfPorts; i++) {
		cc_actuator_config_t actuator_config;
		String knoblabel;

		if (i < 4)
		{
			// Setup continuous controllers here
			knoblabel = "Analogue ";			
			actuator_config.type = CC_ACTUATOR_CONTINUOUS;
			actuator_config.max = 1023.0;
			actuator_config.supported_modes = CC_MODE_REAL | CC_MODE_INTEGER;
			actuator_config.value = &potValues[i];
		}
		else
		{
			// Setup switches here
			knoblabel = "Button ";			
			actuator_config.type = CC_ACTUATOR_MOMENTARY;
			//actuator_config.type = CC_ACTUATOR_CONTINUOUS;
			actuator_config.max = 1.0;
			//actuator_config.supported_modes = CC_MODE_TOGGLE | CC_MODE_TRIGGER;
			actuator_config.value = &buttonValue;									
		}

		knoblabel += i+1;
		char charBuf[20];						
		knoblabel.toCharArray(charBuf, 20);
		actuator_config.name = charBuf;

		actuator_config.min = 0.0;
		actuator_config.max_assignments = 1;

		// create and add actuator to device
		cc_actuator_t *actuator;
		actuator = cc.newActuator(&actuator_config);
		cc.addActuator(device, actuator);
	}

	lcd.init();			//initialize the lcd
	lcd.backlight();	//open the backlight

	startupmessage();

	//static_cast<cc_assignment_t*>(updateNames));
				//static_cast<FilterAuthenticate*>(eventData) 

				//startupmessage();

				//set event callbacks
				// the currently possible event callbacks are:
				// CC_EV_ASSIGNMENT, CC_EV_UNASSIGNMENT and CC_EV_UPDATE
	cc.setEventCallback(CC_EV_UPDATE,		updateValues);
	cc.setEventCallback(CC_EV_ASSIGNMENT,	updateNames);
	cc.setEventCallback(CC_EV_UNASSIGNMENT, clearlcd);
}

void startupmessage() {
	lcd.setCursor(0, 0);
	lcd.print("ISRMOD");
	lcd.setCursor(0, 2);
	lcd.print("Version 0.1");
	delay(2000);
	lcd.clear();
}

String val2;
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

void loop() {
	//lcd.clear();
	// read button state
	// state =  1 -> button pressed
	// state = -1 -> button released
	// state =  0 -> same state as before (no change)
	int state = readButton();
	if (state == 1) {
		buttonValue = 1.0;
	}
	else if (state == -1) {
		buttonValue = 0.0;
	}

	displayInfo();
	readpots();
	cc.run();
	delay(100);
}

int readButton(void) {
	// read the state of the switch into a local variable:
	int reading = digitalRead(buttonPin);

	// check to see if you just pressed the button
	// (i.e. the input went from LOW to HIGH),  and you've waited
	// long enough since the last press to ignore any noise:

	// If the switch changed, due to noise or pressing:
	if (reading != lastButtonState) {
		// reset the debouncing timer
		lastDebounceTime = millis();
	}

	if ((millis() - lastDebounceTime) > debounceDelay) {
		// whatever the reading is at, it's been there for longer
		// than the debounce delay, so take it as the actual current state:

		// if the button state has changed:
		if (reading != buttonState) {
			buttonState = reading;

			// save button last state
			lastButtonState = reading;

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
	lastButtonState = reading;
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
	actuatorValues[assignment->actuator_id] = assignment->value;
	//	  writeValues(assignment->actuator_id);

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

//when assigned to a actuator, gets the needed information and calls the write names function
void updateNames(cc_assignment_t *assignment) {
	minValues[assignment->actuator_id] = assignment->min;
	maxValues[assignment->actuator_id] = assignment->max;

	int len = assignment-> label.size;
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

	// configure button pin as input and enable internal pullup
	pinMode(buttonPin, INPUT);
	digitalWrite(buttonPin, HIGH);
	pinMode(9, INPUT);
	digitalWrite(9, HIGH);
	pinMode(10, INPUT);
	digitalWrite(10, HIGH);
	pinMode(11, INPUT);
	digitalWrite(10, HIGH);
	pinMode(12, INPUT);
	digitalWrite(12, HIGH);
}
