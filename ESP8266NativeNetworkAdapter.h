/*
 *  Author: Claudiu Matei
 */

#ifndef ESP8266NativeNetworkAdapter_h
#define ESP8266NativeNetworkAdapter_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <IPAddress.h>
#include <IProtocolHandler.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

class ESP8266NativeNetworkAdapter : public INetworkAdapter {
protected:

	WiFiServer* server = NULL;

public:

	/*
	 * @flower { constructorVariant="Default" }
	 */
	ESP8266NativeNetworkAdapter(String ipAddress, String ssid, String password);

	void setup();

	void loop();

};

ESP8266NativeNetworkAdapter::ESP8266NativeNetworkAdapter(String ipAddress, String ssid, String password) {
	WiFi.begin(ssid, password);

	uint8_t ipAddressBuf[4];
	parseBytes(ipAddress.c_str(), '.', ipAddressBuf, 4, 10);

	IPAddress gateway = IPAddress(ipAddressBuf);
	gateway[3] = 1;

	WiFi.config(ipAddressBuf, gateway, IPAddress({0xFF, 0xFF, 0xFF, 0x00 }));
}

void ESP8266NativeNetworkAdapter::setup() {
	this->server = new WiFiServer(protocolHandler->port);
	this->server->begin();
}

void ESP8266NativeNetworkAdapter::loop() {
	// listen for incoming clients
	WiFiClient client = server->available();
	if (client) {
		int t = 0;
		while (!client.available() && t++ < 500) {
			delay(1);
		}
		protocolHandler->processClientRequest(&client);
	}
}

#endif
