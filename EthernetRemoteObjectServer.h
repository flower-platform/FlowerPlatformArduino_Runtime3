/**
 * @author Claudiu Matei
 */

#ifndef ETHERNETREMOTEOBJECTSERVER_H_
#define ETHERNETREMOTEOBJECTSERVER_H_

#include <FlowerPlatformArduinoRuntime.h>
#include <RemoteObjectProcessor.h>
#include <RemoteObjectProtocol.h>

class EthernetRemoteObjectServer : public RemoteObjectProcessor {
public:

	EthernetRemoteObjectServer(const char* macAddress, const char* ipAddress, int port, const char* securityTokenPSTR);

	virtual ~EthernetRemoteObjectServer() { }

	virtual void sendPacketHeader(Print* out, char command, const char* securityTokenPSTR, size_t payloadSize);

	virtual void endPacket(Print* out);

	void loop();

protected:

	EthernetServer* server = NULL;

};

EthernetRemoteObjectServer::EthernetRemoteObjectServer(const char* macAddress, const char* ipAddress, int port, const char* securityTokenPSTR) : RemoteObjectProcessor(securityTokenPSTR) {
	uint8_t  macAddressBuf[6];
	parseBytes(macAddress, ':', macAddressBuf, 6, 16);

	uint8_t ipAddressBuf[4];
	parseBytes(ipAddress, '.', ipAddressBuf, 4, 10);

	Ethernet.begin(macAddressBuf, ipAddressBuf);

	server = new EthernetServer(port);
}

void EthernetRemoteObjectServer::sendPacketHeader(Print* out, char command, const char* securityTokenPSTR, size_t payloadSize) {
	// send HTTP header
	write_P(out, PSTR("HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\n")); // HTTP headers
	write_P(out, PSTR("Content-Length: ")); out->println(FPRP_FIXED_PACKET_SIZE + payloadSize);
	out->println();

	// send packet header
	fprp_startPacket(out, command, securityTokenPSTR); // command = RESULT
}

void EthernetRemoteObjectServer::endPacket(Print* out) {
	fprp_endPacket(out);
}

void EthernetRemoteObjectServer::loop() {
	EthernetClient client = server->available();
	if (!client) {
		return;
	}
	Serial.println("client connected");

	// skip http headers
	if (!client.find((char*) "\r\n\r\n")) {
		// http headers not found or incomplete
		client.stop();
		return;
	}

	if (!processCommand(&client, &client)) {
		client.stop();
		return;
	}

	client.flush();
	delay(100);
	client.stop();
}

#endif /* ETHERNETREMOTEOBJECTSERVER_H_ */
