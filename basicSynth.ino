/* DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 

 Copyright (C) 2016 Logan McCandless

 Everyone is permitted to copy and distribute verbatim or modified 
 copies of this license document, and changing it is allowed as long 
 as the name is changed. 
*/

IntervalTimer audioTimer;

#define phaseRes 627
#define acGround 2048

const float tune = 70 - 0.65354;
unsigned int LSin[627];
bool notePlaying = true;
float midiToFreq[127];
int currentNote = 0;

float waveIndex = 0;
float waveIndexIncrement = 1;
int wavType = 0;

void setup(void) {
  // fill sine look up table
  for (int i = 0; i < phaseRes; i++){
    LSin[i] = acGround + ( acGround*(sin(i/100.0)));
  }
  int a = 440; 
  // fill midi note to frequency table
  for (int x = 0; x < 127; ++x){
     midiToFreq[x] = ((a / 32.0) * (pow(2 , ((x - 9) / 12.0))))/tune;
  }
  
  pinMode(16, OUTPUT);
  analogWriteFrequency(16,88200);
  usbMIDI.setHandleNoteOn(OnNoteOn) ;
  usbMIDI.setHandleNoteOff(OnNoteOff) ;
  usbMIDI.setHandleControlChange(OnControlChange);
  audioTimer.begin(dacIsr, 23);  //44.1khz
  analogWriteRes(13);
  pinMode(A12, OUTPUT);
}

void dacIsr(void) {
  int dacOut = 0;
  waveIndex+=waveIndexIncrement;
  if (waveIndex >= phaseRes)
    waveIndex  -=phaseRes;

  //Configured for similar volumes across waveTypes
  switch(wavType) {
    case 0: //sine
      dacOut = LSin[(int)waveIndex]/2;
      break;
     case 1: //tri
      dacOut = acGround + 6*( (waveIndex<phaseRes/2) ? (waveIndex-phaseRes/4) : (waveIndex-phaseRes*3/4)*-1);
      break;
    case 2: //saw
      dacOut = acGround + -3*(waveIndex-phaseRes/2);
      break;
    case 3: //square
      dacOut = (waveIndex>phaseRes/2) ? acGround/2 : (acGround+acGround/2);
      break;
  }

  if (notePlaying)
    analogWrite(A12, dacOut);
  else
    analogWrite(A12, acGround);
}

void loop(void) {
  while (usbMIDI.read());
}

void OnNoteOn(byte channel, byte note, byte velocity){
  notePlaying = true;
  waveIndexIncrement=midiToFreq[note];
  currentNote = note;
}

void OnControlChange(byte channel, byte control, byte value){
  switch(control){
    case 20:
      wavType = min(map(value, 0, 127,0,4),3);
  }
}

void OnNoteOff(byte channel, byte note, byte velocity){
  if (currentNote == note)
   notePlaying = false;
}


