/*
 *  Author: Claudiu Matei
 */

#ifndef WiFi101NetworkAdapter_h
#define WiFi101NetworkAdapter_h

#include <IProtocolHandler.h>
#include <INetworkAdapter.h>
#include <SPI.h>
#include <WiFi101.h>

class WiFi101NetworkAdapter : public INetworkAdapter {
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

WiFi101NetworkAdapter::WiFi101NetworkAdapter(String ipAddress, String ssid, String password) {
	uint8_t ipAddressBuf[4];
	parseBytes(ipAddress.c_str(), '.', ipAddressBuf, 4, 10);

	WiFi.config(ipAddressBuf);
	WiFi.begin(ssid, password);
}

void WiFi101NetworkAdapter::setup() {
	this->server = new WiFiServer(protocolHandler->port);
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
