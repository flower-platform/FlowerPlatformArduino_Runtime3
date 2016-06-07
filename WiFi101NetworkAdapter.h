/*
 *  Author: Claudiu Matei
 */

#ifndef WiFi101NetworkAdapter_h
#define WiFi101NetworkAdapter_h

#include <IProtocolHandler.h>
#include <IWiFiNetworkAdapter.h>
#include <SPI.h>
#include <WiFi101.h>

class WiFi101NetworkAdapter : public IWiFiNetworkAdapter {
protected:

	WiFiServer* server = NULL;

public:

	void setup();

	void loop();

};

void WiFi101NetworkAdapter::setup() {
	INetworkAdapter::setup();

	this->server = new WiFiServer(protocolHandler->port);
	WiFi.config(ipAddress);
	WiFi.begin(ssid, password);
	this->server->begin();
}

void WiFi101NetworkAdapter::loop() {
	// listen for incoming clients
	WiFiClient client = server->available();
	if (client) {
		server->begin();
		protocolHandler->processClientRequest(&client);
	}
}

#endif
