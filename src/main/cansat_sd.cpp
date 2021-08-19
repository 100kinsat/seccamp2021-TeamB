// Reference:
// https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
// https://github.com/ny-a/100kinsat-data-logger/blob/main/src/fast-calibrate/cansat_sd.cpp
#include "cansat_sd.hpp"

/**
 * @brief SDカードの初期化処理
 *
 */
CanSatSd::CanSatSd() {
  // Serial.begin(115200);
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

/**
 * @brief ファイルを読み込む
 *
 * @param fs
 * @param path
 */
void CanSatSd::readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

int CanSatSd::readFileInt(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return 0;
  }

  Serial.print("Read from file: ");
  char buffer[16];
  int i = 0;
  while (file.available()) {
    char next_character = file.read();
    if (next_character != '\n') {
      buffer[i] = next_character;
      i++;
    }
  }
  file.close();

  std::string buffer_string(buffer, i);

  if (i == 0) {
    return 0;
  } else {
    return std::stoi(buffer_string);
  }
}

/**
 * @brief ファイルに書き込む（上書き）
 *
 * @param fs
 * @param path
 * @param message
 */
void CanSatSd::writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void CanSatSd::writeFileInt(fs::FS &fs, const char *path, int number) {
  Serial.printf("Writing file: %s\n", path);

  std::string s = std::to_string(number);
  const char *message = s.c_str();

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

/**
 * @brief ファイルに書き込む（追記）
 *
 * @param fs
 * @param path
 * @param message
 */
void CanSatSd::appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void CanSatSd::appendFileString(fs::FS &fs, const char *path, String &message) {
  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    // suppress message
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

/**
 * @brief ファイルを削除する
 *
 * @param fs
 * @param path
 */
void CanSatSd::deleteFile(fs::FS &fs, const char *path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void CanSatSd::listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

bool CanSatSd::existDir(fs::FS &fs, const char * dirname){
  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return false;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return false;
  }

  return true;
}

bool CanSatSd::existFile(fs::FS &fs, const char * filename){
  File root = fs.open(filename);
  if(!root){
    Serial.println("Failed to open directory");
    return false;
  }
  if(root.isDirectory()){
    Serial.println("Is a directory");
    return false;
  }

  return true;
}

void CanSatSd::createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void CanSatSd::removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void CanSatSd::renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void CanSatSd::testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

void CanSatSd::check(fs::FS &fs) {
  listDir(fs, "/", 0);
  createDir(fs, "/mydir");
  listDir(fs, "/", 0);
  removeDir(fs, "/mydir");
  listDir(fs, "/", 2);
  writeFile(fs, "/hello.txt", "Hello ");
  appendFile(fs, "/hello.txt", "World!\n");
  readFile(fs, "/hello.txt");
  deleteFile(fs, "/foo.txt");
  renameFile(fs, "/hello.txt", "/foo.txt");
  readFile(fs, "/foo.txt");
  testFileIO(fs, "/test.txt");
  // Serial.printf("Total space: %lluMB\n", fs.totalBytes() / (1024 * 1024));
  // Serial.printf("Used space: %lluMB\n", fs.usedBytes() / (1024 * 1024));
}