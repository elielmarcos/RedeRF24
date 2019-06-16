int i = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.print("sensor_1 ");
  Serial.println(i++);
  delay(1000);
}
