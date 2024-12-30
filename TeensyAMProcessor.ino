/*
Simple (and somwhat agressive) audio filter an processor for the MW transmitter project
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioInputI2S i2s1;                //xy=441,196
AudioAnalyzePeak peak3;            //xy=579,289
AudioAmplifier amp2;               //xy=629,195
AudioAnalyzePeak peak2;            //xy=784,276
AudioFilterBiquad biquad1;         //xy=798,192
AudioAmplifier amp1;               //xy=946,197
AudioFilterStateVariable filter1;  //xy=1179,193
AudioAnalyzePeak peak1;            //xy=1480,336
AudioAnalyzePeak peak4;            //xy=1495,372
AudioAnalyzePeak peak5;            //xy=1511,416
AudioMixer4 mixer1;                //xy=1576,216
AudioOutputI2S i2s2;               //xy=1809,214
AudioConnection patchCord1(i2s1, 1, amp2, 0);
AudioConnection patchCord2(i2s1, 1, peak3, 0);
AudioConnection patchCord3(amp2, peak2);
AudioConnection patchCord4(amp2, biquad1);
AudioConnection patchCord5(biquad1, amp1);
AudioConnection patchCord6(amp1, 0, filter1, 0);
AudioConnection patchCord7(filter1, 0, mixer1, 0);
AudioConnection patchCord8(filter1, 0, peak1, 0);
AudioConnection patchCord9(filter1, 1, mixer1, 1);
AudioConnection patchCord10(filter1, 1, peak4, 0);
AudioConnection patchCord11(filter1, 2, mixer1, 2);
AudioConnection patchCord12(filter1, 2, peak5, 0);
AudioConnection patchCordgainLimit(mixer1, 0, i2s2, 1);
AudioControlSGTL5000 audioShield;

const int myInput = AUDIO_INPUT_LINEIN;  //const int myInput = AUDIO_INPUT_MIC;
float peakMax = 1;
float peakGain;
float rate = 0.01;
int lowpass = 6000;
float filterGain;
int LEDTime;
float midGain;
float hiGain;
int gainLimit = 8;

void setup() {
  AudioMemory(128);
  audioShield.enable();
  audioShield.inputSelect(myInput);
  audioShield.volume(0.5);
  biquad1.setHighpass(0, 80, 0.7);
  biquad1.setLowpass(1, lowpass, 0.7);
  biquad1.setLowpass(2, lowpass, 0.7);
  biquad1.setLowpass(3, lowpass, 0.7);
  amp1.gain(1);
  amp2.gain(0.5);
  mixer1.gain(0, 0.5);
  mixer1.gain(1, 0.5);
  mixer1.gain(2, 0.5);
  //AudioNoInterrupts();
  //Serial.begin(9600);
}

void loop() {
  //Serial.print(AudioProcessorUsageMax()); Serial.print (" "); Serial.println (AudioMemoryUsageMax());
  if (peak1.available()) {
    float monoPeak = peak1.read();
    if (monoPeak < peakMax) {
      peakGain += rate;
    } else {
      peakGain -= (5 * rate);
      if (peakGain <= 0) { peakGain = 0; }
    }
    if (peakGain > gainLimit) { peakGain = gainLimit; }
    mixer1.gain(0, peakGain);
    //Serial.println(peakGain);
  }
  if (peak2.available()) {
    float filterPeak = (peak2.read());

    if (filterPeak <= .5) {
      filterGain += 1;
    } else {
      filterGain -= 1;
    }
    if (filterGain > 4) { filterGain = 4; }
    amp2.gain((filterGain / 10));
  }
  if (peak3.available()) {
    float inputClip = peak3.read();

    if (inputClip >= .7) {
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
      if (midGain <= 0) { midGain = 0; }
    }
    if (midGain > gainLimit) { midGain = gainLimit; }
    mixer1.gain(1, midGain);
  }
  if (peak5.available()) {
    float monoPeak = peak5.read();
    if (monoPeak < peakMax) {
      hiGain += rate;
    } else {
      hiGain -= (5 * rate);
      if (hiGain <= 0) { midGain = 0; }
    }
    if (hiGain > gainLimit) { hiGain = gainLimit; }
    mixer1.gain(2, hiGain);
  }
}
