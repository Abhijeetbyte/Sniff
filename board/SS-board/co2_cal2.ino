const int mq135Pin = 36;
const float RL_MQ135 = 0.7;
const float M_MQ135 = -1.303; 
const float B_MQ135 = 1.449;

const float VCC = 5.0;
const int numSamples = 50;

float getAverageADC(int pin) {
  float adcSum = 0;
  for (int i = 0; i < numSamples; i++) {
    adcSum += analogRead(pin);
    delay(10);
  }
  return adcSum / numSamples;
}

float calculateRs(float adc, float RL) {
  float V_esp32 = adc * 3.3 / 4095.0;
  float V_RL_actual = V_esp32 * (260.0 / 150.0); 
  return ((VCC * RL) / V_RL_actual) - RL;
}

float calculatePPM(float Rs, float Ro, float m, float b) {
  float ratio = Rs / Ro;
  return pow(10, ((log10(ratio) - b) / m));
}

float RoFromReference(float Rs, float ppmRef, float m, float b) {
  float expectedRatio = pow(10, ((log10(ppmRef) - b) / m));
  return Rs / expectedRatio;
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
}
void loop() {
  float adc135 = getAverageADC(mq135Pin);
  float Rs135 = calculateRs(adc135, RL_MQ135);
  float Ro135 = RoFromReference(Rs135, 500.0, M_MQ135, B_MQ135);
  float ppm135 = calculatePPM(Rs135, Ro135, M_MQ135, B_MQ135);

  
  Serial.printf("ADC: %.2f\n", adc135);
  Serial.printf("Rs: %.2f kOhm\n", Rs135);
  Serial.printf("Calculated Ro: %.2f kOhm\n", Ro135);
  Serial.printf("Estimated PPM: %.2f ppm\n", ppm135);

  delay(10000);
}
