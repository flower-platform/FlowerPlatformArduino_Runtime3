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

	/*
	 * @flower { constructorVariant="Default" }
	 */
	WiFi101NetworkAdapter(String ipAddress, String ssid, String password);

	void setup();

	void loop();

};

WiFi101NetworkAdapter::WiFi101NetworkAdapter(String ipAddress, String ssid, String password) : IWiFiNetworkAdapter(ipAddress, ssid, password) { };

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

		// wait until data is received in buffer (max. 1s)
		unsigned long deadline = millis() + 1000;
		while (client.available() == 0 && millis() < deadline);

		protocolHandler->processClientRequest(&client);
	}
}

#endif
