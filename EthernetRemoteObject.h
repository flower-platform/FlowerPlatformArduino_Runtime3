/**
 * @author Claudiu Matei
 *
 */

#ifndef ETHERNETNETREMOTEOBJECT_H_
#define ETHERNETNETREMOTEOBJECT_H_

#include <Arduino.h>
#include <EthernetClient.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <HardwareSerial.h>
#include <RemoteObject.h>
#include <RemoteObjectProtocol.h>
#include <SmartBuffer.h>
#include <Stream.h>
#include <utility/w5100.h>

class EthernetRemoteObject : public RemoteObject {
public:

	EthernetRemoteObject(const char* rappInstancePSTR, const char* instanceNamePSTR, const char* securityTokenPSTR, const char* remoteAddressPSTR, int endpointPort) : RemoteObject(rappInstancePSTR, instanceNamePSTR, securityTokenPSTR) {
		this->remoteAddress = remoteAddressPSTR;
		this->endpointPort = endpointPort;
		W5100.setRetransmissionTime(0x1388); // 500ms
		W5100.setRetransmissionCount(3);
	}

protected:

	const char* remoteAddress;

	int endpointPort = 80;

	void disconnect();

	Stream* sendRequest(SmartBuffer<DEFAULT_BUFFER_SIZE>* buf, SmartBuffer<>* argsBuf);

};

Stream* EthernetRemoteObject::sendRequest(SmartBuffer<DEFAULT_BUFFER_SIZE>* buf, SmartBuffer<>* argsBuf) {
	// connect
	char address[strlen_P(remoteAddress) + 1];
	strcpy_P(address, remoteAddress);
	Serial.print(address); Serial.println(" connecting...");
	EthernetClient client;
	int connStatus = client.connect(address, endpointPort);
	Serial.print(address); Serial.print(" connected: "); Serial.println(connStatus);

	// compute payload size
	size_t contentLength = buf->available();
	if (argsBuf) {
		contentLength += argsBuf->available();
	}

	// send http headers
	write_P(&client, PSTR("POST ")); client.print(rappInstance ? "/hub" : "/remoteObject"); write_P(&client, PSTR(" HTTP/1.1\r\n"));
	write_P(&client, PSTR("Content-Length: ")); client.println(contentLength);
	write_P(&client, PSTR("Host: ")); write_P(&client, remoteAddress); client.println();
	client.println();

	// send payload
	buf->flush(&client);
	if (argsBuf) {
		argsBuf->flush(&client);
	}

	// skip http headers; exit if headers not found
	if (!client.find((char*) "\r\n\r\n")) {
		return NULL;
	}

	// read response
	size_t n = client.readBytesUntil(EOT, buf->getBuffer(), buf->capacity());
	buf->setSize(n);
	Serial.println(n);

	// disconnect
	delay(100);
	client.stop();
	Serial.println("Disconnected");

	return buf;
}


#endif /* ETHERNETNETREMOTEOBJECT_H_ */
