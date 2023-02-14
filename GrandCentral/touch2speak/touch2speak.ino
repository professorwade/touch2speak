
#define DEBUG 1


/*************************************************** 
  This is an example for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout 
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/
// include SPI, MP3 and SD libraries
//#include <SPI.h>
//#include <Adafruit_VS1053.h>
//#include <SD.h>
#include <BufferedPrint.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <RingBuf.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <sdios.h>

// touchscreen includes
#include <Adafruit_GFX.h>
//#include <gfxfont.h>
#include <TouchScreen.h>
#include "Adafruit_HX8357.h"
//#include <Adafruit_TFTLCD.h>
//#include <pin_magic.h>
//#include <registers.h>
//#include <pins_arduino.h>
#include <Adafruit_ImageReader.h>

/*
#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif
*/
//#define DEBUG

/* -----------------  Touch Screen Initialization ----------------------------*/
// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).
//   D0 connects to digital pin 22
//   D1 connects to digital pin 23
//   D2 connects to digital pin 24
//   D3 connects to digital pin 25
//   D4 connects to digital pin 26
//   D5 connects to digital pin 27
//   D6 connects to digital pin 28
//   D7 connects to digital pin 29

#define YP A12  // must be an analog pin, use "An" notation!
#define XM A13  // must be an analog pin, use "An" notation! 
#define YM 32   // can be a digital pin
#define XP 33   // can be a digital pin          
#define TS_MINX 170   // 150
#define TS_MINY 105   // 120
#define TS_MAXX 870   // 920
#define TS_MAXY 920   // 940

// aligns cursor with touch point
#define X_FUDGE 115
#define Y_FUDGE 120

// define touch pressure range
#define MINPRESSURE 10
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
#define TFT_RST 8 // RST can be set to -1 if you tie it to Arduino's reset

// Parallel Access Lines
/*
#define LCD_CS A11 // Chip Select goes to Analog 3
#define LCD_CD A10 // Command/Data goes to Analog 2
#define LCD_WR A9 // LCD Write goes to Analog 1
#define LCD_RD A8 // LCD Read goes to Analog 0
// optional
#define LCD_RESET A14
*/

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


//#define BOXSIZE 80
//#define BUFFPIXEL 20

void readFiles(File dir, char files[][13], int16_t size);

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

/*
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
*/

/* ------------------- MP3 Shield ---------------------------- */
/*
// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)
*/

/*
// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin
*/

/*
Adafruit_VS1053_FilePlayer musicPlayer = 
  // create breakout-example object!
  //Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
  // create shield-example object!
  Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);
*/

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
  ImageReturnCode stat;
  //reader.drawBMP("/arduino.bmp", tft, j , i);  

  
  // stat = reader.drawBMP("/arduino.bmp", tft, 0, 0);
  // reader.printStatus(stat);

  // int32_t width, height;
  // stat = reader.bmpDimensions("/arduino.bmp", &width, &height);
  // reader.printStatus(stat);
  // Serial.println(width);
  // Serial.println(height); 
  
  /* Draw Grid */
  readFiles(SD.open("/ICONS/"), files);

  int16_t ndx = 0;
  for (int16_t i = 0; i < tft.width(); i += 80) {
    for (int16_t j = 0; j < tft.height(); j += 80) {
        char fullpath[25] = "/ICONS/\0";
        strncat(fullpath, files[ndx], 13);
        reader.drawBMP(fullpath, tft, i, j);
        
        #if defined(DEBUG)
        Serial.println(fullpath);
        #endif    
        ndx++;
    }
  }
  Serial.println("Symbols Loaded - Unit Ready");  
}
/*
  // initialise the music player
  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  Serial.println(F("VS1053 found"));
  //musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
 
  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  Serial.println("SD OK!");
  
  // list files
  //printDirectory(SD.open("/sounds"), 0);
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(10,10);
*/
  /***** Two interrupt options! *******/ 
  // This option uses timer0, this means timer1 & t2 are not required
  // (so you can use 'em for Servos, etc) BUT millis() can lose time
  // since we're hitchhiking on top of the millis() tracker
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT);
  
  // This option uses a pin interrupt. No timers required! But DREQ
  // must be on an interrupt pin. For Uno/Duemilanove/Diecimilla
  // that's Digital #2 or #3
  // See http://arduino.cc/en/Reference/attachInterrupt for other pins
  // *** This method is preferred

  
  //if (!musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT))
  //  Serial.println(F("DREQ pin is not an interrupt pin"));
  
  /* ---------------------- Touchscreen Initialization ---------------------*/
  //tft.reset();
  
  //uint16_t identifier = tft.readID();
