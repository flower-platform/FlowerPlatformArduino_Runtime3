/*
 *  Author: Claudiu Matei
 */

#ifndef ESP8266NativeNetworkAdapter_h
#define ESP8266NativeNetworkAdapter_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <IPAddress.h>
#include <IProtocolHandler.h>
#include <IWiFiNetworkAdapter.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

class ESP8266NativeNetworkAdapter : public IWiFiNetworkAdapter {
protected:

	WiFiServer* server = NULL;

public:

	void setup();

	void loop();

};

void ESP8266NativeNetworkAdapter::setup() {
	this->server = new WiFiServer(protocolHandler->port);
	WiFi.begin(ssid, password);
	IPAddress ip = IPAddress(ipAddress);
	IPAddress gateway = ip;
	gateway[3] = 1;
	WiFi.config(ip, gateway, IPAddress({0xFF, 0xFF, 0xFF, 0x00 }));
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
