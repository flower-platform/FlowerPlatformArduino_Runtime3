/*
 * ESP8266ArduinoISP.h
 *
 *  Created on: Dec 13, 2017
 *      Author: Flower
 */

#ifndef ESP8266ARDUINOISP_H_
#define ESP8266ARDUINOISP_H_

#include <Arduino.h>
#include <Esp.h>
#include <flash_utils.h>
#include <HardwareSerial.h>
#include <pins_arduino.h>
#include <stdlib.h>
#include <SPI.h>
#include <cstdint>

//#define DEBUG
#ifdef DEBUG
#define dbg_print(X) Serial.print(X)
#define dbg_println(X) Serial.println(X)
#else
#define dbg_print(X)
#define dbg_println(X)
#endif

#define AVRISP_PTIME 10
extern "C" uint32_t _SPIFFS_start;
uint32_t _SPIFFS_PHYS_ADDR = (uint32_t) &_SPIFFS_start - 0x40200000;

class ESP8266ArduinoISP {
protected:
	int resetPin = D1;
	int resetActiveHigh = false;
	int spiFrequency = 300e3;

	bool initialized = false;

	char* currentHexRecord = NULL;
	int currentHexRecordIndex = -1;
	int currentHexRecordSize = -1;

	uint32_t lastSpiFlashAddress = -1;

	uint32_t chunkInfo[5][2]; // [chunkIndex][0] = address; [chunkIndex][1] = size
	int chunkIndex = -1;


	uint8_t spiTransaction(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
		SPI.transfer(a);
		SPI.transfer(b);
		SPI.transfer(c);
		return SPI.transfer(d);
	}

	uint32_t getHexValue(char *buf, short len) {
	  uint32_t v = 0;
	  while (len--) {
	    v = (v<<4) | (uint32_t)(*buf & 0xf);
	    if (*buf > '9') v += 9;
	    buf++;
	  }
	  return v;
	}

	bool syncAndInit() {
		dbg_println("\nsyncAndInit()");
		SPI.begin();
		SPI.setFrequency(spiFrequency);
		SPI.setHwCs(false);

		// try to sync the bus
		SPI.transfer(0x00);
		digitalWrite(resetPin, !resetActiveHigh);
		delayMicroseconds(50);
		digitalWrite(resetPin, resetActiveHigh);
		delay(30);

		int n = spiTransaction(0xAC, 0x53, 0x00, 0x00);
		if (n != 0) {
			return false;
		}

		// read signature
		uint8_t high = spiTransaction(0x30, 0x00, 0x00, 0x00);
		uint8_t middle = spiTransaction(0x30, 0x00, 0x01, 0x00);
		uint8_t low = spiTransaction(0x30, 0x00, 0x02, 0x00);

		dbg_print("SIGNATURE: high="); dbg_print(high); dbg_print(" middle="); dbg_print(middle); dbg_print(" low="); dbg_println(low);
		if (high != 0x1e || middle != 0x95 || low != 0x0f) {
			return false;
		}

		return true;
	}

	void ispChipErase() {
		dbg_print("Chip erase... ");
		spiTransaction(0xAC, 0x80, 0x00, 0x00);
		delay(AVRISP_PTIME);
		dbg_println("Done");
	}

	void ispCommit(int addr) {
		spiTransaction(0x4C, (addr >> 8) & 0xFF, addr & 0xFF, 0);
		delay(AVRISP_PTIME);
	}

	void ispFlashWrite(uint8_t hilo, int addr, uint8_t data) {
		spiTransaction(0x40 + 8 * hilo, (addr >> 8) & 0xFF, addr & 0xFF, data);
		delay(1);
	}

	uint8_t ispFlashRead(uint8_t hilo, int addr) {
		return spiTransaction(0x20 + hilo * 8, (addr >> 8) & 0xFF, addr & 0xFF, 0);
	}

	#define addr_to_page(x) (x & 0xFFFFE0) // 64 bytes (==32 words) page; address is in words => 32 bit mask
	void ispWriteBuffer(int address, uint8_t* data, int length) {
		int x = 0;
		int page = addr_to_page(address);
		while (x < length) {
			if (page != addr_to_page(address)) {
				dbg_print("committing page "); dbg_print(page); dbg_print(" "); dbg_println(x);
				ispCommit(page);
				page = addr_to_page(address);
			}
			ispFlashWrite(LOW, address, data[x++]);
			ispFlashWrite(HIGH, address, data[x++]);
			address++;
		}
		dbg_print("committing page "); dbg_print(page); dbg_print(" "); dbg_println(x);
		ispCommit(page);

		#ifdef DEBUG
		address = address - length/2;
		for (int i = 0; i < length / 2; i++) {
			dbg_print(2*i); dbg_print(" "); dbg_print(ispFlashRead(LOW, address + i) & 0xFF); dbg_print(" "); dbg_println(data[2*i] & 0xFF);
			dbg_print(2*i + 1); dbg_print(" "); dbg_print(ispFlashRead(HIGH, address + i) & 0xFF); dbg_print(" "); dbg_println(data[2*i + 1] & 0xFF);
		}
		#endif
	}

