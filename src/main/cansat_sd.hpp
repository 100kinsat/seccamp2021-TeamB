// Reference:
// https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
// https://github.com/ny-a/100kinsat-data-logger/blob/main/src/fast-calibrate/cansat_sd.hpp
#ifndef __CANSAT_SD_H__
#define __CANSAT_SD_H__

/*
 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32
 *    D2       -
 *    D3       SS
 *    CMD      MOSI
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      SCK
 *    VSS      GND
 *    D0       MISO
 *    D1       -
 */

#include "Arduino.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <string>

class CanSatSd {
 public:
  CanSatSd();

  void readFile(fs::FS &fs, const char *path);
  int readFileInt(fs::FS &fs, const char *path);
  void writeFile(fs::FS &fs, const char *path, const char *message);
  void writeFileInt(fs::FS &fs, const char *path, int number);
  void appendFile(fs::FS &fs, const char *path, const char *message);
  void appendFileString(fs::FS &fs, const char *path, String &message);
  void deleteFile(fs::FS &fs, const char *path);

  void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
  bool existDir(fs::FS &fs, const char * dirname);
  bool existFile(fs::FS &fs, const char * filename);
  void createDir(fs::FS &fs, const char *path);
  void removeDir(fs::FS &fs, const char *path);
  void renameFile(fs::FS &fs, const char * path1, const char * path2);
  void testFileIO(fs::FS &fs, const char *path);
  void check(fs::FS &fs);
};

#endif  // __CANSAT_SD_H__