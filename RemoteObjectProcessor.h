/**
 * @author Claudiu Matei
 *
 */

#ifndef REMOTEOBJECTPROCESSOR_H_
#define REMOTEOBJECTPROCESSOR_H_

#include <BufferedPrint.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <RemoteObject.h>
#include <RemoteObjectProtocol.h>
#include <Stream.h>

class RemoteObjectProcessor {
public:

	RemoteObjectProcessor(const char* securityTokenPSTR, const char* nodeIdPSTR = NULL) {
		this->securityTokenPSTR = securityTokenPSTR;
		this->nodeIdPSTR = nodeIdPSTR;
	}

	virtual ~RemoteObjectProcessor() { }

	bool processCommand(Stream* in, Print* out);

	virtual void sendPacketHeader(Print* out, char command, const char* securityTokenPSTR, size_t payloadSize) = 0;

	virtual void endPacket(Print* out) = 0;

	virtual void loop() = 0;

protected:

	const char* securityTokenPSTR;

	const char* nodeIdPSTR;

};

bool RemoteObjectProcessor::processCommand(Stream* in, Print* out) {
	int cmd = fprp_readCommand(in, securityTokenPSTR);
	if (cmd < 0 || cmd != 'I') {
		return false;
	}
	size_t size = 0;
	char *rbuf = new char[RECV_BUFFER_SIZE];

	// read target node id
	size = in->readBytesUntil(TERM, rbuf, RECV_BUFFER_SIZE); rbuf[size] = '\0'; // target node id (rappInstanceId)
	if (nodeIdPSTR != NULL && strcmp_P(rbuf, nodeIdPSTR) != 0) { // not our packet
		return false;
	}

	size = in->readBytesUntil(TERM, rbuf, RECV_BUFFER_SIZE); // callbackId
	char callbackIdStr[size + 1];
	strncpy(callbackIdStr, rbuf, size);
	callbackIdStr[size] = TERM;
	size = in->readBytesUntil(EOT, rbuf, RECV_BUFFER_SIZE); // function call
	rbuf[size] = TERM;

	//buffer return value
	uint8_t *resultBuf = new uint8_t[SEND_BUFFER_SIZE];
	BufferedPrint<0> resultPrint(out, resultBuf, SEND_BUFFER_SIZE);
	bool ok = dispatchFunctionCall(rbuf, &resultPrint);

	Serial.println("Sending response");
	if (ok) {
		// send result
		sendPacketHeader(out, 'R', securityTokenPSTR, strlen(callbackIdStr) + 1 + resultPrint.getSize());
		out->print(callbackIdStr); out->print(TERM); // callbackId
		resultPrint.flush();
		endPacket(out);
	} else {
		//send error
		sendPacketHeader(out, 'X', securityTokenPSTR, 0);
		endPacket(out);
	}

	delete resultBuf;
	delete rbuf;

	return true;
}

#endif /* REMOTEOBJECTPROCESSOR_H_ */
