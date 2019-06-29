int i = 0;
int sensor1 = 23, sensor2 = 23, sensor3 = 23;

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));
}

void loop() {
  sensor1 += random(-1, 2);
  sensor2 += random(-1, 2);
  sensor3 += random(-1, 2);
  
  if (random(10) > 5)
    Serial.println("sensor_1 " + String(sensor1));
  if (random(10) > 0)  
    Serial.println("sensor_2 " + String(sensor2));
  if (random(10) > 0)
    //Serial.println("sensor_3 " + String(sensor3));
  delay(500);
}
