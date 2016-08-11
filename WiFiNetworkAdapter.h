/*
 *  Author: Claudiu Matei
 */

#ifndef WiFiNetworkAdapter_h
#define WiFiNetworkAdapter_h

#include <Arduino.h>
#include <INetworkAdapter.h>
#include <IProtocolHandler.h>
#include <stddef.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>



class WiFiNetworkAdapter : public INetworkAdapter {
protected:

	WiFiServer* server = NULL;

public:

	/*
	 * @flower { constructorVariant="Default" }
	 */
	WiFiNetworkAdapter(String ipAddress, String ssid, String password);

	void setup();

	void loop();

};

WiFiNetworkAdapter::WiFiNetworkAdapter(String ipAddress, String ssid, String password) {
	// Disable SPI for SD card.
	// This workaround is needed for Ethernet shield clones. The original Ethernet shield should work properly without this, but the clones don't.
	pinMode(4, OUTPUT);
	pinMode(4, HIGH);

	uint8_t ipAddressBuf[4];
	parseBytes(ipAddress.c_str(), '.', ipAddressBuf, 4, 10);

	WiFi.config(ipAddressBuf);
	WiFi.begin(ssid, password);
}

void WiFiNetworkAdapter::setup() {
	this->server = new WiFiServer(protocolHandler->port);
	this->server->begin();
}

void WiFiNetworkAdapter::loop() {
	// listen for incoming clients
	WiFiClient client = server->available();
	if (client) {
		server->begin();
		protocolHandler->processClientRequest(&client);
	}
}

#endif
