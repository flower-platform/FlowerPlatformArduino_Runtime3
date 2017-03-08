/**
 * @author Claudiu Matei
 *
 */


#ifndef NETWORKCONNECTION_H_
#define NETWORKCONNECTION_H_

#include <FlowerPlatformArduinoRuntime.h>
#include <Print.h>
#include <Stream.h>

class NetworkConnection {
public:

	const char* remoteAddress;

	int endpointPort = 80;

	Stream* in = NULL;

	Print* out = NULL;

	NetworkConnection(const char* remoteAddressPSTR, int endpointPort) {
		this->remoteAddress = remoteAddressPSTR;
		this->endpointPort = endpointPort;
	}

	void startHttpRequest(const char* url, int contentLength = -1) {
		write_P(out, PSTR("POST ")); out->print(url); write_P(out, PSTR(" HTTP/1.1\r\n"));
		if (contentLength >= 0) {
			write_P(out, PSTR("Content-Length: ")); out->println(contentLength);
		}
		write_P(out, PSTR("Host: ")); write_P(out, remoteAddress); out->println();
		out->println();
	}

	virtual int connect() = 0;

	virtual void disconnect() = 0;

	virtual uint8_t connected() = 0;

	virtual int available() = 0;

	virtual int read() = 0;

	virtual void flush() = 0;

	virtual ~NetworkConnection() { }

};


#endif /* NETWORKCONNECTION_H_ */
