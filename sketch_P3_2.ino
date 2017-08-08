

const int sensorPin = A0;
const float baselineTemp = 19.0;

void setup() {
 Serial.begin(9600); //open a serial port



for (int pinNumber = 2; pinNumber<5; pinNumber++) {
      pinMode(pinNumber, OUTPUT);
      digitalWrite(pinNumber, LOW);
}
}

void loop() {
  int sensorVal = analogRead (sensorPin);
  Serial.print("Sensor Value: ");
  Serial.print(sensorVal);
  
  float voltage = (sensorVal/1024.0) * 5.0;
  Serial.print(" , Volts: ");
  Serial.print(voltage);
  Serial.print(" , degrees C: ");
  // convert the voltage to tempretuare in degrees
  float temperature = (voltage - 0.5) * 100;
  Serial.print(temperature);
  
  Serial.print(" , , Faisal , , ");
  Serial.println(temperature+2);
  
  if(temperature < baselineTemp) {
    digitalWrite (2, HIGH);
  digitalWrite (3, LOW);
  digitalWrite (4, HIGH);
  delay(100);
  digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW); 
    delay(100);
    digitalWrite (2, HIGH);
  digitalWrite (3, HIGH);
  digitalWrite (4, HIGH);
  delay(100);
  digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW); 
} else if(temperature >= baselineTemp+2 && temperature < baselineTemp+3) {
  digitalWrite (2, HIGH);
  digitalWrite (3, LOW);
  digitalWrite (4, LOW);
  
} else if(temperature >= baselineTemp+3 && temperature < baselineTemp+5) {
  digitalWrite (2, HIGH);
  digitalWrite (3, HIGH);
  digitalWrite (4, LOW);
  
} else if(temperature >= baselineTemp+5) { 
  digitalWrite (2, HIGH);
  digitalWrite (3, HIGH);
  digitalWrite (4, HIGH);  
}
delay(100);
}

