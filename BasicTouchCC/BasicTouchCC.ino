// Group Nine Inch Nails, Modified Piano for Stroke Patients 
// Team Members: Eason Gao, Tanya Ralliaram, Rachelyn Collins, Annie Zhang, Charmaine Wang, Neil Agarwal

// A script written to use for four MPR121 cap-touch boards and Teensy 3.5 board as a MIDI device
// The script below is modified from Professor Borland's BasicTouchCC.ino script

// Libraries
#include <Wire.h>
#include "Adafruit_MPR121.h"

// Define Analog Input for Octave Selector
#define pot1 A0

// Initialize four cap touch boards
// First Octave
Adafruit_MPR121 capA = Adafruit_MPR121();
// Chord Selection
Adafruit_MPR121 capB = Adafruit_MPR121();
// Third Octave
Adafruit_MPR121 capC = Adafruit_MPR121();
// Second Octave
Adafruit_MPR121 capD = Adafruit_MPR121();

// Current and previous state of the systems
uint16_t currstateA = 0;
uint16_t prevstateA = 0;
uint16_t currstateB = 0;
uint16_t prevstateB = 0;
uint16_t currstateC = 0;
uint16_t prevstateC = 0;
uint16_t currstateD = 0;
uint16_t prevstateD = 0;

// Create state variables for cap
uint16_t currcap = 0;
uint16_t prevcap = 0;

// Initialize timer variable to delay for adafruit board
elapsedMillis timer = 0;

// Chord selection variables for LEDs and buttons 
const int led1Pin = 10;
const int led2Pin = 9;
const int led3Pin = 8;
const int led4Pin = 7;

const int record1 = 2;
const int record2 = 3;
const int record3 = 4;
const int record4 = 5;

// MIDI channel to send messages on
int channel = 1;

// Define notes array for keys on the piano 
int notesA[] = {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71};
int notesC[] = {72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83};
int notesD[] = {84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95};

// Initialize and declare an array to store the selected notes
int chords[5][36] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
// Variable to store which note needs to be played 
unsigned int noteToPlay = 0;

// Initialize variables for octave selection
unsigned int octave = 0;
unsigned int prevOctave = 0;

// Preset array with MIDI values corresponding to each octave
int octaves[15][12] = {{24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35}, {36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47}, {48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59},
  {36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47}, {48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59}, {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71},
  {48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59}, {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71}, {72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83},
  {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71}, {72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83}, {84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95},
  {72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83}, {84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95}, {96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107}
};

// Set up integers to store the state of the potentiometer values
int currPotValue = 0;
int prevPotValue = 0;

// Velocity is 120
int vel = 120;

// Send CC values from the potentiometer on CC channel 90
int CCpot = 90; 
// Send CC values from the capacitor on CC channel 91
int CCcap = 91; 

