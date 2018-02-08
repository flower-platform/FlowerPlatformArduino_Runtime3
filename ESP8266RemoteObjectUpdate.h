#ifndef ESP8266REMOTEOBJECTUPDATE_H_
#define ESP8266REMOTEOBJECTUPDATE_H_

#include <FlowerPlatformArduinoRuntime.h>
#include <Updater.h>

char _update_md5[32];

bool _update_init(int totalSize, const char* md5) {
	if (Update.isRunning()) {
		Update.end();
	}
	strncpy(_update_md5, md5, 32);
	bool res = Update.setMD5(_update_md5);
	res &= Update.begin(totalSize);
	Update.printError(Serial);
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

bool _esp8266_remote_object_update_dispatchFunctionCall(char* functionCall, Print* response) {
	if (strcmp_P(functionCall, PSTR("_update_init")) == 0) {
		functionCall += strlen(functionCall) + 1;
		int totalSize = atoi(functionCall); functionCall += strlen(functionCall) + 1;
		Serial.println(totalSize);
		char* md5 = (functionCall); functionCall += strlen(functionCall) + 1;
		Serial.println(md5);
		response->print(_update_init(totalSize, md5));
	} else if (strcmp_P(functionCall, PSTR("_update_write")) == 0) {
		functionCall += strlen(functionCall) + 1;
		char* data = (functionCall); functionCall += strlen(functionCall) + 1;
		base64_decode(data, data, functionCall - data -1);
		int chunkSize = atoi(functionCall); functionCall += strlen(functionCall) + 1;
		response->print(_update_write((uint8_t*) data, chunkSize));
	} else {
		return false;
	}
	return true;
}

#endif /* ESP8266REMOTEOBJECTUPDATE_H_ */
