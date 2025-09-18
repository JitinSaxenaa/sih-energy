const int analogPin = A0;  // Sensor output to A0

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n📟 Voltage Sensor Direct Reading...");
}

void loop() {
  int sensorValue = analogRead(analogPin);

  // Convert ADC value (0–1023) to actual volts at A0
  float voltageA0 = (sensorValue * 3.3*4.6) / 1023.0;

  Serial.print("Raw ADC: ");
  Serial.print(sensorValue);
  Serial.print(" | Voltage at A0: ");
  Serial.print(voltageA0);
  Serial.println(" V");

  delay(1000);
}
