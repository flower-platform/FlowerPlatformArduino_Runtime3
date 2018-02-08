/*
 * SPIRemoteObjectProcessor.h
 *
 *  Created on: Dec 21, 2017
 *      Author: Flower
 */

#ifndef SPIREMOTEOBJECTPROCESSOR_H_
#define SPIREMOTEOBJECTPROCESSOR_H_

#include <Arduino.h>
#include <RemoteObjectProcessor.h>
#include <RemoteObjectProtocol.h>
#include <SmartBuffer.h>
#include <SPI.h>

#define ENQ '\5'
#define NAK '\21'

char spiBufferArray[128]; SmartBuffer spiBuffer(spiBufferArray, 128);
char spiResponseBufferArray[128]; SmartBuffer spiResponseBuffer(spiResponseBufferArray);
volatile bool processSpiCommand = false;

// SPI interrupt routine
ISR (SPI_STC_vect) {
	uint8_t c = SPDR;  // grab byte from SPI Data Register
	if (processSpiCommand) {
		SPDR = NAK;
	} else if (c == ENQ) {
		if (spiResponseBuffer.available()) {
			c = spiResponseBuffer.read();
			SPDR = c;
		} else {
			SPDR = NAK;
		}
	} else {
		SPDR = NAK;
		spiBuffer.write(c);
		if (c == EOT) {
			processSpiCommand = true;
		}
	}
}

class SPIRemoteObjectProcessor : public RemoteObjectProcessor {

public:

	SPIRemoteObjectProcessor(const char* securityTokenPSTR, const char* nodeIdPSTR = NULL) : RemoteObjectProcessor(securityTokenPSTR, nodeIdPSTR) {
		SPCR |= bit(SPE); // enable SPI slave
		pinMode(MISO, OUTPUT); // have to send on master in, *slave out*
		SPI.attachInterrupt();
	}

	bool loop() {
		if (processSpiCommand) {
//			Serial.println("\nloop Processing");
			spiResponseBuffer.clear();
			bool res = processCommand(&spiBuffer, &spiResponseBuffer);
			spiBuffer.clear();
			processSpiCommand = false;
			return res;
		}
		return false;
	}

};


#endif /* SPIREMOTEOBJECTPROCESSOR_H_ */
