#ifndef ESP8266REMOTEOBJECTUPDATE_H_
#define ESP8266REMOTEOBJECTUPDATE_H_

#include <Esp.h>
#include <HardwareSerial.h>
#include <Updater.h>
#include <cstdint>
#include <cstring>

char _update_md5[32];

bool _update_init(int totalSize, const char* md5) {
	if (Update.isRunning()) {
		Update.end();
	}
	strncpy(_update_md5, md5, 32);
	bool res = Update.setMD5(_update_md5);
	res &= Update.begin(totalSize);
	return res;
}

bool _update_write(uint8_t* chunk, int chunkSize) {
	Update.write(chunk, chunkSize);
	if (Update.isFinished() && Update.end()) {
		Update.printError(Serial);
		ESP.restart();
	} else {
		Update.printError(Serial);
	}
	return !Update.hasError();
}




#endif /* ESP8266REMOTEOBJECTUPDATE_H_ */
