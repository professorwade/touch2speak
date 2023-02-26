

// EGR 122 Touch Screen Assistive Device Prototype Proof-of-Concept
#include <Arduino.h>
#define DEBUG 1 // uncomment for extra debug info
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <sdios.h>

#include "SDWaveFile.hh"
#include <Adafruit_ZeroI2S.h>

// WAV File Player
//#define I2S_DEVICE 1
/* max volume for 32 bit data */
/* max volume for 32 bit data */


#define VOLUME 0.2


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
// tweak as necessary
#define X_FUDGE 115
#define Y_FUDGE 120

// define touch pressure range
#define MINPRESSURE 800     // SAMD51 much more sensitive A/D converter, need to boost upper limit
#define MAXPRESSURE 1000

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
SdFat                SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
SDWaveFile waveFile; // only one file active at a time
Adafruit_ZeroI2S player = Adafruit_ZeroI2S();
int32_t playout_buffer[256];

// Serial TFT Access Lines
#define TFT_CS 10     // touch screen chip select
#define TFT_DC 9      // touch screen data / command
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

// function to read all the files from the specified directory
void readFiles(File dir, char files[][13], int16_t size);

// Use hardware SPI above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

#define MAX_ICONS 24
char files[MAX_ICONS][13];

void setup() {
  Serial.begin(9600);
  Serial.println("Touchscreen Assistive Device Prototype Starting Up");

  // initialize SPI interface
  if(!SD.begin(PIN_SPI1_SS, SD_SCK_MHZ(25))) { // ESP32 requires 25 MHz limit
    Serial.println(F("SD begin() failed"));
  }

  // crank up display and init to black
  tft.begin();
  tft.fillScreen(BLACK);
  
  // Draw Grid
  readFiles(SD.open("/ICONS/"), files);  // returns with file names in files  
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
  // open question on github about whether or not the TouchScreen library needs to be tweaked  
  TSPoint p = ts.getPoint();

  // recommmend an averaging sample here
  // get x and y coordinates
  p.x = TS_MAXX - p.x + X_FUDGE;
  p.y = TS_MAXY - p.y + Y_FUDGE;

  // if pressure (touch) detected
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {

    // scale from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);

    int16_t zone = getGrid(p.x,  p.y);
    char filename[25] = "/SOUNDS/\0";
    strncat(filename, files[zone], 13);
    char* ndx = strchr(filename,'.'); // build sound file name
    strncpy(ndx, ".WAV", 4); 

    // Play WAV File Here
    waveFile = SDWaveFile(&SD, filename);

    // check if the WaveFile is valid
    if (!waveFile) {
      Serial.print("There is no .wav file called ");
      Serial.println(filename);
    }
    // print the file's duration:
    long duration = waveFile.duration();
    Serial.print("Duration = ");
    Serial.print(duration);
    Serial.println(" seconds");

    Serial.print("Sample Rate: ");
    Serial.println(waveFile.sampleRate());

    Serial.print("Bits Per Sample: ");
    Serial.println(waveFile.bitsPerSample());

    Serial.print("Channels: ");
    Serial.println(waveFile.channels());

    player.begin(I2S_32_BIT, 44100);    
    player.enableTx();
    int sz = sizeof(playout_buffer);
    Serial.print("Size: ");
    Serial.println(sz);
    int n = sz;
    Serial.println(VOLUME);
    waveFile.begin();
    while (n > 0) {    
      n = waveFile.read(&playout_buffer, sizeof(playout_buffer));
      //Serial.print("Read: ");
      //Serial.println(n);
      int max = n / 4;
      for (int i = 0; i < max; i += 1) {
        int32_t left = (int32_t)(playout_buffer[i] * VOLUME);
        int32_t right = (int32_t)(playout_buffer[i] * VOLUME);
        player.write(left, right);
      }
    }  
    waveFile.end();   
    player.disableTx(); 

    if (DEBUG) {
      Serial.print("Filename: ");
      Serial.println(filename);
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
    delay(100); // slow down the sampling, remove when averaging implemented
  }
}

// Get grid location of touch
// divides up screen into a grid and returns a unique index based on location
// of touch
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

// Read all the files in specified directory and return names in files parameter
void readFiles(File dir, char files[][13]) {
    int16_t ndx = 0;
    while(true) {     
    File entry =  dir.openNextFile();\
    if (! entry) {
       // no more files
       break;
    }
    char tmp[13];
    entry.getName(tmp,13);  // returns name of file
    // filter out folders, renames, hidden files, etc.,
    if (ndx < MAX_ICONS && tmp[0] != '_' && tmp[0] != '.' && strlen(tmp) > 1) {
      strncpy(files[ndx], tmp, 13);
      ndx++;
    }
  }
}



