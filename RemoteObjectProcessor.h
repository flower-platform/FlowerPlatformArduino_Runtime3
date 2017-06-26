/**
 * @author Claudiu Matei
 *
 */

#ifndef REMOTEOBJECTPROCESSOR_H_
#define REMOTEOBJECTPROCESSOR_H_

#include <FlowerPlatformArduinoRuntime.h>
#include <RemoteObject.h>
#include <RemoteObjectProtocol.h>

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
	if (cmd < 0) {
		return false;
	}
	size_t size = 0;
	char rbuf[RECV_BUFFER_SIZE];

	// read target node id
	size = in->readBytesUntil(TERM, rbuf, RECV_BUFFER_SIZE); // target node id
	if (nodeIdPSTR != NULL && strncmp_P(rbuf, nodeIdPSTR, size) != 0) { // not our packet
		return false;
	}

	switch (cmd) {
	case 'I': // INVOKE
		size = in->readBytesUntil(TERM, rbuf, RECV_BUFFER_SIZE); // callbackId
		char callbackIdStr[size + 1];
		strncpy(callbackIdStr, rbuf, size);
		callbackIdStr[size] = TERM;
		size = in->readBytesUntil(EOT, rbuf, RECV_BUFFER_SIZE); // function call
		rbuf[size] = TERM;

		//buffer return value
		BufferedPrint<64> tbuf(out);
//		tbuf.print("asdasd");
		dispatchFunctionCall(rbuf, &tbuf);

		// send response packet
		sendPacketHeader(out, 'R', securityTokenPSTR, tbuf.getSize());
		out->print(callbackIdStr); out->print(TERM); // callbackId
		tbuf.flush();
		endPacket(out);
		break;
	}
	return true;
}

#endif /* REMOTEOBJECTPROCESSOR_H_ */
