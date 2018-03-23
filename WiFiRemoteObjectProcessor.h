/**
 * @author Claudiu Matei
 */

#ifndef ESP8266WIFIREMOTEOBJECTPROCESSOR_H_
#define ESP8266WIFIREMOTEOBJECTPROCESSOR_H_

#include <HttpRemoteObjectProcessor.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

class WiFiRemoteObjectProcessor : public HttpRemoteObjectProcessor {
public:

	WiFiRemoteObjectProcessor(int port, const char* securityTokenPSTR);

	bool loop();

protected:

	WiFiServer* server = NULL;

};

WiFiRemoteObjectProcessor::WiFiRemoteObjectProcessor(int port, const char* securityTokenPSTR) : HttpRemoteObjectProcessor(securityTokenPSTR) {
	server = new WiFiServer(port);
	server->begin();
}

bool WiFiRemoteObjectProcessor::loop() {
	WiFiClient client = server->available();
	if (client) {
		Serial.print("Connection from: "); client.remoteIP().printTo(Serial); Serial.print(":"); Serial.println(client.remotePort());
		return HttpRemoteObjectProcessor::processClient(&client);
	}
	return false;
}

#endif /* ESP8266WIFIREMOTEOBJECTPROCESSOR_H_ */