	bool processHexRecord() {
		if (*currentHexRecord != ':') {
			return false;
		}

		dbg_println(currentHexRecord);
		int byteCount = getHexValue(currentHexRecord + 1, 2);
		int address = getHexValue(currentHexRecord + 3, 4);
		int recordType = getHexValue(currentHexRecord + 7, 2);

		uint8_t computedChecksum = byteCount + ((address >> 8) & 0xFF) + (address & 0xFF) + recordType;

		dbg_print("byteCount="); dbg_print(byteCount); dbg_print(" address="); dbg_print(address); dbg_print(" recordType="); dbg_print(recordType); dbg_println();

		if (recordType == 0x00) { // data
			uint8_t buf[byteCount];
			for (int i = 0; i < byteCount; i++) {
				buf[i] = getHexValue(currentHexRecord + 9 + 2*i, 2);
				computedChecksum += buf[i];
			}

			computedChecksum = ~computedChecksum + 1;
			int checksum = getHexValue(currentHexRecord + 9 + 2 * byteCount, 2);
			dbg_print("checksum="); dbg_print(checksum); dbg_print(" computedChecksum="); dbg_println(computedChecksum);

			if (checksum != computedChecksum) {
				return false;
			}

			uint32_t flashAddress = _SPIFFS_PHYS_ADDR + address;
			dbg_print(lastSpiFlashAddress - _SPIFFS_PHYS_ADDR); dbg_print(" : "); dbg_println(flashAddress - _SPIFFS_PHYS_ADDR);

			if (flashAddress > lastSpiFlashAddress + 1) { // we have a new chunk (i.e. bootloader)
				dbg_println("NEW CHUNK; ");
				if (chunkIndex != -1) {
					chunkInfo[chunkIndex][1] = lastSpiFlashAddress - chunkInfo[chunkIndex][0] + 1; // compute last chunk's size
					dbg_print("old chunk size: "); dbg_println(chunkInfo[chunkIndex][1]);
				}
				chunkIndex++;
				chunkInfo[chunkIndex][0] = flashAddress; // save new chunk's start address
				dbg_print("new chunk's address: "); dbg_println(chunkInfo[chunkIndex][0]);
			}

			// http://www.espressif.com/sites/default/files/99a-sdk-espressif_iot_flash_rw_operation_en_v1.0_0.pdf

			if ((flashAddress & 0xFFFFF000) != (lastSpiFlashAddress & 0xFFFFF000)) {
				dbg_print("ERASE: "); dbg_println(flashAddress / FLASH_SECTOR_SIZE);
				ESP.flashEraseSector(flashAddress / FLASH_SECTOR_SIZE);
			}

			ESP.flashWrite(flashAddress, (uint32_t*) buf, byteCount);

			#ifdef DEBUG
			dbg_println("WRITE");
			for (int i = 0; i < byteCount; i++) {
				dbg_print(buf[i]); dbg_print(" ");
				buf[i] = 0xFF;
			}
			dbg_println();

			ESP.flashRead(flashAddress, (uint32_t*) buf, byteCount);
			dbg_println("READ");
			for (int i = 0; i < byteCount; i++) {
				dbg_print(buf[i]); dbg_print(" ");
			}
			dbg_println();
			#endif

			lastSpiFlashAddress = flashAddress + byteCount - 1;

		} else if (recordType == 0x01) { // end of file
			dbg_println("EOF");
			if (chunkIndex != -1) {
				chunkInfo[chunkIndex][1] = lastSpiFlashAddress - chunkInfo[chunkIndex][0] + 1; // compute last chunk's size
				dbg_print("old chunk size: "); dbg_println(chunkInfo[chunkIndex][1]);
			}

			ispChipErase();

			uint8_t buf[128];
			for (int i = 0; i <= chunkIndex; i++) {
				uint32_t spiFlashAddr = chunkInfo[i][0];
				uint32_t size = chunkInfo[i][1];
				dbg_print("Flashing "); dbg_print(size); dbg_print(" bytes from ");  dbg_println(spiFlashAddr);
				while (size > 0) {
					int k = size > 128 ? 128 : size;
					ESP.flashRead(spiFlashAddr, (uint32_t*) buf, k);
					yield();
					ispWriteBuffer((spiFlashAddr - _SPIFFS_PHYS_ADDR) / 2, buf, k); // we need word address for isp (byte address / 2)
					yield();
					spiFlashAddr += k;
					size -= k;
				}
			}
			stop();
		}

		return true;
	}


public:

	ESP8266ArduinoISP() {
		pinMode(resetPin, OUTPUT);
		digitalWrite(resetPin, !resetActiveHigh);
	}

	bool init() {
		dbg_print("RAM: "); dbg_println(freeRam());
		pinMode(resetPin, OUTPUT);
		digitalWrite(resetPin, !resetActiveHigh);
		if (currentHexRecord == NULL) {
			currentHexRecord = (char*) malloc(128);
		}
		currentHexRecordIndex = 0;
		lastSpiFlashAddress = -1;
		chunkIndex = -1;
		initialized = syncAndInit();
		return initialized;
	}

	bool processHexData(uint8_t* buf, int length) {
		while (length > 0) {
			if (*buf == '\n') {
				currentHexRecord[currentHexRecordIndex] = '\0';
				if (!processHexRecord()) {
					stop();
					return false;
				}
				yield();
				currentHexRecordIndex = 0;
			} else if (*buf != '\r') {
				currentHexRecord[currentHexRecordIndex++] = *buf;
			}
			length--;
			buf++;
		}
		return true;
	}

	void stop() {
		initialized = false;
		free(currentHexRecord);
		currentHexRecord = NULL;
		SPI.end();
		digitalWrite(resetPin, !resetActiveHigh);
	}

};

#undef DEBUG

#endif /* ESP8266ARDUINOISP_H_ */
