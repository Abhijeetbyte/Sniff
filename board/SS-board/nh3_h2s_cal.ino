// MQ136 and MQ137 Calibration Code for 0 ppm in Clean Air
// Includes Rs averaging, Ro calibration, and PPM estimation

#include <Arduino.h>

// Sensor pins (ADC capable)
const int mq136Pin = 33;
const int mq137Pin = 32;

// RL values (in kOhms)
const float RL_MQ136 = 0.8;
const float RL_MQ137 = 0.7;

// Clean air Rs/Ro ratios (from datasheet)
const float RATIO_CLEAN_MQ136 = 10.0;
const float RATIO_CLEAN_MQ137 = 11.0;

const float VCC = 5.0;
const int numSamples = 50;

// Constants for logarithmic ppm estimation (from datasheet graphs)
const float M_MQ136 = -1.53;
const float B_MQ136 = 0.93;
const float M_MQ137 = -1.41;
const float B_MQ137 = 0.87;

float getAverageADC(int pin) {
  float adcSum = 0;
  for (int i = 0; i < numSamples; i++) {
    adcSum += analogRead(pin);
    delay(10);
  }
  return adcSum / numSamples;
}

float calculateRs(float adc, float RL) {
  float V_RL = adc * 3.3 / 4095.0;
  return ((VCC * RL) / V_RL) - RL;
}

float calculatePPM(float Rs, float Ro, float m, float b) {
  float ratio = Rs / Ro;
  return pow(10, ((log10(ratio) - b) / m));
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting MQ136 & MQ137 Calibration (0 ppm Clean Air)...");

  // === MQ136 Calibration ===
  float adc136 = getAverageADC(mq136Pin);
  float Rs136 = calculateRs(adc136, RL_MQ136);
  float Ro136 = Rs136 / RATIO_CLEAN_MQ136;
  float ppm136 = calculatePPM(Rs136, Ro136, M_MQ136, B_MQ136);

  Serial.println("\n[MQ136 - H2S]");
  Serial.printf("ADC: %.2f\n", adc136);
  Serial.printf("Rs: %.2f kOhm\n", Rs136);
  Serial.printf("Calibrated Ro: %.2f kOhm\n", Ro136);
  Serial.printf("Estimated PPM: %.2f ppm\n", ppm136);

  // === MQ137 Calibration ===
  float adc137 = getAverageADC(mq137Pin);
  float Rs137 = calculateRs(adc137, RL_MQ137);
  float Ro137 = Rs137 / RATIO_CLEAN_MQ137;
  float ppm137 = calculatePPM(Rs137, Ro137, M_MQ137, B_MQ137);

  Serial.println("\n[MQ137 - NH3]");
  Serial.printf("ADC: %.2f\n", adc137);
  Serial.printf("Rs: %.2f kOhm\n", Rs137);
  Serial.printf("Calibrated Ro: %.2f kOhm\n", Ro137);
  Serial.printf("Estimated PPM: %.2f ppm\n", ppm137);
}

void loop() {
  // nothing here; single-run calibration
}
