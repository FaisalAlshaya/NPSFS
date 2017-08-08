int potpin = A0;
int led = 9;
float readvalue;
float voltage;
float ledon;



void setup () {
  Serial.begin (9600);
  pinMode(potpin, INPUT);
  pinMode(led, OUTPUT);
} 
  
  void loop () {
    
  readvalue = analogRead(potpin);
  voltage = (5./1023.) * readvalue;
  ledon= 51. * voltage;
 
 analogWrite(led, ledon); 
  
  Serial.print( " pot value = ");
  Serial.print(readvalue);
  Serial.print( " " );
  Serial.print( " Voltage = ");
  Serial.print(voltage);
  Serial.print( " " );
  Serial.print( " Write Value = ");
  Serial.println(ledon);
  
  }