// Runs once to define and initialize important variables
void setup() {
    
  // Define input and output pins for LEDs and button used for chord selection
  pinMode(record1, INPUT);
  pinMode(led1Pin, OUTPUT);
  pinMode(record2, INPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(record3, INPUT);
  pinMode(led3Pin, OUTPUT);
  pinMode(record4, INPUT);
  pinMode(led4Pin, OUTPUT);

  // Start the "A" cap-touch board - 0x5A is the I2C address (ground)
  capA.begin(0x5A);
  if (!capA.begin(0x5A)) {
    Serial.println("MPR121 A not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 A found!");

  // Start the "B" cap-touch board - 0x5B is the I2C address (power)
  capB.begin(0x5B);
  if (!capC.begin(0x5B)) {
    Serial.println("MPR121 B not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 B found!");

  // Start the "C" cap-touch board - 0x5C is the I2C address (SDA)
  capC.begin(0x5C);
  if (!capC.begin(0x5C)) {
    Serial.println("MPR121 0x5C (SDA) not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 0x5C (SDA) found!");

  // Start the "D" cap-touch board - 0x5D is the I2C address (SCL)
  capD.begin(0x5D);
  if (!capD.begin(0x5D)) {
    Serial.println("MPR121 0x5D (SCL) not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 0x5D (SCL) found!");
}

// ====================== MAIN LOOP ================================
void loop() {
  // Functions to check the whether the record button for chord selection has been selected
  checkButton1();
  checkButton2();
  checkButton3();
  checkButton4();

  // Identify and stroe the pads that have been triggered
  currstateA = capA.touched();
  currstateB = capB.touched();
  currstateC = capC.touched();
  currstateD = capD.touched();

  // Check the current state for the octave selector
  checkOctave();
  
  // Check the state of the cap-touch board and turn on/off notes as needed
  // Send -1 as a sentient flag to differentiate from recording functionality 
  checkCap(-1);

  // Check the control changes 20 times a second to prevent MIDI-bus overload
  if (timer > 20) {
    // Check Pot
    // checkCCpot();

    // Check Cap
    checkCCcap();
    timer = 0;
  }

  // Update our state
  prevstateA = currstateA;
  prevstateB = currstateB;
  prevstateC = currstateC;
  prevstateD = currstateD;
}

// Function to check what cap touch pins are being activated and send notes
void checkCap(int recording) {
  // Loop through the 12 touch points on each board
  for (int n = 0; n < 12; n++) {
    // ============= CHECKING CAP A ===================================
    if (bitRead(currstateA, n) != bitRead(prevstateA, n)) {
      // If state has changed turn on corresponding note
      if (bitRead(currstateA, n)) {
        // Send out a MIDI message on both usbMIDI and hardware MIDI ports
        usbMIDI.sendNoteOn(notesA[n], vel, 2);
        // Store key value to corresponding index in recorded chords array
        if (recording > -1 ) {
          chords[recording][n] = 1;
        }
        // otherwise it must have changed and is FALSE, so turn off that note
      } else {
        // Send out a MIDI message on both usbMIDI and hardware MIDI ports
        usbMIDI.sendNoteOff(notesA[n], 0, 2);
      }
    }

    // ============= CHECKING CAP B (chords) ===================================
    if (bitRead(currstateB, n) != bitRead(prevstateB, n)) {
      if (bitRead(currstateB, n)) {

        for (int note = 0; note < 36; note++) {
          if (chords[n + 1][note] == 1) {
            // First octave
            if (note >= 0 && note < 12) {
              noteToPlay = notesA[note];
            }
            // Displace by 12 notes for second octave
            else if (note >= 12 && note < 24) {
              noteToPlay = notesC[note - 12];
            }
            // Displace by 24 notes for second octave
            else {
              noteToPlay = notesD[note - 24];
            }
            usbMIDI.sendNoteOn(noteToPlay, vel, 2);
          }
          else {
            // Send out a MIDI message on both usbMIDI and hardware MIDI ports
            usbMIDI.sendNoteOff((60 + note), 0, 2);
          }

        }
      }
    }

    // ============= CHECKING CAP C ===================================
    if (bitRead(currstateC, n) != bitRead(prevstateC, n)) {

      // If it has changed and is TRUE, then turn on that note
      if (bitRead(currstateC, n)) {
        // Send out a MIDI message on both usbMIDI and hardware MIDI ports
        usbMIDI.sendNoteOn(notesD[n], vel, 2);
        if (recording > -1 ) {
          chords[recording][n + 24] = 1;
        }
      } else {
        // Send out a MIDI message on both usbMIDI and hardware MIDI ports
        usbMIDI.sendNoteOff(notesD[n], 0, 2);
      }
    }

    // ============= CHECKING CAP D ===================================
    if (bitRead(currstateD, n) != bitRead(prevstateD, n)) {
      // If it has changed and is TRUE, then turn on that note
      if (bitRead(currstateD, n)) {
        // Send out a MIDI message on both usbMIDI and hardware MIDI ports
        usbMIDI.sendNoteOn(notesC[n], vel, 2);
        if (recording > -1 ) {
          chords[recording][n + 12] = 1;
        }
      } else {
        // Send out a MIDI message on both usbMIDI and hardware MIDI ports
        usbMIDI.sendNoteOff(notesC[n], 0, 2);
      }
    }
  }
}


// ============================ HELPER FUNCTIONS ============================
void checkCCpot() {
  currPotValue = analogRead(pot1) / 8; // Read the pin and divide by 8 to get a value from (0-127)
  // Check if the Potentiometer value has changed, and send a new CC message if it has
  if (currPotValue != prevPotValue) {
    usbMIDI.sendControlChange(CCpot, currPotValue, channel);
  }
  prevPotValue = currPotValue; // Update the prevPotValue for the next loop
}

void checkCCcap() {
  currcap = constrain(map(capA.filteredData(0), 40, 130, 120, 30), 30, 120);
  currcap = constrain(map(capB.filteredData(0), 40, 130, 120, 30), 30, 120);
  currcap = constrain(map(capC.filteredData(0), 40, 130, 120, 30), 30, 120);
  currcap = constrain(map(capD.filteredData(0), 40, 130, 120, 30), 30, 120);

  if (currcap != prevcap) {
    usbMIDI.sendControlChange(CCcap, currcap, channel);
  }
  prevcap = currcap;
}

// Check current selected octave based on position of slider on logarithmic potentiometer
void checkOctave() {
  // Check position
  currPotValue = analogRead(pot1);

  // Separate potentiometer into 5 sections
  if (currPotValue < 75)
  {
    octave = 0;
  } else if (currPotValue < 150)
  {
    octave = 1;
  } else if (currPotValue < 300)
  {
    octave = 2;
  } else if (currPotValue < 980)
  {
    octave = 3;
  } else // currPotValue > 980
  {
    octave = 4;
  }
  // If octave changed, copy over preset MIDI notes to the current active octaves array
  if (octave != prevOctave) {
    memcpy(notesA, octaves[octave * 3], sizeof(notesA));
    memcpy(notesC, octaves[octave * 3 + 1], sizeof(notesC));
    memcpy(notesD, octaves[octave * 3 + 2], sizeof(notesD));
    prevOctave = octave;
  }
}

// Clears stored notes in chord for key n
void clearChords (int n) {
  for (int i = 0; i < 36; i ++) {
    chords[n][i] = 0;
  }
}

// ======================= CHORD SELECTION =============================
void checkButton1() {
  // User pressed chord1 record button
  if (digitalRead(record1) == HIGH) {
    // Wait for user to release record button
    while (digitalRead(record1) == HIGH) {}
    clearChords(4);
    // Change state of the record button after the user releases
    // If button has been switched to on, turn off all other record buttons and turn led on
    digitalWrite(led1Pin, HIGH);
    // Until user turns off record button
    while (digitalRead(record1) == LOW) {
      record(4);
    }
    // Wait for user to release record button
    while (digitalRead(record1) == HIGH) {}
    digitalWrite(led1Pin, LOW);
  }
}

void checkButton2() {
  // User pressed chord2 record button
  if (digitalRead(record2) == HIGH) {
    // Wait for user to release record button
    while (digitalRead(record2) == HIGH) {}
    clearChords(3);
    // Change state of the record button after the user releases
    // If button has been switched to on, turn off all other record buttons and turn led on
    digitalWrite(led2Pin, HIGH);
    // Until user turns off record button
    while (digitalRead(record2) == LOW) {
      // Record notes that are pressed and save to chord 2
      record(3);
    }
    // Wait for user to release record button
    while (digitalRead(record2) == HIGH) {}
    digitalWrite(led2Pin, LOW);
  }
}

void checkButton3() {
  // User pressed chord3 record button
  if (digitalRead(record3) == HIGH) {
    // Wait for user to release record button
    while (digitalRead(record3) == HIGH) {}
    clearChords(2);
    // Change state of the record button after the user releases
    // If button has been switched to on, turn off all other record buttons and turn led on
    digitalWrite(led3Pin, HIGH);
    // Until user turns off record button
    while (digitalRead(record3) == LOW) {
      // Record notes that are pressed and save to chord 3
      record(2);
    }
    // Wait for user to release record button
    while (digitalRead(record3) == HIGH) {}
    digitalWrite(led3Pin, LOW);
  }
}

void checkButton4() {
  // User pressed chord4 record button
  if (digitalRead(record4) == HIGH) {
    // Wait for user to release record button
    while (digitalRead(record4) == HIGH) {}
    clearChords(1);
    // Change state of the record button after the user releases
    // If button has been switched to on, turn off all other record buttons and turn led on
    digitalWrite(led4Pin, HIGH);
    // Until user turns off record button
    while (digitalRead(record4) == LOW) {
      // Record notes that are pressed and save to chord1
      record(1);
    }
    // Wait for user to release record button
    while (digitalRead(record4) == HIGH) {}
    digitalWrite(led4Pin, LOW);
  }
}

// Store state of cap touch
void record(int chord) {
  currstateA = capA.touched();
  currstateB = capB.touched();
  currstateC = capC.touched();
  currstateD = capD.touched();
  checkCap(chord);

  // Only check the control changes 20 times a second so we dont overload
  // the MIDI-bus with too many messages
  if (timer > 20) {
    // Check Cap
    checkCCcap();
    timer = 0;
  }

  // Update our state
  prevstateA = currstateA;
  prevstateB = currstateB;
  prevstateC = currstateC;
  prevstateD = currstateD;
}
