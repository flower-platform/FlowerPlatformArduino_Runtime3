/**
 * @author Claudiu Matei
 */

#ifndef ETHERNETREMOTEOBJECTPROCESSOR_H_
#define ETHERNETREMOTEOBJECTPROCESSOR_H_

#include <EthernetClient.h>
#include <EthernetServer.h>
#include <HttpRemoteObjectProcessor.h>

class EthernetRemoteObjectProcessor : public HttpRemoteObjectProcessor {
public:

	EthernetRemoteObjectProcessor(int port, const char* securityTokenPSTR);

	void loop();

protected:

	EthernetServer* server = NULL;

};

EthernetRemoteObjectProcessor::EthernetRemoteObjectProcessor(int port, const char* securityTokenPSTR) : HttpRemoteObjectProcessor(securityTokenPSTR) {
	server = new EthernetServer(port);
	server->begin();
}

void EthernetRemoteObjectProcessor::loop() {
	EthernetClient client = server->available();
	if (client) {
		HttpRemoteObjectProcessor::processClient(&client);
	}
}

#endif /* ETHERNETREMOTEOBJECTPROCESSOR_H_ */
