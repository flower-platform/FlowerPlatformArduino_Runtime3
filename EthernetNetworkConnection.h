/**
 * @author Claudiu Matei
 *
 */

#ifndef ETHERNETNETWORKCONNECTION_H_
#define ETHERNETNETWORKCONNECTION_H_

#include <avr/pgmspace.h>
#include <Arduino.h>
#include <EthernetClient.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>
#include <NetworkConnection.h>
#include <Printable.h>
#include <stdint.h>
#include <utility/w5100.h>
#include <WString.h>

class EthernetNetworkConnection : public NetworkConnection {
public:

	EthernetNetworkConnection(const char* remoteAddressPSTR, int endpointPort) : NetworkConnection(remoteAddressPSTR, endpointPort) {
		W5100.setRetransmissionTime(0x1388); // 500ms
		W5100.setRetransmissionCount(3);
		bufOut.out = &client;
		in = &client;
		out = &bufOut;
	}

protected:

	EthernetClient client;

	BufferedPrint<32> bufOut;

	int connect();

	void disconnect();

	uint8_t connected() {
		return client.connected();
	}

	int available() {
		return client.available();
	}

	int read() {
		return client.read();
	}

	void flush() {
		bufOut.flush();
		client.flush();
	}

};

int EthernetNetworkConnection::connect() {
	char address[strlen_P(remoteAddress) + 1];
	strcpy_P(address, remoteAddress);
	Serial.print(address); Serial.println(" connecting...");
	int connStatus = client.connect(address, endpointPort);
	Serial.print(address); Serial.print(" connected: "); Serial.println(connStatus);
	return connStatus;
}

void EthernetNetworkConnection::disconnect() {
	flush();
	delay(100);
	client.stop();
	Serial.println("Disconnected");
}

#endif /* ETHERNETNETWORKCONNECTION_H_ */
