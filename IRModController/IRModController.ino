// Ian R Dec 17
// V0.1 - Basic control from a single knob
// V0.2 - Display value of a single label (after enabling strings in config.h)

#include <Wire.h> 
#include <ControlChain.h>
#include <LiquidCrystal_I2C.h>

//LCDs used
#define lines 4
#define characters 20

// the following variables are unsigned long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay    = 20; // the debounce time; increase if the output flickers

float buttonValue;
int   buttonPin = 2;
int displayloop = 0;

int ledPin = 13;
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;  // the previous reading from the input pin

//amount of actuators connected 
#define amountOfPorts 10
#define amountOfPotentiometers 2
float actuatorValues[amountOfPorts], maxValues[amountOfPorts], minValues[amountOfPorts];
//potValues[amountOfPorts], 
//2D array for saving the actuator labels
char  actuatorNames[amountOfPorts][20];
//String testactname = "name unassigned";

// Rotary Encoders
#define Encoder1Port 6
#define Encoder1APin  26
#define Encoder1BPin  28
int Encoder1AState, Encoder1ALastState, Encoder1Ct = 0;

#define Encoder2Port 7
#define Encoder2APin 22
#define Encoder2BPin 24
int Encoder2Ct = 0, Encoder2AState, Encoder2ALastState;

ControlChain cc;

// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x3F, characters, lines); // set the LCD address to 0x3F for a 20 chars and 4 line display

void setup() {
	// configure led
	/*pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, LOW); */

	// configure button pin as input and enable internal pullup


	Encoder1ALastState = digitalRead(Encoder1APin);
	Encoder2ALastState = digitalRead(Encoder2APin);

	// initialize control chain
	cc.begin();

	// define device name (1st parameter) and its URI (2nd parameter).  The URI must be an unique identifier for your device. A good practice is to use a URL pointing to your project's code or documentation
	const char *uri = "https://www.rogersons.net/ModControl";
	cc_device_t *device = cc.newDevice("Ext", uri);

	for (int i = 0; i < amountOfPorts; i++) {
		cc_actuator_config_t actuator_config;
		//		actuator_config.name = "Chatn #" + i;

		switch (i) {
		case 0: actuator_config.name = "Button 1"; break; //actuator_config.type = CC_ACTUATOR_MOMENTARY; actuator_config.max = 1; break;
		case 1: actuator_config.name = "Button 2"; break; //actuator_config.type = CC_ACTUATOR_MOMENTARY; break;
		case 2: actuator_config.name = "Button 3"; break; //actuator_config.type = CC_ACTUATOR_MOMENTARY; break;
		case 3: actuator_config.name = "Button 4"; break; //actuator_config.type = CC_ACTUATOR_MOMENTARY; break;	
		case 4: actuator_config.name = "Knobswitch 1";  break;
		case 5:	actuator_config.name = "Knobswitch 2";  break;
		case 6:	actuator_config.name = "Knob 1";   break;
		case 7: actuator_config.name = "Knob 2";   break;
		case 8: actuator_config.name = "Expression 1";  break;
		case 9:	actuator_config.name = "Expression 2";  break;
		}

		actuator_config.value = &actuatorValues[i];
	//	actuator_config.min = 0.0;
		if (i < 6)
		{
		// Setup switches here					
			actuator_config.type = CC_ACTUATOR_MOMENTARY;
			//actuator_config.name = "Btn " + i - 4;
			////////actuator_config.value = 1.0;
			actuator_config.min = 0.0;
			actuator_config.max = 1.0;
			actuator_config.supported_modes = CC_MODE_TOGGLE | CC_MODE_TRIGGER;
		}
		else
		{			
			// Setup continuous controllers here
			actuator_config.type = CC_ACTUATOR_CONTINUOUS;
			if (i<8) 
				actuator_config.max = 255.0;
			else
				actuator_config.max = 1023.0;

			actuator_config.supported_modes = CC_MODE_REAL | CC_MODE_INTEGER;		
		}
	//	
	//	
		actuator_config.max_assignments = 1;
		// create and add actuator to device
		cc_actuator_t *actuator;
		actuator = cc.newActuator(&actuator_config);
		cc.addActuator(device, actuator);
	}

	
	lcd.init(); //initialize the lcd
	lcd.backlight(); // turn on the backlight

					 // static_cast<cc_assignment_t*>(updateNames));
					 //static_cast<FilterAuthenticate*>(eventData) 

	StartupMessage();
	//set event callbacks
	// the currently possible event callbacks are:
	// CC_EV_ASSIGNMENT, CC_EV_UNASSIGNMENT and CC_EV_UPDATE

	cc.setEventCallback(CC_EV_UPDATE, updateValues);
	cc.setEventCallback(CC_EV_ASSIGNMENT, updateNames);
/*	cc.setEventCallback(CC_EV_UNASSIGNMENT, clearlcd);*/
}



