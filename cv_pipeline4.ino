
#include <EloquentTinyML.h>
#include "digits_model.h"
#define NUMBER_OF_INPUTS 150
#define NUMBER_OF_OUTPUTS 2
#define TENSOR_ARENA_SIZE 8*1024

Eloquent::TinyML::TfLite<
    NUMBER_OF_INPUTS,
    NUMBER_OF_OUTPUTS,
    TENSOR_ARENA_SIZE> ml;

#include <JPEGDecoder.h>
//### https://eloquentarduino.github.io/2020/11/tinyml-on-arduino-and-stm32-cnn-convolutional-neural-network-example/
#include <Adafruit_VC0706.h>
#include <SPI.h>
#include <SD.h>
#define SD_CS  12

    
#if defined(__AVR__) || defined(ESP8266)
#include <SoftwareSerial.h>         
SoftwareSerial cameraconnection(6, 7);
#else
#define cameraconnection Serial1
#endif

Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

#define chipSelect 3

File myFile;
uint32_t time1w;
uint32_t code_start;
float y_pred[2];
int y_test = 8;
  
void setup() {

#if !defined(SOFTWARE_SPI)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  if(chipSelect != 12) pinMode(12, OUTPUT); // SS on Mega
#else
  if(chipSelect != 12) pinMode(12, OUTPUT); // SS on Uno, etc.
#endif
#endif
  
  Serial.begin(9600);
  code_start = micros();
  
  digitalWrite(chipSelect, HIGH);
  Serial.println("VC0706 Camera snapshot test");
  digitalWrite(chipSelect, LOW);
  
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }  
  
  

  ml.begin(digits_model);                 
  
  time1w = micros() - code_start;
  
}


void initBuff(char* buff) {
  for(int i = 0; i < 240; i++) {
    buff[i] = 0;
  }
}

