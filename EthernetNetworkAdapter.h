/*
 *  Author: Claudiu Matei
 */

#ifndef EthernetNetworkAdapter_h
#define EthernetNetworkAdapter_h

#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <INetworkAdapter.h>


class EthernetNetworkAdapter : public INetworkAdapter {
protected:
	EthernetServer* server;

public:
	uint8_t macAddress[6];

	// TOCO CM: temporary; we will use setters for ip and mac
	const char* macAddressStr = NULL;

	void setup();

	void loop();

};

void EthernetNetworkAdapter::setup() {
	INetworkAdapter::setup();
	if (macAddressStr != NULL) {
		INetworkAdapter::parseBytes(macAddressStr, ':', macAddress, 6, 16);
	}

	// Disable SPI for SD card.
	// This workaround is needed for Ethernet shield clones. The original Ethernet shield should work properly without this, but the clones don't.
	pinMode(4, OUTPUT);
	pinMode(4, HIGH);

	Ethernet.begin(macAddress, ipAddress);
	this->server = new EthernetServer(protocolHandler->port);
}

void EthernetNetworkAdapter::loop() {
	// listen for incoming clients
	EthernetClient client = server->available();
	if (client) {
		protocolHandler->processClientRequest(&client);
	}
}

#endif
