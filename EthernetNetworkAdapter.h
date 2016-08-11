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
	EthernetServer* server = NULL;

public:
	/*
	 * @flower { constructorVariant="Default" }
	 */
	EthernetNetworkAdapter(String ipAddress, String macAddress);

	void setup();

	void loop();

};

EthernetNetworkAdapter::EthernetNetworkAdapter(String ipAddress, String macAddress) {
	// Disable SPI for SD card.
	// This workaround is needed for Ethernet shield clones. The original Ethernet shield should work properly without this, but the clones don't.
	pinMode(4, OUTPUT);
	pinMode(4, HIGH);

	uint8_t  macAddressBuf[6];
	parseBytes(macAddress.c_str(), ':', macAddressBuf, 6, 16);

	uint8_t ipAddressBuf[4];
	parseBytes(ipAddress.c_str(), '.', ipAddressBuf, 4, 10);

	Ethernet.begin(macAddressBuf, ipAddressBuf);
};

void EthernetNetworkAdapter::setup() {
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
