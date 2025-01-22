/*
A multiband compressor attempting to emulate AM transmission. 
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <MD_AD9833.h>


// GUItool: begin automatically generated code
AudioInputI2S i2s1;         //xy=441,196
AudioFilterBiquad biquad1;  //xy=624,198
AudioAmplifier amp1;        //xy=771,198
AudioAnalyzePeak peak6;     //xy=918,268
AudioOutputI2S i2s2;        //xy=1001,202
AudioConnection patchCord1(i2s1, 1, biquad1, 0);
AudioConnection patchCord2(biquad1, amp1);
AudioConnection patchCord3(amp1, peak6);
AudioConnection patchCord4(amp1, 0, i2s2, 1);
AudioControlSGTL5000 audioShield;  //xy=482,496
// GUItool: end automatically generated code



const uint8_t PIN_DATA = 26;   ///< SPI Data pin number
const uint8_t PIN_CLK = 27;    ///< SPI Clock pin number
const uint8_t PIN_FSYNC = 28;  ///< SPI Load pin number (FSYNC in AD9833 usage)
MD_AD9833 AD(PIN_DATA, PIN_CLK, PIN_FSYNC);
const int myInput = AUDIO_INPUT_LINEIN;
;
float peakMax = .7;
float peakGain;
float rate = 0.001;
int lowpass = 5000;
int highpass = 100;
int gainLimit = 2;
float intGain;
int fCal = 8;

void setup() {
  Serial.begin(115200);
  AudioMemory(8);
  audioShield.enable();
  audioShield.inputSelect(myInput);
  audioShield.volume(0.5);
  biquad1.setHighpass(0, highpass, 1);
  biquad1.setLowpass(1, lowpass, 0.7);
  biquad1.setLowpass(2, lowpass, 1.4);
  biquad1.setLowpass(3, lowpass, 0.7);
  AD.begin();
  for (int i = 34; i <= 41; i++) {  // set inputs for DIP switches
    pinMode(i, INPUT_PULLUP);
  }
  getFreq();
  delay(1000);
}

void loop() {

  if (peak6.available()) {
    float Peak = peak6.read();
    if (Peak <= peakMax) {
      peakGain += rate;
    } else {
      peakGain -= (20 * rate);
      if (peakGain <= 0.1) { peakGain = 0.1; }
    }
    if (peakGain > gainLimit) { peakGain = gainLimit; }
    amp1.gain(peakGain);
    //Serial.println("peak1");
    //Serial.print(peakGain);
  }
}
void getFreq() {
  int channel = 0;
  unsigned long frequency = 0;
  if (!digitalRead(34)) { channel += 1; }  // read dip switches
  if (!digitalRead(35)) { channel += 2; }
  if (!digitalRead(36)) { channel += 4; }
  if (!digitalRead(37)) { channel += 8; }
  if (!digitalRead(38)) { channel += 16; }
  if (!digitalRead(39)) { channel += 32; }
  if (!digitalRead(40)) { channel += 64; }
  if (!digitalRead(41)) { channel += 128; }
  if (channel > 0 && channel <= 122) {  // calculate EU (9KHz) channels
    frequency = ((channel * 9) + 522);
    frequency *= 1000;
  }
  if (channel >= 129 && channel <= 247) {  // calculuate US (10KHz) channels
    frequency = (((channel - 128) * 10) + 520);
    frequency *= 1000;
  }
  Serial.println(frequency);
  AD.setFrequency(MD_AD9833::CHAN_0, (frequency + fCal));  // set ad9833
  delay(100);
}