/*
  #if defined(DEBUG)
  if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Adafruit_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    return;
  }
  #endif

  tft.begin(identifier);

  
  tft.fillScreen(BLACK);

}
*/

void loop() {  

  //Serial.println("Touchscreen Assistive Device Prototype");
  TSPoint p = ts.getPoint();
  p.x = TS_MAXX - p.x + X_FUDGE;
  p.y = TS_MAXY - p.y + Y_FUDGE;

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {

    // scale from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);

    int16_t zone = getGrid(p.x,  p.y);
    char filename[25] = "/SOUNDS/\0";
    strncat(filename, files[zone], 13);
    char* ndx = strchr(filename,'.');
    strncpy(ndx, ".OGG", 4); 

    #if defined(DEBUG)
    Serial.print("Coord: ");
    Serial.print(p.x);
    Serial.print(",");
    Serial.println(p.y);
    Serial.print("Zone:");
    Serial.println(zone);
    Serial.print("Sound File: ");
    Serial.println(filename);
    #endif

    //musicPlayer.playFullFile(filename); 
  }

  /*

  // Alternately, we can just play an entire file at once
  // This doesn't happen in the background, instead, the entire
  // file is played and the program will continue when it's done!
  //musicPlayer.playFullFile("track001.ogg");

  // Start playing a file, then we can do stuff while waiting for it to finish
  if (! musicPlayer.startPlayingFile("/jf.mp3")) {
    Serial.println("Could not open file jf.mp3");
    while (1);
  }

  while (musicPlayer.playingMusic) {
    // file is now playing in the 'background' so now's a good time
    // to do something else like handling LEDs or buttons :)
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Done playing music");

  */

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
       //Serial.println("**nomorefiles**");
       break;
    }
    char tmp[14];
    entry.getName(tmp,13);
    if (ndx < MAX_ICONS && tmp[0] != '_') {
      strncpy(files[ndx], tmp, 13);
      Serial.println(files[ndx]);

      ndx++;
    }
  }
}


/// File listing helper
/*
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}
*/
/*
void bmpDraw(char *filename, int x, int y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
  uint16_t lcdbuffer[BUFFPIXEL];  // pixel out buffer (16-bit per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();
  uint8_t  lcdidx = 0;
  boolean  first = true;

  if((x >= tft.width()) || (y >= tft.height())) return;

  #if defined(DEBUG)
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  #endif
  
  // Open requested file on SD card

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.println(F("File not found"));
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    int32_t tmp32 = read32(bmpFile);
    
    
    (void)read32(bmpFile); // Read & ignore creator bytes
    
    bmpImageoffset = read32(bmpFile); // Start of image data
    tmp32 = read32(bmpFile);

    #if defined(DEBUG)
    Serial.println(F("File size: ")); 
    Serial.println(tmp32);
    Serial.print(F("Image Offset: ")); 
    Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: "));
    Serial.println(tmp32);
    #endif
    
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel      
      uint32_t tmp = read32(bmpFile);
      
      #if defined(DEBUG)
      Serial.print(F("Bit Depth: ")); 
      Serial.println(bmpDepth);
      Serial.print("BMP Depth: ");
      Serial.println(bmpDepth);
      Serial.print("next read: ");
      Serial.println(tmp);
      #endif

      if((bmpDepth == 24) && (tmp == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
      
        #if defined(DEBUG)
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);
        #endif

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...
          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each column...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              // Push LCD buffer to the display first
              if(lcdidx > 0) {
                tft.pushColors(lcdbuffer, lcdidx, first);
                lcdidx = 0;
                first  = false;
              }
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            lcdbuffer[lcdidx++] = tft.color565(r,g,b);
          } // end pixel
        } // end scanline
        // Write any remaining data to LCD
        if(lcdidx > 0) {
          tft.pushColors(lcdbuffer, lcdidx, first);
        }

        #if defined(DEBUG) 
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");
        #endif

      } // end goodBmp

    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println(F("BMP format not recognized."));
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
*/

