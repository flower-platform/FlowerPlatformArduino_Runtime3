/**
 * @author Claudiu Matei
 */

#ifndef ETHERNETREMOTEOBJECT_H_
#define ETHERNETREMOTEOBJECT_H_

#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>
#include <RemoteObject.h>

class EthernetRemoteObjectServer : public RemoteObjectServer {
public:

	EthernetRemoteObjectServer(const char* macAddress, const char* ipAddress, int port, const char* securityTokenPSTR);

	void loop();

protected:

	EthernetServer* server = NULL;

};

EthernetRemoteObjectServer::EthernetRemoteObjectServer(const char* macAddress, const char* ipAddress, int port, const char* securityTokenPSTR) : RemoteObjectServer(securityTokenPSTR) {
	uint8_t  macAddressBuf[6];
	parseBytes(macAddress, ':', macAddressBuf, 6, 16);

	uint8_t ipAddressBuf[4];
	parseBytes(ipAddress, '.', ipAddressBuf, 4, 10);

	Ethernet.begin(macAddressBuf, ipAddressBuf);

	server = new EthernetServer(port);
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

	processCommand(&client, &client);
	client.flush();
	delay(100);
	client.stop();
}

#endif /* ETHERNETREMOTEOBJECT_H_ */
