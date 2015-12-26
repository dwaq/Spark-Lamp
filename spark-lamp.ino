// This #include statement was automatically added by the Particle IDE.
#include "clickButton/clickButton.h"
#include "RCSwitch/RCSwitch.h"

// set up the RC Switch object
RCSwitch mySwitch = RCSwitch();

// "addresses" for the relays
#define DILLON_ON   4527411  // 0x451533
#define DILLON_OFF  4527420  // 0x45153C

#define SARA_ON     4527555  // 0x4515C3
#define SARA_OFF    4527564  // 0x4515CC

#define LIQUOR_ON   0x451703
#define LIQUOR_OFF  0x45170C

// length of each bit
#define PULSE_LENGTH 190     // 0xBE

// length of packet to send
#define BIT_LENGTH 24

// pin definitions
#define TRANSMITTER D0  // 433 Mhz transmitter
#define dillonLamp D4   // Dillon's lamp switch
#define saraLamp D2     // Sara's lamp switch
#define lightSwitch A0  // room light switch

// stores the state of the lightSwitch
/*
  Defaults to 1 so, if the switch is on when it powers up,
  the lights will not turn on.
  This will be useful if there is a power outage in the middle
  of the night, so regardless of the switch position, the lights
  won't automatically turn on.
*/
int lightSwitchState = 1;

// stores the state of the liquorSwitch
int liquorSwitchState =0;

// the lamp's state is stored in a two bit binary number
#define DILLON_BIT 0b01 // bit 0 is the state of Dillon's lamp
#define SARA_BIT 0b10   // bit 1 is the state of Sara's lamp
int lampState = 0b00;   // stores the state of both lamps

// set up the Click Button object
//const int buttonPin1 = 4;
ClickButton dillonButton(dillonLamp, LOW, CLICKBTN_PULLUP);
ClickButton saraButton(saraLamp, LOW, CLICKBTN_PULLUP);

/* There's an issue with the relay interrupt being called right after
   a lamp switch is pressed (but only if the light switch is "off"),
   so we use this counter to block the interrupt */
int blockInterrupt = 0;
#define blockInterruptTimes 5

/* Set up pins and Particle web functions */
void setup()
{
    //Serial.begin(9600);

    // set up the 3 switch pins as inputs
    pinMode(dillonLamp, INPUT_PULLUP);
    pinMode(saraLamp, INPUT_PULLUP);
    pinMode(lightSwitch, INPUT_PULLDOWN);

    // if the lightSwtich pin changes value, go to its interrupt
    attachInterrupt(lightSwitch, lightSwitchLamps, CHANGE);

    // setup the transmitter on pin D0
    mySwitch.enableTransmit(TRANSMITTER);
    // Set pulse length of a bit
    mySwitch.setPulseLength(PULSE_LENGTH);

    // create function(s) so app can change states
    Particle.function("switch", webSwitch);

    // create new function for liquor lights
    Particle.function("liquor", webLiquorSwitch);

    // make variable available to GET
    Particle.variable("state", &lampState, INT);

    // take control of the RGB status LED
    RGB.control(true);

    // set the RGB LED brightness to 0% or off
    RGB.brightness(0);
}

/* Continously check the button states */
void loop()
{
    // Update buttons state
    dillonButton.Update();
    saraButton.Update();

    // count down as it goes through the loop
    if (blockInterrupt)
    {
        //Serial.println(blockInterrupt);
        blockInterrupt--;
    }

    // Dillon's button was clicked
    if (dillonButton.clicks != 0){

        if(dillonButton.clicks == 1){
            //Serial.println("SINGLE click");
            blockInterrupt = blockInterruptTimes;
            toggleDillon();
        }

        if(dillonButton.clicks == 2){
            //Serial.println("DOUBLE click");
            blockInterrupt = blockInterruptTimes;
            toggleSara();
        }

        if(dillonButton.clicks == -1){
            //Serial.println("SINGLE LONG click");
            blockInterrupt = blockInterruptTimes;
            matchToggle("DILLON");
        }
    }

    // Sara's button was clicked
    if (saraButton.clicks != 0){

        if(saraButton.clicks == 1){
            //Serial.println("SINGLE click");
            blockInterrupt = blockInterruptTimes;
            toggleSara();
        }

        if(saraButton.clicks == 2){
            //Serial.println("DOUBLE click");
            blockInterrupt = blockInterruptTimes;
            toggleDillon();
        }

        if(saraButton.clicks == -1){
            //Serial.println("SINGLE LONG click");
            blockInterrupt = blockInterruptTimes;
            matchToggle("SARA");
        }
    }

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

/* Exposed function to turn Liquor lights on or off */
int webLiquorSwitch(String state){
  if(state == "TOGGLE"){
    // toggle liquor lights by setting the state to the opposite
    // of the current state
    if (liquorSwitchState == 0){
      state = "ON";
    }
    else if(liquorSwitchState == 1){
      state = "OFF";
    }
  }

  if(state == "OFF"){
    // turn liquor lights off
    mySwitch.send(LIQUOR_OFF, BIT_LENGTH);
    liquorSwitchState = 0;
  }
  else if(state == "ON"){
    // turn liquor lights on
    mySwitch.send(LIQUOR_ON, BIT_LENGTH);
    liquorSwitchState = 1;
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
        mySwitch.send(SARA_ON, BIT_LENGTH);
        mySwitch.send(DILLON_ON, BIT_LENGTH);
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

/* set both lamps to the state of the lightswitch */
void lightSwitchLamps(){
    // only trust this interrupt if the state hasn't changed recently
    /* Note: Due to the relay bouncing, this interrupt is called multiple
       times in a row for one switching motion. We don't want to debounce
       them in software because it's useful to quickly switch the lights
       from one state to another to get them in sync */
    if (blockInterrupt == 0)
    {
        if (digitalRead(lightSwitch) == LOW){
            //Serial.println("lightSwitch off");
            //blockInterrupt = blockInterruptTimes;
            // turn both lamps off
            mySwitch.send(DILLON_OFF, BIT_LENGTH);
            mySwitch.send(SARA_OFF, BIT_LENGTH);
            lampState = 0b00;
        }
        else{
            //Serial.println("lightSwitch on");
            //blockInterrupt = blockInterruptTimes;
            // turn both lamps on
            mySwitch.send(SARA_ON, BIT_LENGTH);
            mySwitch.send(DILLON_ON, BIT_LENGTH);
            lampState = 0b11;
        }
    }
}
