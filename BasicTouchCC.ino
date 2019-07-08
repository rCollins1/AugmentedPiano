// A script written to use an MPR121 cap-touch board and a Teensy 3.2 board as a MIDI device
// In the Arduino Tools/USB-type menu select "MIDI" when programming the board
// This script will play different notes as you touch each cap-touch pin
// It also uses a pad as a capacitive potentiometer to send CC
// Make sure to program your Teensy 3.2 in MIDI mode

// You need to include two libraries:
// the I2C "Wire.h" library, 
// the Adafruit MPR121 Cap-touch Board "Adafruit_MPR121.h" library

#include <Wire.h>
#include "Adafruit_MPR121.h"

// Define Analog Input
#define pot1 A0

// Create the cap-touch board variables
// You can have up to 4 boards on one I2C bus (because there are 4 different board addresses)
Adafruit_MPR121 capA = Adafruit_MPR121();

// use unsigned integers in binary format to represent the state of touched pins

// current and previous state of the system
uint16_t currstateA = 0;
uint16_t prevstateA = 0;

// Create variables for the pressure sesnsing pad
uint16_t currcap = 0;
uint16_t prevcap = 0;

elapsedMillis timer = 0;

/*
touchedA is a 16 bit unsigned integers, with the state of the 12 pins of 
the cap-touch board represented as the binary bits of that integer.
That is, either as a 0 for "not-touched" or a 1 for "touched". 

For example:
  If pin 0 was touched, and pin 1 to pin 11 were not touched, then the variable 
  touchedA would be:
  0000 0000 0000 0001 which is equal to the decimal value 2^0 = 1

  If pin 0, pin 3, pin 7 were touched, and all other pins were not touched, then the variable 
  touchedA would be:
  0000 0000 1000 1001 which is equal to the decimal value 2^0 + 2^3 + 2^7 = 1 + 8 + 128 = 137

  Later in the code we'll look at the individual bits to determine the states of each pin
  on the cap-touch board.
*/

// the MIDI channel to send messages on
int channel = 1;

// Notes played for each cap touch board
// these are identifed using the standard MIDI note numbers
// Middle C is note 60

// We only have 7 touch pads...we also don't use pin 0, so start at pin 1
// These correspond to pins:
              // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11
//int notesA[] = { 0,60,62,63,65,67,69,70,72, 0, 0, 0};
int notesA[] = {60,61,62,63,64,65,66,67,68,69,70,71};

// For octave selection
unsigned int octave = 0;
// Preset array with MIDI values corresponding to each octave
int octaves[8][12] = {{24,25,26,27,28,29,30,31,32,33,34,35},
                      {36,37,38,39,40,41,42,43,44,45,46,47},
                      {48,49,50,51,52,53,54,55,56,57,58,59},
                      {60,61,62,63,64,65,66,67,68,69,70,71},
                      {72,73,74,75,76,77,78,79,80,81,82,83},
                      {84,85,86,87,88,89,90,91,92,93,94,95},
                      {96,97,98,99,100,101,102,103,104,105,106,107},
                      {108,109,110,111,112,113,114,115,116,117,118,119}};

// Some common scale intervals
//dorian       = "2,1,2,2,2,1,2"
//phrygian     = "1,2,2,2,1,2,2"
//lydian       = "2,2,2,1,2,2,1"
//mixolydian   = "2,2,1,2,2,1,2"
//aeolian      = "2,1,2,2,1,2,2"
//locrian      = "1,2,2,1,2,2,2"

// The velocity you want to play the note with (value from 0-127)
int vel = 120;

// Set up integers to store the state of the potentiometer values
int currPotValue = 0;
int prevPotValue = 0;
int CCpot = 90; // Send CC values from the potentiometer on CC channel 90
int CCcap = 91; // Send CC values from the capacitor on CC channel 91

// Setup loop that intitiates the I2C hardware and the hardware MIDI
void setup() {
  // start the "A" cap-touch board - 0x5A is the I2C address
  capA.begin(0x5A);
  if (!capA.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
}

// the main loop
void loop() {

  // Get the currently touched pads
  currstateA = capA.touched();

  // Check Octave 
  checkOctave();
  // Check the state of the cap-touch board and turn on/off notes as needed
  checkCap();

  // Only check the control changes 20 times a second so we dont overload
  // the MIDI-bus with too many messages
  if(timer > 50){    
    // Check Pot
    //checkCCpot();

    // Check Cap
    checkCCcap();
    timer = 0;
  }
  
  // Update our state
  prevstateA = currstateA;
}

// Function to check the cap-touch board and send turn on/off notes as needed
void checkCap(){
  // Loop through the 12 touch points on the board
  // We use pin 0 for cap touch CC later, so skip that one and start at n = 1
  for(int n = 1; n <12; n++){
    // Compare the current state to the previous state
    // If it has changed you need to do something
    if(bitRead(currstateA,n) != bitRead(prevstateA,n)){
      
      // If it has changed and is TRUE, then turn on that note
      if(bitRead(currstateA,n)){
        // Send out a MIDI message on both usbMIDI and hardware MIDI ports
//        usbMIDI.sendNoteOn(notesA[n],vel,2); 
      // I have set it (below) so that pin 0 controls the volume of all the other keys
        usbMIDI.sendNoteOn(notesA[n],currcap,2);
//        Serial.println(notesA[n]);
      // otherwise it must have changed and is FALSE, so turn off that note
      } else {
        // Send out a MIDI message on both usbMIDI and hardware MIDI ports
        usbMIDI.sendNoteOff(notesA[n],0,2); 
//        Serial.println(notesA[n]);
//        Serial.print("new stuff ");
//        Serial.println(pot1);
      }
    }
  }
}

void checkCCpot() {
  currPotValue = analogRead(pot1)/8; // Read the pin and divide by 8 to get a value from (0-127)
  // Check if the Potentiometer value has changed, and send a new CC message if it has
  if(currPotValue != prevPotValue){
    usbMIDI.sendControlChange(CCpot,currPotValue,channel);
  }
  prevPotValue = currPotValue; // Update the prevPotValue for the next loop
}

void checkCCcap(){
  currcap = constrain(map(capA.filteredData(0), 40, 130, 120, 30), 30, 120);
//  Serial.println(capA.filteredData(0));
  if(currcap != prevcap){
    usbMIDI.sendControlChange(CCcap,currcap,channel);
  }
  prevcap = currcap;
}

void checkOctave() {
  currPotValue = analogRead(pot1);
  //float voltage = currPotValue * (5.0 / 1023.0);
  
  if(currPotValue != prevPotValue){
    octave = (currPotValue/*0-1023*/ / 128);
    // Print the first value of the selected octave array
    Serial.println(octaves[octave][0]);
    memcpy(notesA, octaves[octave], sizeof(notesA));
  }
}

/*********************************************************
This is a library for the MPR121 12-channel Capacitive touch sensor

Designed specifically to work with the MPR121 Breakout in the Adafruit shop 
  ----> https://www.adafruit.com/products/

These sensors use I2C communicate, at least 2 pins are required 
to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.  
BSD license, all text above must be included in any redistribution
**********************************************************/
