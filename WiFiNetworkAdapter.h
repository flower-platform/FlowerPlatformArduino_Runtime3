/*
 *  Author: Claudiu Matei
 */

#ifndef WiFiNetworkAdapter_h
#define WiFiNetworkAdapter_h

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>



class WiFiNetworkAdapter : public IWiFiNetworkAdapter {
protected:

	WiFiServer* server = NULL;

public:

	void setup();

	void loop();

};

void WiFiNetworkAdapter::setup() {
	INetworkAdapter::setup();

	// Disable SPI for SD card.
	// This workaround is needed for Ethernet shield clones. The original Ethernet shield should work properly without this, but the clones don't.
	pinMode(4, OUTPUT);
	pinMode(4, HIGH);

	this->server = new WiFiServer(protocolHandler->port);
	WiFi.begin(ssid, password);
	WiFi.config(ipAddress);
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
