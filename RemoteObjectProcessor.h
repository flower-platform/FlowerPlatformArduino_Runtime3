/**
 * @author Claudiu Matei
 *
 */

#ifndef REMOTEOBJECTPROCESSOR_H_
#define REMOTEOBJECTPROCESSOR_H_

#include <Arduino.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>
#include <RemoteObject.h>
#include <RemoteObjectProtocol.h>
#include <SmartBuffer.h>


#ifdef ESP8266
#define PACKET_BUFFER_SIZE 6210
#define RESULT_BUFFER_SIZE 2048
#else
#define PACKET_BUFFER_SIZE 128
#define RESULT_BUFFER_SIZE 64
#endif


class RemoteObjectProcessor {
public:

	RemoteObjectProcessor(const char* securityTokenPSTR, const char* nodeIdPSTR = NULL) {
		this->securityTokenPSTR = securityTokenPSTR;
		this->nodeIdPSTR = nodeIdPSTR;
	}

	virtual ~RemoteObjectProcessor() { }

	bool processCommand(Stream* in, Print* out);

	virtual void sendPacketHeader(Print* out, char command, const char* securityTokenPSTR, size_t payloadSize);

	virtual void endPacket(Print* out);

	virtual bool loop() = 0;

protected:

	const char* securityTokenPSTR;

	const char* nodeIdPSTR;

};

void RemoteObjectProcessor::sendPacketHeader(Print* out, char command, const char* securityTokenPSTR, size_t) {
	// default implementateion
	fprp_startPacket(out, command, securityTokenPSTR);
}

void RemoteObjectProcessor::endPacket(Print* out) {
	// default implementateion
	fprp_endPacket(out);
}


bool RemoteObjectProcessor::processCommand(Stream* in, Print* out) {
	int cmd = fprp_readCommand(in, securityTokenPSTR);
	if (cmd < 0 || cmd != 'I') {
		return false;
	}
	size_t size = 0;
	char *buf = new char[PACKET_BUFFER_SIZE];

	// read target node id
	size = in->readBytesUntil(TERM, buf, PACKET_BUFFER_SIZE); buf[size] = '\0'; // target node id (rappInstanceId)
	if (nodeIdPSTR != NULL && strcmp_P(buf, nodeIdPSTR) != 0) { // not our packet
		return false;
	}

	size = in->readBytesUntil(TERM, buf, PACKET_BUFFER_SIZE); // callbackId
	char callbackIdStr[size + 1];
	strncpy(callbackIdStr, buf, size);
	callbackIdStr[size] = TERM;
	size = in->readBytesUntil(EOT, buf, PACKET_BUFFER_SIZE); // function call
	buf[size] = TERM;

	//buffer return value
	uint8_t *resultBuf = new uint8_t[RESULT_BUFFER_SIZE];
	SmartBuffer resultPrint(resultBuf, RESULT_BUFFER_SIZE);
	bool ok = dispatchFunctionCall(buf, &resultPrint);

	SmartBuffer responseBuf(out, (uint8_t*) buf, PACKET_BUFFER_SIZE);
	if (ok) {
		// send result
		sendPacketHeader(&responseBuf, 'R', securityTokenPSTR, strlen(callbackIdStr) + 1 + resultPrint.available());
		responseBuf.print(callbackIdStr); responseBuf.print(TERM); // callbackId
		resultPrint.flush(&responseBuf);
		endPacket(&responseBuf);
		responseBuf.flush();
	} else {
		//send error
		sendPacketHeader(out, 'X', securityTokenPSTR, 0);
		endPacket(out);
	}

	delete resultBuf;
	delete buf;

	return true;
}

#endif /* REMOTEOBJECTPROCESSOR_H_ */