void loop() {
  if (cam.begin()) {
      Serial.println("Camera Found:");
    } else {
      Serial.println("No camera found?");
      return;
    }

  digitalWrite(chipSelect, LOW);
  time1w = micros();
  
  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
  
  char filename[13];
  strcpy(filename, "IMAGE00.JPG");
  for (int i = 0; i < 100; i++) {
    filename[5] = '0' + i/10;
    filename[6] = '0' + i%10;
    
    if (! SD.exists(filename)) {
      break;
    }
  }
  
  File imgFile = SD.open(filename, FILE_WRITE);
  if (!imgFile) {
        Serial.println("Open Failed !");
    }
    else{
        Serial.println("Open done !");
    }

  uint16_t jpglen = cam.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image.");
  ////Serial.println(filename);
  
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    uint8_t *buffer;
    uint8_t bytesToRead = min((uint16_t)32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    
    imgFile.write(buffer, bytesToRead);
      
    jpglen -= bytesToRead;
  }
  imgFile.close();
  digitalWrite(chipSelect, HIGH);
  uint32_t image_end = micros() - time1w;
  
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////
  pinMode(1, OUTPUT);
  char text_name[13];
  strcpy(text_name, filename);
  text_name[8] = 't' ;
  text_name[9] = 'x' ;
  text_name[10] = 't' ;
  myFile = SD.open(text_name, FILE_WRITE); 
  
  int x = 0;
  int y = 0;
  int row;
  int col;
  int mcuHeight = 8;
  int mcuWidth = 16;
  int mcu_pixels = mcuHeight * mcuWidth;
  int image_width = 160;
  int image_height= 120;
  int height_mcu = int(image_height/mcuHeight);
  int width_mcu = int(image_width/mcuWidth);
  int index = -1;
  int received = 0;
  int mcu_num;

  float sum = 0;
  int window_x = mcuHeight;
  int window_y = mcuWidth;
  float compressed_image[height_mcu * width_mcu];
  
  int height_window = int(image_height/window_x);
  int width_window = int(image_width/window_y);
  
  digitalWrite(chipSelect, HIGH);
  digitalWrite(chipSelect, LOW);
  if(!SD.begin(chipSelect)) {
    Serial.println("SD failed or not present!");
    while(1);
  }

    File jpgFile = SD.open(filename);
    if(!jpgFile) {
      Serial.println("not found");//break;
    }

    JpegDec.decodeSdFile(jpgFile);
    char dataBuff[240];

    initBuff(dataBuff);

    String header = "$ITHDR,";
    header += JpegDec.width;
    header += ",";
    header += JpegDec.height;
    header += ",";
    header += JpegDec.MCUSPerRow;
    header += ",";
    header += JpegDec.MCUSPerCol;
    header += ",";
    header += jpgFile.name();
    header += ",";
    header.toCharArray(dataBuff, 240);

    for(int j=0; j<240; j++) {
      Serial.write(dataBuff[j]);
    }
    uint16_t *pImg;

    uint16_t color;

    strcpy(dataBuff, "$ITDAT");
    uint8_t i = 6;
    while(JpegDec.read()) {
      pImg = JpegDec.pImage;
      int mcuXCoord = JpegDec.MCUx;
      int mcuYCoord = JpegDec.MCUy;
      uint32_t mcuPixels = JpegDec.MCUWidth * JpegDec.MCUHeight;
      while(mcuPixels--) {
        color = *pImg++;
        
        dataBuff[i] = color >> 8;
        dataBuff[i+1] = color;
        i += 2;

        if(i == 240) {
          for(int j=0; j<240; j++) {
    
            if ((j > 6) && ((j%2) == 0) && (received == 0)){
                sum = sum + int(dataBuff[j]);
                index = index + 1;
                mcu_num = int(index/mcu_pixels);
                row = mcu_num%width_mcu;
                col = int(mcu_num/width_mcu);
                x = x + 1;
                if (x == mcuWidth){
                    x = 0;
                    y = y + 1;
                }
        
                if (y == mcuHeight){
                    x = 0;
                    y = 0;
                    row = row + 1;
                    
                    sum = sum/window_x;
                    sum = sum/window_y;
                    compressed_image[col*width_mcu + row] = sum/255;
                    sum = 0;
                    myFile.println(float(compressed_image[col*width_mcu + row]));
                }
        
                if(row == width_mcu+1){
                    x = 0;
                    y = 0;
                    row = 1;
                    col = col + 1;

                    sum = sum/window_x;
                    sum = sum/window_y;
                    compressed_image[col*width_mcu + row] = sum/255;
                    sum = 0;
                    myFile.println(float(compressed_image[col*width_mcu + row]));
                }
        
                if (col == height_mcu+1){ 
                    received = 1;
                }
            }    
          }
          i = 6;
        }

        if((mcuXCoord == JpegDec.MCUSPerRow - 1) && 
          (mcuYCoord == JpegDec.MCUSPerCol - 1) && 
          (mcuPixels == 1)) {
          
          for(int j=0; j<i; j++) {
            ////Serial.println(int(dataBuff[j]));
            //myFile.println(int(dataBuff[j]));
          }
          
          for(int k=i; k<240; k++) {
            //Serial.write(0);
            ////Serial.println(0);
            //myFile.println(int(0));
          }
        }
      }
    }
  myFile.close();
  uint32_t convert_end = micros() - image_end;

  ml.predict(compressed_image, y_pred);
  uint32_t code_end = micros() - time1w;

  Serial.print("Test output is: ");
  Serial.print("Predicted proba are: ");
  for (int i = 0; i < 2; i++) {
      Serial.print(y_pred[i]);
      Serial.print(i == 9 ? '\n' : ',');
  }
  
  Serial.print("It took ");
  Serial.print(image_end/1000);
  Serial.print("  ");
  //Serial.print(convert_end/1000);
  //Serial.print("  ");
  Serial.print(code_end/1000);
  Serial.println(" s to run");

}
