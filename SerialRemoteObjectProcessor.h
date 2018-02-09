/**
 * @author Claudiu Matei
 *
 */

#ifndef SERIALREMOTEOBJECTPROCESSOR_H_
#define SERIALREMOTEOBJECTPROCESSOR_H_

#include <RemoteObjectProcessor.h>
#include <RemoteObjectProtocol.h>
#include <RS485Serial.h>

class SerialRemoteObjectProcessor : public RemoteObjectProcessor {
protected:

	RS485Serial* rs485;

public:

	SerialRemoteObjectProcessor(RS485Serial* rs485, const char* securityTokenPSTR, const char* nodeIdPSTR = NULL) : RemoteObjectProcessor(securityTokenPSTR, nodeIdPSTR) {
		this->rs485 = rs485;
	}

	void sendPacketHeader(Print* out, char command, const char* securityTokenPSTR, size_t payloadSize) {
		int size = FPRP_PACKET_OVERHEAD_SIZE + payloadSize;

//		delay(50);
		rs485->setBatchWriteMode(true);
		out->write((size >> 8) & 0xFF);
		out->write(size & 0xFF);

		// send packet header
		fprp_startPacket(out, command, securityTokenPSTR); // command = RESULT
	}

	void endPacket(Print* out) {
		fprp_endPacket(out);
		rs485->setBatchWriteMode(false);
	}

	bool loop() {
		if (rs485->available()) {
//			size_t frameSize = ((rs485->timedRead() & 0xFF) << 8) | (rs485->timedRead() & 0xFF);
			rs485->timedRead(); rs485->timedRead(); // ignore frame size
			return processCommand(rs485, rs485);
		}
		return false;
	}

};

#endif /* SERIALREMOTEOBJECTPROCESSOR_H_ */
