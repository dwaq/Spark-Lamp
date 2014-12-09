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

void setup() {
    // set up the 3 switch pins as inputs and pulled down
    pinMode(dillonLamp, INPUT_PULLDOWN);
    pinMode(saraLamp, INPUT_PULLDOWN);
    pinMode(lightSwitch, INPUT_PULLDOWN);
    
    // if the pin changes value, go to its corresponding interrupt
    attachInterrupt(dillonLamp,   toggleDillon, RISING);
    attachInterrupt(saraLamp,     toggleSara,   RISING);
    attachInterrupt(lightSwitch,  switchLamps,  CHANGE);

    // setup the transmitter on pin D0
    mySwitch.enableTransmit(TRANSMITTER);
    // Set pulse length of a bit
    mySwitch.setPulseLength(PULSE_LENGTH);

    // create function(s) so app can change states
    Spark.function("switch", webSwitch);
    
    // make variable available to GET
    Spark.variable("state", &lampState, INT);
}

void loop() {
    
}

// Exposed function to turn lamps on, off, or toggle individually
int webSwitch(String state){
    if(state == "ON"){
        // turn both lamps on
        mySwitch.send(DILLON_ON, BIT_LENGTH);
        mySwitch.send(SARA_ON, BIT_LENGTH);
        lampState = 0b11;
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
        mySwitch.send(DILLON_OFF, BIT_LENGTH);
        mySwitch.send(SARA_OFF, BIT_LENGTH);
        lampState = 0b00;
    }
}

void toggleDillon(){
    // toggle the state of Dillon's lamp
    lampState ^= DILLON_BIT;
    // new state for Dillon's lamp
    int newState = lampState & DILLON_BIT;
    // send new state
    if (newState == 0){
        // turn off Dillon's lamp
        mySwitch.send(DILLON_OFF, BIT_LENGTH);
    }
    else{
        // turn on Dillon's lamp
        mySwitch.send(DILLON_ON, BIT_LENGTH);
    }
}

void toggleSara(){
    // toggle the state of Sara's lamp
    lampState ^= SARA_BIT;
    // new state for Sara's lamp
    int newState = lampState & SARA_BIT;
    // send new state
    if (newState == 0){
        // turn off Sara's lamp
        mySwitch.send(SARA_OFF, BIT_LENGTH);
    }
    else{
        // turn on Sara's lamp
        mySwitch.send(SARA_ON, BIT_LENGTH);
    }
}

void switchLamps(){
    if (digitalRead(lightSwitch) == LOW){
        // turn both lamps off
        mySwitch.send(DILLON_OFF, BIT_LENGTH);
        mySwitch.send(SARA_OFF, BIT_LENGTH);
        lampState = 0b00;
    }
    else{
        // turn both lamps on
        mySwitch.send(DILLON_ON, BIT_LENGTH);
        mySwitch.send(SARA_ON, BIT_LENGTH);
        lampState = 0b11;
    }
}

