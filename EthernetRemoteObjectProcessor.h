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

	bool loop();

protected:

	EthernetServer* server = NULL;

};

EthernetRemoteObjectProcessor::EthernetRemoteObjectProcessor(int port, const char* securityTokenPSTR) : HttpRemoteObjectProcessor(securityTokenPSTR) {
	server = new EthernetServer(port);
	server->begin();
}

bool EthernetRemoteObjectProcessor::loop() {
	EthernetClient client = server->available();
	if (client) {
		return HttpRemoteObjectProcessor::processClient(&client);
	}
	return false;
}

#endif /* ETHERNETREMOTEOBJECTPROCESSOR_H_ */
