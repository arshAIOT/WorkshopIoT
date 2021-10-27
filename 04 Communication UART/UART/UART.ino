const int led_pin = 2;

void setup() {
  // put your setup code here, to run once:

  pinMode(led_pin,OUTPUT);
  delay(50);
  //start serial communication
  Serial.begin(9600);

}//close setup

void loop() {

  if(Serial.available()>0){
   int command = Serial.read();
   switch (command){
    case 'a':
    digitalWrite(led_pin, HIGH);
    case 'b':
    digitalWrite(led_pin,LOW);
    break; 
   }
  }
}//close loop
