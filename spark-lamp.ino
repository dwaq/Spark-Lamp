// This #include statement was automatically added by the Spark IDE.
#include "clickButton/clickButton.h"

// the Button
//const int buttonPin1 = 4;
ClickButton dillonButton(dillonLamp, LOW, CLICKBTN_PULLUP);
ClickButton saraButton(saraLamp, LOW, CLICKBTN_PULLUP);

// Button results 
int dillonClicks = 0;
int saraClicks = 0;

// This #include statement was automatically added by the Spark IDE.
#include "RCSwitch/RCSwitch.h"

// switch object
RCSwitch mySwitch = RCSwitch();

// "addresses" for the relays
#define DILLON_ON   4527411  // 0x451533
#define DILLON_OFF  4527420  // 0x45153C

#define SARA_ON     4527555  // 0x4515C3
#define SARA_OFF    4527564  // 0x4515CC

// length of each bit
#define PULSE_LENGTH 190     // 0xBE

// length of packet to send
#define BIT_LENGTH 24

// pin definitions
#define TRANSMITTER D0  // 433 Mhz transmitter
#define dillonLamp A5   // Dillon's lamp switch
#define saraLamp A6     // Sara's lamp switch
#define lightSwitch A4  // room light switch

// the lamp's state is stored in a two bit binary number
#define DILLON_BIT 0b01 // bit 0 is the state of Dillon's lamp
#define SARA_BIT 0b10   // bit 1 is the state of Sara's lamp
int lampState = 0b00;   // stores the state of both lamps

void setup()
{
    Serial.begin(9600);
    //pinMode(D4, INPUT_PULLUP);

    // set up the 3 switch pins as inputs
    pinMode(dillonLamp, INPUT_PULLUP);
    pinMode(saraLamp, INPUT_PULLUP);
    pinMode(lightSwitch, INPUT_PULLDOWN);

    // Setup button timers (all in milliseconds / ms)
    // (These are default if not set, but changeable for convenience)
    dillonButton.debounceTime   = 20;   // Debounce timer in ms
    dillonButton.multiclickTime = 250;  // Time limit for multi clicks
    dillonButton.longClickTime  = 1000; // time until "held-down clicks" register

    // set Sara's timers the same as Dillon's
    saraButton.debounceTime     = dillonButton.debounceTime;
    saraButton.multiclickTime   = dillonButton.multiclickTime;
    saraButton.longClickTime    = dillonButton.longClickTime;

    // setup the transmitter on pin D0
    mySwitch.enableTransmit(TRANSMITTER);
    // Set pulse length of a bit
    mySwitch.setPulseLength(PULSE_LENGTH);

    // create function(s) so app can change states
    Spark.function("switch", webSwitch);

    // make variable available to GET
    Spark.variable("state", &lampState, INT);
}


void loop()
{
  // Update button state
  dillonButton.Update();

  // Save click codes in dillonClicks, as click codes are reset at next Update()
  // is this line even needed?
  // can't I just directly use dillonButton.clicks ?
  // maybe I can wrap the 3 dillonClicks calls into this function?
  if (dillonButton.clicks != 0) dillonClicks = dillonButton.clicks;
  
  if(dillonClicks == 1){
      Serial.println("SINGLE click");
      toggleDillon();
  }

  if(dillonClicks == 2){
    Serial.println("DOUBLE click");
    toggleSara();
  } 

if(dillonClicks == -1){
      Serial.println("SINGLE LONG click");
      matchToggle("DILLON");
  }
  
  dillonClicks = 0;
  delay(5);
}


/* Exposed function to turn lamps on, off, or toggle individually */
int webSwitch(String state){
    if(state == "ON"){
        // turn both lamps on
        switchLamps(state);
    }
    else if(state == "DILLON"){
        // toggle Dillon's lamp
        toggleDillon();
    }
    else if(state == "SARA"){
        // toggle Sara's lamp
        toggleSara();
    }
    else if(state == "OFF"){
        // turn both lamps off
        switchLamps(state);
    }
}

/* toggle Dillon's lamp */
void toggleDillon(){
    // toggle the state of Dillon's lamp
    lampState ^= DILLON_BIT;

    // send new state
    if ((lampState & DILLON_BIT) == DILLON_BIT){
        //turn on Dillon's lamp
        mySwitch.send(DILLON_ON, BIT_LENGTH);
    }
    else{
        // turn off Dillon's lamp
        mySwitch.send(DILLON_OFF, BIT_LENGTH);
    }
}

/* toggle Sara's lamp */
void toggleSara(){
    // toggle the state of Sara's lamp
    lampState ^= SARA_BIT;

    // send new state
    if ((lampState & SARA_BIT) == SARA_BIT){
        // turn on Sara's lamp
        mySwitch.send(SARA_ON, BIT_LENGTH);
    }
    else{
        // turn off Sara's lamp
        mySwitch.send(SARA_OFF, BIT_LENGTH);
    }
}

/* set both lamps to a new state */
void switchLamps(String state){
    if(state == "ON"){
        // turn both lamps on
        mySwitch.send(DILLON_ON, BIT_LENGTH);
        mySwitch.send(SARA_ON, BIT_LENGTH);
        lampState = 0b11;
    }
    else if(state == "OFF"){
        // turn both lamps off
        mySwitch.send(DILLON_OFF, BIT_LENGTH);
        mySwitch.send(SARA_OFF, BIT_LENGTH);
        lampState = 0b00;
    }
}

/* set both lamps to the opposite of the button's lamp's current state */
void matchToggle(String button){
    int buttonBit;

    // decide which button to check
    if (button == "DILLON"){
        buttonBit = DILLON_BIT;
    }
    else if (button == "SARA"){
        buttonBit = SARA_BIT;
    }

    // currently on, so turn both off
    if ((lampState & buttonBit) == buttonBit){
        switchLamps("OFF");
    }
    // currently off, so turn both on
    else {
        switchLamps("ON");
    }
}
