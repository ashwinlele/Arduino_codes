
int outPin = 1;               // digital pin 8
//int myDelay = 100;

void setup() {
  pinMode(outPin, OUTPUT);    // sets the digital pin as output
      int frequency = 10000;
      int myDelay = 1000000/frequency/2 - 4; // 4 us taken by each digital write
      
      for (int i=0; i<1000*frequency; i++){ // actuates for 10 seconds
          digitalWrite(outPin, HIGH); // sets the pin on99
          delayMicroseconds(myDelay);      // pauses for 50 microseconds  7 us is execution time.
          digitalWrite(outPin, LOW);  // sets the pin off
          delayMicroseconds(myDelay);      // pauses for 50 microseconds
      }
}
/*
  ////////////// 100-1k ////////////////////////
  for(int j=1; j<10; j++){
      int frequency = j*100;
      int myDelay = 1000000/frequency/2 - 4; // 4 us taken by each digital write
      
      for (int i=0; i<10*frequency; i++){ // actuates for 10 seconds
          digitalWrite(outPin, HIGH); // sets the pin on99
          delayMicroseconds(myDelay);      // pauses for 50 microseconds  7 us is execution time.
          digitalWrite(outPin, LOW);  // sets the pin off
          delayMicroseconds(myDelay);      // pauses for 50 microseconds
      }
      delay(3000);
  }
  ///////////// 1k-10k ///////////////////////////
  for(int j=1; j<20; j++){
      int frequency = 1000 + 500*j;
      int myDelay = 1000000/frequency/2 - 4; // 4 us taken by each digital write
      
      for (int i=0; i<10*frequency; i++){ // actuates for 10 seconds
          digitalWrite(outPin, HIGH); // sets the pin on99
          delayMicroseconds(myDelay);      // pauses for 50 microseconds  7 us is execution time.
          digitalWrite(outPin, LOW);  // sets the pin off
          delayMicroseconds(myDelay);      // pauses for 50 microseconds
      }
      delay(3000);
  }
}
*/
void loop() {

}

