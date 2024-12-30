/*
A multiband compressor attempting to emulate AM transmission. 
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <MD_AD9833.h>

AudioInputI2S i2s1;                //xy=441,196
AudioAnalyzePeak peak3;            //xy=591,273
AudioAmplifier amp2;               //xy=629,195
AudioAnalyzePeak peak2;            //xy=784,276
AudioFilterBiquad biquad1;         //xy=798,192
AudioAmplifier amp1;               //xy=993,191
AudioAnalyzePeak peak6;            //xy=1110,258
AudioFilterStateVariable filter1;  //xy=1185,193
AudioAnalyzePeak peak5;            //xy=1481,368
AudioAnalyzePeak peak4;            //xy=1483,330
AudioAnalyzePeak peak1;            //xy=1484,292
AudioMixer4 mixer1;                //xy=1579,214
AudioOutputI2S i2s2;               //xy=1809,214
AudioConnection patchCord1(i2s1, 1, amp2, 0);
AudioConnection patchCord2(i2s1, 1, peak3, 0);
AudioConnection patchCord3(amp2, peak2);
AudioConnection patchCord4(amp2, biquad1);
AudioConnection patchCord5(biquad1, amp1);
AudioConnection patchCord6(amp1, 0, filter1, 0);
AudioConnection patchCord7(amp1, peak6);
AudioConnection patchCord8(filter1, 0, mixer1, 0);
AudioConnection patchCord9(filter1, 0, peak1, 0);
AudioConnection patchCord10(filter1, 1, mixer1, 1);
AudioConnection patchCord11(filter1, 1, peak4, 0);
AudioConnection patchCord12(filter1, 2, mixer1, 2);
AudioConnection patchCord13(filter1, 2, peak5, 0);
AudioConnection patchCord14(mixer1, 0, i2s2, 1);
AudioControlSGTL5000 audioShield;

const int myInput = AUDIO_INPUT_LINEIN;  //const int myInput = AUDIO_INPUT_MIC;
float peakMax = .9;
float peakGain;
float rate = 1;
int lowpass = 6000;
int highpass = 100;
float filterGain;
int LEDTime;
float midGain;
float hiGain;
int gainLimit = 3;
float intGain;

void setup() {
  AudioMemory(128);
  audioShield.enable();
  audioShield.inputSelect(myInput);
  audioShield.volume(0.5);
  biquad1.setHighpass(0, highpass, 0.7);
  biquad1.setLowpass(1, lowpass, 0.7);
  biquad1.setLowpass(2, lowpass, 0.7);
  biquad1.setLowpass(3, lowpass, 0.7);
  amp1.gain(1);
  amp2.gain(0.5);
  mixer1.gain(0, 0.5);
  mixer1.gain(1, 0.5);
  mixer1.gain(2, 0.5);
  filter1.frequency(lowpass/2);
  filter1.resonance(1);
}

void loop() {
  
  if (peak1.available()) {
    float monoPeak = peak1.read();
    if (monoPeak < peakMax) {
      peakGain += rate;
    } else {
      peakGain -= (5 * rate);
      if (peakGain <= 0.1) { peakGain = 0.1; }
    }
    if (peakGain > gainLimit) { peakGain = gainLimit; }
    mixer1.gain(0, peakGain);
    //Serial.println(peakGain);
  }
  if (peak2.available()) {
    float filterPeak = (peak2.read());

    if (filterPeak <= .7) {
      filterGain += .5;
    } else {
      filterGain -= 2;
    }
    if (filterGain > 2) { filterGain = 2; }
    amp2.gain((filterGain / 10));
  }
  if (peak3.available()) {
    float inputClip = peak3.read();
    if (inputClip >= .5) {
      digitalWrite(gainLimit, HIGH);
      LEDTime = millis();
    } else {
      if (LEDTime + 50 <= millis()) {
        digitalWrite(gainLimit, LOW);
      }
    }
  }
  if (peak4.available()) {
    float monoPeak = peak4.read();
    if (monoPeak < peakMax) {
      midGain += rate;
    } else {
      midGain -= (5 * rate);
      if (midGain <= 0.1) { midGain = 0.1; }
    }
    if (midGain > gainLimit) { midGain = gainLimit; }
    mixer1.gain(1, midGain);
  }
  if (peak5.available()) {
    float monoPeak = peak5.read();
    if (monoPeak < peakMax) {
      hiGain += rate;
    } else {
      hiGain -= ( rate);
      if (hiGain <= .1) { hiGain = 0.1; }
    }
    if (hiGain > gainLimit) { hiGain = gainLimit; }
    mixer1.gain(2, hiGain);
  }
  if (peak6.available()) {
    float monoPeak = peak6.read();
    if (monoPeak < 0.5) {
      intGain += rate;
    } else {
      intGain -= (5 * rate);
      if (intGain <= 0.1) { intGain = 0.1; }
    }
    if (intGain > gainLimit) { intGain = gainLimit; }
    amp1.gain(intGain);
  }
}