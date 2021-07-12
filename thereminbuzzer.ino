//www.elegoo.com
//2016.12.08
#include "SR04.h"
#include "pitches.h"
/*
#include <pitchToFrequency.h>
#include <pitchToNote.h>
#include <frequencyToNote.h>
#include <MIDIUSB_Defs.h>
#include <MIDIUSB.h>
*/
//INPUTS
//distance detection
#define TRIG_PIN 12
#define ECHO_PIN 10 
SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);

//rotary
#define encoder0PinA 2
#define encoder0PinB 3
//#define encoder0Btn 4

//joystick
#define Vx A0 // Define / Equate "Vx" with A0, the pin where Vx is connected
#define Vy A1 // Define / Equate "Vy" with A1, the pin where Vy is connected
#define Button A2 // Define / Equate Button with A2, the pin where the button is connected

int pitches[] = {
  NOTE_C1,  NOTE_CS1, NOTE_D1, NOTE_DS1, NOTE_E1,  NOTE_F1, 
  NOTE_FS1, NOTE_G1,  NOTE_GS1,NOTE_A1,  NOTE_AS1, NOTE_B1, 
  NOTE_C2,  NOTE_CS2, NOTE_D2, NOTE_DS2, NOTE_E2,  NOTE_F2, 
  NOTE_FS2, NOTE_G2,  NOTE_GS2,NOTE_A2,  NOTE_AS2, NOTE_B2, 
  NOTE_C3,  NOTE_CS3, NOTE_D3, NOTE_DS3, NOTE_E3,  NOTE_F3, 
  NOTE_FS3, NOTE_G3,  NOTE_GS3,NOTE_A3,  NOTE_AS3, NOTE_B3, 
  NOTE_C4,  NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4,  NOTE_F4, 
  NOTE_FS4, NOTE_G4,  NOTE_GS4,NOTE_A4,  NOTE_AS4, NOTE_B4, 
  NOTE_C5,  NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5,  NOTE_F5, 
  NOTE_FS5, NOTE_G5,  NOTE_GS5,NOTE_A6,  NOTE_AS6, NOTE_B6, 
  NOTE_C7,  NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7,  NOTE_F7, 
  NOTE_FS7, NOTE_G7,  NOTE_GS7,NOTE_A7,  NOTE_AS7, NOTE_B7, 
  NOTE_C8,  NOTE_CS8, NOTE_D8, NOTE_DS8
};

#define MAJ 0
#define MIN 1
#define DIM 2
#define AUG 3
int chords[4][13] = {{0, 4, 7, 12, 16, 19, 24, 28, 31, 36, 40, 43, 48},   //MAJ
                     {0, 3, 7, 12, 15, 19, 24, 27, 31, 36, 39, 43, 48},   //MIN
                     {0, 3, 6, 12, 15, 18, 24, 27, 30, 36, 39, 42, 48},   //DIM
                     {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48}};  //AUG
// THEREMIN MODES
#define ARP 1
#define OOB -1

// global variables
long recDist;
int pitch; 
int startingTempo = 600;
int tempo = startingTempo;
int waitTime = 60000/tempo;
int valRotary,lastValRotary;
int encoder0Pos = 0;
int rotaryScale = 10;
int joyX, joyY, joyBtn;
int mode;
int chord;
int numNotes = 1;
         
void setup() {
   Serial.begin(9600);//Initialization of Serial Port
   pinMode(encoder0PinA, INPUT_PULLUP);
   pinMode(encoder0PinB, INPUT_PULLUP);
   //pinMode(encoder0Btn, INPUT_PULLUP);

   pinMode(Vx, INPUT); // Configure Vx (A0) as an Input
   pinMode(Vy, INPUT); // Configure Vy (A1) as an Input
   pinMode(Button, INPUT_PULLUP); // Configure Button (A2) as an Input, internally "pulled-up" to 5V
                                 // Note, we're configuring an Analog input as digital input
                                 // which is perfectly fine.  I did this to make the wiring easier
                                 // and keep all of the wires on the same side of the board
  
   attachInterrupt(0, doEncoder, CHANGE);
   delay(1000);
}

void loop() {
  // Data recorded from modules
  joyX = analogRead(Vx); // Read the analog value of Vx (Analog Values are from 0-1023 which equate to 0V to 5V)
  joyY = analogRead(Vy); // Read the analog value of Vy
  joyBtn = digitalRead(Button); // Read the button.  When the button is open (unpushed),
                             // the input will read High (+5V)
                             // When the button is closed (pressed), the input pin
                             // is connected to ground and will read Low (0V)
  recDist=sr04.Distance();
  
  // Process Data
  numNotes = getNumNotes(joyBtn);
  chord = getChord(joyX,joyY);
  pitch = distanceToPitch(recDist);
  
  mode = ARP;
  if (pitch == -1)
    mode = OOB;  
  switch(mode) {
    //case MOD: modulator(); displayReadout(); break;
    case ARP: arpeggiator(); displayReadout(); break;
    case OOB: break;
    default: theremin();
  }
}
void displayReadout() {
  Serial.print("\n\npitch = ");
  Serial.println(pitch);
  Serial.print("chord = ");
  Serial.println(chord);
  Serial.print("(x,y) = ");
  Serial.print(joyX);
  Serial.print(",");
  Serial.println(joyY);
  Serial.print("numNotes = ");
  Serial.println(numNotes);
  Serial.print("joy Button = ");
  Serial.println(joyBtn);
  
}
int distanceToPitch(int dist) {
    if (dist < 88 && dist > 0)
      return dist/3;
    else 
      return -1;
}
// NOTE: Y is inverted, up is 0, down is 1023
int getChord(int x, int y) {
  int center = 1023/2;
  if(x < center && y < center) 
    return DIM;
  if(x >= center && y < center) 
    return MIN;
  if(x <= center && y >= center) 
    return MAJ;
  if(x > center && y >= center) 
    return AUG;   
}
int getNumNotes(int btn) {
  if (!btn) 
    return (numNotes + 1) % 13;
  else 
    return numNotes;
}
void arpeggiator() {
    for (int i = 0; i <= numNotes; i++) {
      tone(8, pitches[pitch + chords[chord][i] ], waitTime);
      delay(waitTime);
    }
}
void theremin() {
    tone(8, pitches[pitch], waitTime);
    delay(waitTime);
}

//rotary knob 
void doEncoder() {
  if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB))
    encoder0Pos-=rotaryScale;
  else
    encoder0Pos+=rotaryScale;
  tempo = startingTempo + encoder0Pos;
  waitTime = 60000/tempo;
  Serial.print("encoder0Pos = ");
  Serial.println(encoder0Pos);
}
