
#include <EloquentTinyML.h>
#include "prey_car.h"
#define NUMBER_OF_INPUTS 150
#define NUMBER_OF_OUTPUTS 4
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

#define cameraconnection Serial1
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);
#define chipSelect 3

File myFile;
void setup() {
  
  Serial.begin(9600);
  delay(4000);
  uint32_t code_start = micros();
  
  digitalWrite(chipSelect, HIGH);
  Serial.println("VC0706 Camera snapshot test");
  digitalWrite(chipSelect, LOW);
    
  if (cam.begin()) {
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
    return;
  }
  
  cam.setImageSize(VC0706_160x120);         
  uint8_t imgsize = cam.getImageSize();
  if (imgsize == VC0706_160x120) Serial.println("160x120");
  
  delay(3000);   
  uint32_t time1w = micros() - code_start;
  Serial.print("It took ");
  Serial.print(time1w/1000);
  Serial.println(" s to run init");

    
  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
  
  char filename[13];
  strcpy(filename, "IMAGE00.JPG");
  for (int i = 0; i < 100; i++) {
    filename[5] = '0' + i/10;
    filename[6] = '0' + i%10;
    
  }
  
  uint16_t jpglen = cam.frameLength();
  uint32_t jpgArraySize = cam.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image.");
  uint8_t jpgArray[jpglen];
  int blocks = 0;
  
  int32_t time = millis();
  byte wCount = 0; 
  while (jpglen > 0) {
    uint8_t *buffer;
    uint8_t bytesToRead = min((uint16_t)32, jpglen); 
    buffer = cam.readPicture(bytesToRead);
    for(int g=0;g<bytesToRead;g++){
      jpgArray[g+blocks] = buffer[g];
    }
    blocks = blocks + bytesToRead;
      
    jpglen -= bytesToRead;
  }
  //delay(200);
  time = millis() - time;
  Serial.print(time); ////Serial.println(" ms elapsed");
  digitalWrite(chipSelect, HIGH);
  
  uint32_t image_end = micros() - time1w;
    Serial.print("It took ");
    Serial.print(image_end/1000);
    Serial.println(" s to run image");
    
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////
  pinMode(1, OUTPUT);
  //myFile = SD.open("test4 .txt", FILE_WRITE); 
  
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
  float orig_image[image_height][image_width];
  
  int height_window = int(image_height/window_x);
  int width_window = int(image_width/window_y);
  
  digitalWrite(chipSelect, HIGH);
  digitalWrite(chipSelect, LOW);
  
    //File jpgFile = SD.open(filename);
    //if(!jpgFile) {
    //  Serial.println("not found");//break;
    //}
    //JpegDec.decodeSdFile(jpgFile);
    JpegDec.decodeArray(jpgArray,jpgArraySize);
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
    //header += jpgFile.name();
    //header += ",";
    header.toCharArray(dataBuff, 240);
 
    for(int j=0; j<240; j++) {
      Serial.write(dataBuff[j]);
      myFile.println(dataBuff[j]);
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
            ////Serial.println(int(dataBuff[j]));
            myFile.println(int(dataBuff[j]));

            if ((j > 6) && ((j%2) == 0) && (received == 0)){
                sum = sum + int(dataBuff[j]);
                index = index + 1;
                mcu_num = int(index/mcu_pixels);
                row = mcu_num%width_mcu;
                col = int(mcu_num/width_mcu);
                ////Serial.println(row);
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
                    //Serial.println(compressed_image[col*width_mcu + row]);
                    //myFile.println(float(compressed_image[col*width_mcu + row]));
                }
        
                if(row == width_mcu+1){
                    x = 0;
                    y = 0;
                    row = 1;
                    col = col + 1;

                    // complete an MCU
                    sum = sum/window_x;
                    sum = sum/window_y;
                    compressed_image[col*width_mcu + row] = sum/255;
                    sum = 0;
                    //myFile.println(float(compressed_image[col*width_mcu + row]));
                    //Serial.println(compressed_image[col*width_mcu + row]);
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
            myFile.println(int(dataBuff[j]));
          }
          
          for(int k=i; k<240; k++) {
            myFile.println(int(0));
          }
        }
      }
    }
  myFile.close();

  uint32_t convert_end = micros() - image_end;
    Serial.print("It took ");
    Serial.print(convert_end/1000);
    Serial.println(" s to convert");
  
  ml.begin(prey_car);                 
    float y_pred[4];
    int y_test = 8;

    ml.predict(compressed_image, y_pred);

    uint32_t timeit = micros() - convert_end;

    Serial.print("It took ");
    Serial.print(timeit/1000);
    Serial.println(" ms to run inference");

    Serial.print("Test output is: ");
    Serial.print("Predicted proba are: ");
    for (int i = 0; i < 4; i++) {
        Serial.print(y_pred[i]);
        Serial.print(i == 9 ? '\n' : ',');
    }
    
    uint32_t code_end = micros() - code_start;
    Serial.print("It took ");
    Serial.print(code_end/1000);
    Serial.println(" s to run");
}

void initBuff(char* buff) {
  for(int i = 0; i < 240; i++) {
    buff[i] = 0;
  }
}
void loop() {
}
