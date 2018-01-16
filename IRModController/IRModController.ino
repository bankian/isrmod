// Ian R Dec 17
// V0.1 - Basic control from a single knob
// V0.2 - Display value of a single label (after enabling strings in config.h)

//#include "U8glib.h"
#include <ControlChain.h>
#include <LiquidCrystal_I2C.h>

//LCD's used
#define lines 4
#define characters 20

//amount of potentiometers connected 
#define amountOfPorts 8
float potValues[amountOfPorts], actuatorValues[amountOfPorts], maxValues[amountOfPorts], minValues[amountOfPorts];
//2D array for saving the actuator labels
char actuatorNames[amountOfPorts][20];
String testactname = "name unassigned";

ControlChain cc;
float potValue;
float portValues[amountOfPorts];
int ledPin = 13, potPin = A0;
int modval1 = 0;

/**********************************************************/
char array1[] = " MOD DUO               "; //the string to print on the LCD
char array2[] = "hello, Ian!             "; //the string to print on the LCD
int tim = 500; //the value of delay time
// initialize the library with the numbers of the interface pins
LiquidCrystal_I2C lcd(0x3F, 20, 4); // set the LCD address to 0x3F for a 20 chars and 4 line display

//U8GLIB_SH1106_128X64 u8g(4, 5, 6, 7);  // SW SPI Com: SCK = 4, MOSI = 5, CS = 6, A0 = 7 (new blue HalTec OLED)

// Set up the big display
//void u8g_prepare(void) {
//	u8g.setFont(u8g_font_6x10);
//	u8g.setFontRefHeightExtendedText();
//	u8g.setDefaultForegroundColor();
//	u8g.setFontPosTop();
//}

void setup() {
	// configure led
	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, LOW);

	lcd.init(); //initialize the lcd
	lcd.backlight(); //open the backlight 

	// initialize control chain
	cc.begin();

	// define device name (1st parameter) and its URI (2nd parameter).  The URI must be an unique identifier for your device. A good practice is to use a URL pointing to your project's code or documentation
	const char *uri = "https://www.rogersons.net/ModControl";
	cc_device_t *device = cc.newDevice("ISR_MOD_Controller", uri);

	for (int i = 0; i < amountOfPorts; i++) {
		cc_actuator_config_t actuator_config;
		actuator_config.type = CC_ACTUATOR_CONTINUOUS;
//		actuator_config.name = "Chatn #" + i;
        
		switch (i) {
		case 0:	actuator_config.name = "Knob 1";   break;
		case 1: actuator_config.name = "Knob 2";   break;
		case 2: actuator_config.name = "Pedal 1";  break;
		case 3:	actuator_config.name = "Pedal 2";  break;
		case 4: actuator_config.name = "Button 1"; break;
		case 5: actuator_config.name = "Button 2"; break;
		case 6: actuator_config.name = "Button 3"; break;
		case 7: actuator_config.name = "Button 4"; break;
		}

		actuator_config.value = &portValues[i];
		actuator_config.min = 0.0;
		actuator_config.max = 1023.0;
		actuator_config.supported_modes = CC_MODE_REAL | CC_MODE_INTEGER;
		actuator_config.max_assignments = 1;

		// create and add actuator to device
		cc_actuator_t *actuator;
		actuator = cc.newActuator(&actuator_config);
		cc.addActuator(device, actuator);
	}

	//*****
	// configure actuator
	cc_actuator_config_t actuator_config;
	actuator_config.type = CC_ACTUATOR_CONTINUOUS;
	actuator_config.name = "Turn me!";
	actuator_config.value = &potValue;
	actuator_config.min = 0.0;
	actuator_config.max = 1023.0;
	actuator_config.supported_modes = CC_MODE_REAL | CC_MODE_INTEGER;
	actuator_config.max_assignments = 1;

	// create and add actuator to device
	cc_actuator_t *actuator;
	actuator = cc.newActuator(&actuator_config);
	cc.addActuator(device, actuator);

	//set event callbacks
	//    cc.setEventCallback(CC_EV_UPDATE, updateValues);
	cc.setEventCallback(CC_EV_ASSIGNMENT, updateNames);
	// static_cast<cc_assignment_t*>(updateNames));

	//static_cast<FilterAuthenticate*>(eventData)
	//    cc.setEventCallback(CC_EV_UNASSIGNMENT, clearlcd);  
}

void displayInfo()
{
	lcd.setCursor(1,1); // set the cursor to column 15, line 1
	lcd.print("Hello");
	//String mval = "Modulator1: " + String(modval1);
	//u8g.drawStr(0, 0, "MOD interface-");
	////u8g.print("MOD interface-");
	////u8g.drawStr( 0, 16, mval );
	//u8g.setPrintPos(0, 16);
	//String val2 = "A1: " + (String)analogRead(0);
	//u8g.print(val2);
	//String val3 = "A2: " + (String)analogRead(1);
	//
	//u8g.setPrintPos(0, 32);
	//u8g.print(testactname);
	//u8g.setPrintPos(0, 48);
	//u8g.print("A2 label:" + (String)actuatorNames[1]);
}

void loop() {
	// put your main code here, to run repeatedly:
	//u8g.firstPage();
	//u8g_prepare();
/*	do {
		displayInfo();
	} while (1 == 1);*/ // (u8g.nextPage());

	displayInfo();
	potValue = analogRead(potPin);

	for (int i = 0; i < amountOfPorts; i++) {
		portValues[i] = analogRead(i);
	}

	cc.run();
	delay(50);
}

//updates actuator value and calls the write function
void updateValues(cc_assignment_t *assignment) {
	  actuatorValues[assignment->actuator_id] = assignment->value;
//	  writeValues(assignment->actuator_id);
}

void writeValues(cc_assignment_t *assignment)
{

}

//when assigned to a actuator, gets the needed information and calls the write names function
void updateNames(cc_assignment_t *assignment) {
	minValues[assignment->actuator_id] = assignment->min;
	maxValues[assignment->actuator_id] = assignment->max;

	//Bu8g.setPrintPos(0, 48);
	//u8g.print("String testname = "name unassigned";");//(val3);
	testactname = (String)assignment->label.text;
	for (int i = 0; i < assignment->label.size; i++) {
		actuatorNames[assignment->actuator_id][i] = assignment->label.text[i];
	}
	//  writeNames(assignment->actuator_id, assignment->label.size, 0);
}

//write the name of the selected potentiometer
void writeNames(int num, int labelsize, int clr) {
	//  switch (num){
	//  case 0:
	//    lcd.setCursor(0,0);
	//    lcd.print ("#Pot 1:              ");
	//    if ( clr != 1){
	//      u8g.setPrintPos(0, 48); 
	//      u8g.print("dummy");//(val3);
	//      lcd.setCursor(10,0);
	//      for (int i = 0; i < labelsize; i++){
	//        lcd.print (actuatorNames[num][i]);
	//      }
	//    }
	//  break;
}
