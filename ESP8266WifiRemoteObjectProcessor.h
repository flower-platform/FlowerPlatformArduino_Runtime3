/**
 * @author Claudiu Matei
 */

#ifndef ESP8266WIFIREMOTEOBJECTPROCESSOR_H_
#define ESP8266WIFIREMOTEOBJECTPROCESSOR_H_

#include <HttpRemoteObjectProcessor.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

class ESP8266WiFiRemoteObjectProcessor : public HttpRemoteObjectProcessor {
public:

	ESP8266WiFiRemoteObjectProcessor(int port, const char* securityTokenPSTR);

	void sendPacketHeader(Print* out, char command, const char* securityTokenPSTR, size_t payloadSize);

	bool loop();

protected:

	WiFiServer* server = NULL;

};

ESP8266WiFiRemoteObjectProcessor::ESP8266WiFiRemoteObjectProcessor(int port, const char* securityTokenPSTR) : HttpRemoteObjectProcessor(securityTokenPSTR) {
	server = new WiFiServer(port);
	server->begin();
}

bool ESP8266WiFiRemoteObjectProcessor::loop() {
	WiFiClient client = server->available();
	if (client) {
		return HttpRemoteObjectProcessor::processClient(&client);
	}
	return false;
}

void ESP8266WiFiRemoteObjectProcessor::sendPacketHeader(Print* out, char command, const char* securityTokenPSTR, size_t payloadSize) {
	BufferedPrint<96> bufOut(out);
	HttpRemoteObjectProcessor::sendPacketHeader(&bufOut, command, securityTokenPSTR, payloadSize);
	bufOut.flush();
}

#endif /* ESP8266WIFIREMOTEOBJECTPROCESSOR_H_ */