String val2;
void displayInfo()
{
	lcd.setCursor(0, 0); // set the cursor to column 15, line 1
	//lcd.print("MOD DUO Controller");

	//String mval = "Modulator1: " + String(modval1);
	//u8g.drawStr(0, 0, "MOD interface-");
	////u8g.print("MOD interface-");
	////u8g.drawStr( 0, 16, mval );
	//u8g.setPrintPos(0, 16);
	//lcd.setCursor(0, 0);
	//val2 = "A1: " + (String)analogRead(0) + "     ";

	//lcd.clear();
	lcd.print(val2);
	lcd.setCursor(0, 0);
	lcd.print("k1:" + (String)actuatorNames[0] + " " + (String)actuatorValues[0]);
	lcd.setCursor(0, 1);
	lcd.print("k2:" + (String)actuatorNames[1] + " " + (String)actuatorValues[1]);
	lcd.setCursor(0, 2);
	lcd.print("p1:" + (String)actuatorNames[2] + " " + (String)actuatorValues[2]);
	lcd.setCursor(0, 3);
	lcd.print("p2:" + (String)actuatorNames[3] + " " + (String)actuatorValues[3]);

	//String val3 = "A2: " + (String)analogRead(1);
	//u8g.print("A2 label:" + (String)actuatorNames[1]);
}

void loop() {
	
	pinMode(buttonPin, INPUT);
	digitalWrite(buttonPin, HIGH);
	if (displayloop==500)
	{ 
		//lcd.setCursor(6, 0);
		//lcd.print("enc 1:" + (String)Encoder1Ct + "   ");
		//lcd.setCursor(6, 2);
		//lcd.print("enc 2:" + (String)Encoder2Ct + "    ");
		displayInfo();
		displayloop = 0;
	}
	displayloop++;

	//lcd.clear();
	//displayInfo();
	//actuatorValues[0] = readButton();

	ReadEncoders();
	//ReadPots();
	cc.run();
	//delay(100);

}

void ReadEncoders() {
	Encoder1AState = digitalRead(Encoder1APin); // Reads the "current" state of the outputA
	if (Encoder1AState != Encoder1ALastState) {
		// If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
		if (digitalRead(Encoder1APin) != (digitalRead(Encoder1BPin))) Encoder1Ct++;
		else Encoder1Ct--;
	}
	if (Encoder1Ct > 255) Encoder1Ct = 255;
	if (Encoder1Ct < 0)  Encoder1Ct = 0;
	Encoder1ALastState = Encoder1AState;
	actuatorValues[Encoder1Port] = (float)Encoder1Ct;

	Encoder2AState = digitalRead(Encoder2APin); // Reads the "current" state of the outputA
	if (Encoder2AState != Encoder2ALastState) {
		if (digitalRead(Encoder2APin) != digitalRead(Encoder2BPin)) Encoder2Ct++;
		else Encoder2Ct--;
	}
	if (Encoder2Ct > 255) Encoder2Ct = 255;
	if (Encoder2Ct < 0)  Encoder2Ct = 0;
	Encoder2ALastState = Encoder2AState;
	actuatorValues[Encoder2Port] = (float)Encoder2Ct;
}

void updateLED(cc_assignment_t *assignment) {
	// check if assignment mode is toggle
	// turn led on/off according the assignment value
	if (assignment->mode & CC_MODE_TOGGLE) {
		if (assignment->value > 0.0)
			digitalWrite(ledPin, HIGH);
		else
			digitalWrite(ledPin, LOW);
	}
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
void  ReadPots() {
	for (int i; i<amountOfPotentiometers; i++) {
	//	potValues[i] = analogRead(i);
	}
}

//updates actuator value and calls the write function
void updateValues(void *ass) {
	cc_assignment_t* assignment = (cc_assignment_t*)ass;
	actuatorValues[assignment->actuator_id] = assignment->value;
	//	  writeValues(assignment->actuator_id);
}

void writeValues(cc_assignment_t *assignment)
{

}

void SetupPins()
{
	// Encoders
	pinMode(Encoder1APin, INPUT);
	pinMode(Encoder1BPin, INPUT);

	// Inputs
	pinMode(2, INPUT);
	pinMode(3, INPUT);
	pinMode(4, INPUT);
	pinMode(5, INPUT);
	pinMode(6, INPUT);

	// LEDs
	pinMode(8, OUTPUT);
	pinMode(9, OUTPUT);
	pinMode(10, OUTPUT);
	pinMode(11, OUTPUT);

}

void StartupMessage() {
	lcd.setCursor(0, 0);
	lcd.print("ISRMOD v1.0");
	lcd.setCursor(0, 2);
	lcd.print("Starting up...");

	for (int i = 8; i < 12; i++)
	{
		digitalWrite(i, HIGH);
		delay(500);
		digitalWrite(i, LOW);
	}
	/*digitalWrite(8, HIGH);
	digitalWrite(9, HIGH);
	digitalWrite(10, HIGH);
	digitalWrite(11, HIGH);*/

	delay(500);
	lcd.clear();

}


void clearlcd(void *ass)
{
	cc_assignment_t* assignment = (cc_assignment_t*)ass;
	//lcd.clear();
	//for (int i=0;i < assignment->label.size;i++)
	//{ 
	//	//actuatorNames[assignment->actuator_id][i] = (char)"";
	//}
}

//when assigned to a actuator, gets the needed information and calls the write names function
//void updateNames(cc_assignment_t *assignment) {
void updateNames(void *ass) {
	cc_assignment_t*  assignment = (cc_assignment_t*)ass;
	minValues[assignment->actuator_id] = assignment->min;
	maxValues[assignment->actuator_id] = assignment->max;
	int len = assignment->label.size;
	if (len >= characters) len = characters - 1;

	//testactname = (String)assignment->label.text;

	//for (int i = 0; i < assignment->label.size; i++) {
	for (int i = 0; i < len; i++) {
		actuatorNames[assignment->actuator_id][i] = assignment->label.text[i];
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
