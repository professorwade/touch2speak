#define DEBUG 1
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <sdios.h>

// touchscreen includes
#include <Adafruit_GFX.h>
#include <TouchScreen.h>
#include "Adafruit_HX8357.h"
#include <Adafruit_ImageReader.h>

#define YP A8  // must be an analog pin, use "An" notation!
#define XM A11  // must be an analog pin, use "An" notation! 
#define YM A10   // can be a digital pin
#define XP A9   // can be a digital pin          
#define TS_MINX 170   // 150
#define TS_MINY 105   // 120
#define TS_MAXX 870   // 920
#define TS_MAXY 920   // 940

// aligns cursor with touch point
#define X_FUDGE 115
#define Y_FUDGE 120

// define touch pressure range
#define MINPRESSURE 800
#define MAXPRESSURE 1000

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
SdFat                SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys

// Serial TFT Access Lines
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST -1 // RST can be set to -1 if you tie it to Arduino's reset

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

void readFiles(File dir, char files[][13], int16_t size);

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

#define MAX_ICONS 24
char files[MAX_ICONS][13];

void setup() {
  Serial.begin(9600);
  Serial.println("Touchscreen Assistive Device Prototype");

  if(!SD.begin(PIN_SPI1_SS, SD_SCK_MHZ(25))) { // ESP32 requires 25 MHz limit
    Serial.println(F("SD begin() failed"));
  }

  tft.begin();
  tft.fillScreen(BLACK);
  
  /* Draw Grid */
  readFiles(SD.open("/ICONS/"), files);

  int16_t ndx = 0;
  for (int16_t i = 0; i < tft.width(); i += 80) {
    for (int16_t j = 0; j < tft.height(); j += 80) {
        char fullpath[25] = "/ICONS/\0";
        strncat(fullpath, files[ndx], 13);
        reader.drawBMP(fullpath, tft, i, j);        
        ndx++;
    }
  }
  Serial.println("Symbols Loaded - Unit Ready");  
}

void loop() {  

  //Serial.println("Touchscreen Assistive Device Prototype");
  TSPoint p = ts.getPoint();
  p.x = TS_MAXX - p.x + X_FUDGE;
  p.y = TS_MAXY - p.y + Y_FUDGE;


  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {

    Serial.println("Touch Detected");
    // scale from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);

    int16_t zone = getGrid(p.x,  p.y);
    char filename[25] = "/SOUNDS/\0";
    strncat(filename, files[zone], 13);
    char* ndx = strchr(filename,'.');
    strncpy(ndx, ".WAV", 4); 

    // Play WAV File Here
    

    //

    if (DEBUG) {
      Serial.print("Coord: ");
      Serial.print(p.x);
      Serial.print(",");
      Serial.print(p.y);
      Serial.print(',');
      Serial.println(p.z);
      Serial.print("Zone:");
      Serial.println(zone);
      Serial.print("Sound File: ");
      Serial.println(filename);
    }
    delay(100);
  }
}

// Get grid location of touch
uint8_t getGrid(int16_t x, int16_t y) {
  int16_t row = 0;
  int16_t column = 0;
  
  // get row
  if (x > 80 && x <= 160)
    row = 1;
  else if (x > 160 && x <= 240)
    row = 2;
  else if (x > 240 && x < 320)
    row = 3;

  // get column
  if (y > 80 && y <= 160)
    column = 1;
  else if (y > 160 && y <= 240)
    column = 2;
  else if (y > 240 && y < 320)
    column = 3;
  else if (y > 320 && y < 400)
    column = 4;
  else if (y > 400 && y < 480)
    column = 5;

  return row * 6 + column;
}

void readFiles(File dir, char files[][13]) {
    int16_t ndx = 0;
    while(true) {     
    File entry =  dir.openNextFile();

    if (! entry) {
       // no more files
       break;
    }
    char tmp[13];
    entry.getName(tmp,13);
    if (ndx < MAX_ICONS && tmp[0] != '_' && tmp[0] != '.' && strlen(tmp) > 1) {
      strncpy(files[ndx], tmp, 13);
      ndx++;
    }
  }
}



