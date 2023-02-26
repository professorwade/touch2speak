/*
  Copyright (c) 2016 Arduino LLC. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef _SD_WAVE_FILE_INCLUDED
#define _SD_WAVE_FILE_INCLUDED

//#include <Arduino.h>
//#include <FreeStack.h>
//#include <MinimumSerial.h>
#include <SdFat.h>
#include <SdFatConfig.h>
//#include <sdios.h>
//#include <SD.h>

class SDWaveFile
{
public:
  SDWaveFile();
  SDWaveFile(SdFat* SD, const char *filename);
  //SDWaveFile(const String& filename);

  virtual ~SDWaveFile();

  operator bool();

  // from AudioIn
  long sampleRate();
  int bitsPerSample();
  int channels();

  // from SoundFile
  long frames();
  long duration();
  long currentTime();
  int cue(long time);
  int begin();
  int read(void* buffer, size_t size);
  int reset();
  void end();

private:
  void readHeader();

private:
  bool _headerRead;
  bool _isValid;
  bool _isPlaying;
  File _file;
  String _filename;
  SdFat* SD;
  long _sampleRate;
  int _bitsPerSample;
  int _channels;
  long _frames;
  int _blockAlign;
  uint32_t _dataOffset;
};

#endif
