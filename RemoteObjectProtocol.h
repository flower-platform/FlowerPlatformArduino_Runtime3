/**
 * @author Claudiu Matei
 *
 */

#include <FlowerPlatformArduinoRuntime.h>

#ifndef REMOTEOBJECTPROTOCOL_H_
#define REMOTEOBJECTPROTOCOL_H_

#define FPRP_VERSION '1'
#define TERM '\0'
#define EOT '\4'
#define FPRP_HEADER "FPRP"
#define FPRP_FIXED_PACKET_SIZE 19

int fprp_readCommand(Stream* in, const char* securityTokenPSTR) {
	size_t maxSize = strlen_P(securityTokenPSTR) + 4;
	char buf[maxSize];
	size_t size = 0;

	size = in->readBytesUntil('\0', buf, maxSize); buf[size] = '\0'; // FP header
	if (strcmp_P(buf, PSTR(FPRP_HEADER)) != 0) {
		// error: invalid protocol header
		return -11;
	}

	size = in->readBytesUntil('\0', buf, maxSize); // protocol version
	if (size != 1 || *buf != FPRP_VERSION) {
		// error: unsupported protocol version
		return -12;
	}

	size = in->readBytesUntil('\0', buf, maxSize); buf[size] = '\0'; // security token
	if (strcmp_P(buf, securityTokenPSTR) != 0) {
		// error: invalid security token
		return -13;
	}

	size = in->readBytesUntil('\0', buf, maxSize); // command
	if (size != 1) {
		// error: invalid command field
		return -14;
	}

	char cmd = *buf;
//	Serial.print("READ CMD: "); Serial.println(cmd);
	return cmd;
}

void fprp_startPacket(Print* out, char cmd, const char* securityTokenPSTR) {
//	Serial.print("START CMD: "); Serial.println(cmd);
	out->print(FPRP_HEADER); out->print(TERM); // FPRP header (4 + 1 bytes)
	out->print(FPRP_VERSION); out->print(TERM); // protocol version (1 + 1 bytes)
	write_P(out, securityTokenPSTR); out->print(TERM); // security token (8 + 1 bytes)
	out->print(cmd); out->print(TERM); // command (1 + 1 bytes)
}

inline void fprp_endPacket(Print* out) {
	out->write('\x04');
}

#endif /* REMOTEOBJECTPROTOCOL_H_ */
