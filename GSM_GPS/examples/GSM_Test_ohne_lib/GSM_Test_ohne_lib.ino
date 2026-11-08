// KEIN include von GSM.h, weil wir die Funktion selbst nachbilden
#define GSM_POWER_PIN 7
#define GSM_SERIAL Serial1
#define DEBUG_SERIAL Serial 
 
void setup(){
  GSM_SERIAL.begin(9600);
  DEBUG_SERIAL.begin(9600);
  
  pinMode(GSM_POWER_PIN,OUTPUT);
  digitalWrite(GSM_POWER_PIN, HIGH);    
 
  delay(2000);  
}
 
void loop(){
  if(GSM_SERIAL.available()){
    DEBUG_SERIAL.write(GSM_SERIAL.read());
  }
  if(DEBUG_SERIAL.available()){
    // handle CTRL+Z (we use $ in serial monitor
    if(DEBUG_SERIAL.peek() == '$')
    {
      char tmp = DEBUG_SERIAL.read();
      GSM_SERIAL.write(26); // CRTL+Z
    }else{
      GSM_SERIAL.write(DEBUG_SERIAL.read());
    }
  }
}
