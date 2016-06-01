/*
 *  Author: Claudiu Matei
 */

#ifndef ESP8266NetworkAdapter_h
#define ESP8266NetworkAdapter_h

#include <FlowerPlatformArduinoRuntime.h>
#include <Arduino.h>
#include <Client.h>
#include <HardwareSerial.h>
#include <IPAddress.h>


//#define DEBUG_ESP8266NetworkAdapter

#ifdef DEBUG_ESP8266NetworkAdapter
#define DB_P_ESP8266NetworkAdapter(text) Serial.print(text)
#define DB_PLN_ESP8266NetworkAdapter(text) Serial.println(text)
#else
#define DB_P_ESP8266NetworkAdapter(text)
#define DB_PLN_ESP8266NetworkAdapter(text)
#endif

#ifdef UBRR1H // has Serial1
#define esp Serial1
#else
#define esp Serial
#undef DEBUG_ESP8266NetworkAdapter
#endif
#define MAX_CLIENTS 4
#define READ_BUFFER_SIZE 64
#define WRITE_TIMEOUT 1500

/*
 * @fp.ignore
 */
class CharSequenceParser {
public:
	CharSequenceParser(const char* sequence) {
		this->sequence = sequence;
		this->index = 0;
		this->length = strlen(sequence);
		this->wildcardValue = '\0';
	}

	bool parseNextChar(char c) {
		if (sequence[index] != c && sequence[index] != '?' && index > 0) {
			index = 0;
		}
		if (sequence[index] == '?') {
			wildcardValue = c;
			index++;
		} else if (sequence[index] == c) {
			index++;
		}
		if (index == length) {
			index = 0;
			return true;
		}
		return false;
	}

	char wildcardValue; // stores the char corresponding to the last wildcard character

protected:
	const char* sequence;
	uint8_t index;
	uint8_t length;
};

class ESP8266NetworkAdapter;

/*
 * @fp.ignore
 */
class ESP8266Client : public Client {
public:

	ESP8266NetworkAdapter* networkAdapter;

	uint8_t clientId;

	uint8_t buf[READ_BUFFER_SIZE];	// read buffer

	size_t bytesExpected;	// number of bytes promised by the ESP8266 module

	long lastRead; // timestamp of last read operation

	int bufSt, bufEn;	// start and end indexes in buffer

	bool stopped = true;	// flag set when stop() method is called; used as a mark for closing the connection

	bool closed = true;		// flag is set when a DISCONNECTED event is received (the socket is closed)

	bool accumulate(uint8_t b);	// tries to accumulate on byte in the buffer; returns whether the byte was accumulated or not (when buffer is full)

	void reset();	// resets buffer head and tail; marks the client as active (closed=false, stopped=false)

	int connect(IPAddress ip, uint16_t port);

	int connect(const char *host, uint16_t port);

	size_t write(uint8_t);

	size_t write(const uint8_t *buf, size_t size);

	int available();

	int read();

	int read(uint8_t *buf, size_t size);

	int peek();

	void flush();

	void stop();

	uint8_t connected();

	operator bool() {
		return true;
	}

	virtual ~ESP8266Client() { }

};


class ESP8266NetworkAdapter : public IWiFiNetworkAdapter {
public:

	// write status constants
	static const int WRITE_STATUS_READY = 0;
	static const int WRITE_STATUS_WAITING_FOR_TRANSMIT_READY = 1;
	static const int WRITE_STATUS_TRANSMIT_READY = 2;
	static const int WRITE_STATUS_WAITING_FOR_SEND_OK = 3;
	static const int WRITE_STATUS_CLOSING_CONNECTION = 4;

	// parsers for module's command responses
	static CharSequenceParser readyParser;
	static CharSequenceParser okParser;
	static CharSequenceParser okToWriteParser;
	static CharSequenceParser sendOkParser;

	// parsers for module's events
	static CharSequenceParser connectParser;
	static CharSequenceParser disconnectParser;
	static CharSequenceParser ipdParser;

	bool accessPointMode = false;

	ESP8266Client clients[MAX_CLIENTS];

	uint8_t extraClientsStack[8];	// stack holding ids of extra connections (when there are more than MAX_CLIENTS connections)

	uint8_t extraClientsSP;	// stack pointer

	ESP8266Client* recvClient;	// active receiver

	void setup();

	void loop();

	int readNextChar(); // reads a char from ESP8266 and processes it

	void closeInactiveClients();	// closes the clients marked to be closed ("stopped") and the extra connections

	void closeConnection(uint8_t clientId);

	int writeStatus;

	unsigned long readDeadlineTimestamp;

};

void ESP8266Client::reset() {
	bufSt = 0;
	bufEn = 0;
	stopped = false;
	closed = false;
};

size_t ESP8266Client::write(uint8_t b) {
	return write(&b, 1);
}

size_t ESP8266Client::write(const uint8_t* buf, size_t size) {
	DB_P_ESP8266NetworkAdapter("* write "); DB_PLN_ESP8266NetworkAdapter(clientId);

	unsigned long writeDeadline;

	esp.print(F("AT+CIPSENDBUF=")); esp.print(clientId); esp.print(','); esp.print(size); esp.print("\r\n"); // prepare for transmission

	networkAdapter->writeStatus = ESP8266NetworkAdapter::WRITE_STATUS_WAITING_FOR_TRANSMIT_READY;
	writeDeadline = millis() + WRITE_TIMEOUT;

	do {
		networkAdapter->readNextChar();
	} while (networkAdapter->writeStatus != ESP8266NetworkAdapter::WRITE_STATUS_TRANSMIT_READY && millis() < writeDeadline);

	DB_PLN_ESP8266NetworkAdapter("* ready");

	if (networkAdapter->writeStatus != ESP8266NetworkAdapter::WRITE_STATUS_TRANSMIT_READY) {
		networkAdapter->writeStatus = ESP8266NetworkAdapter::WRITE_STATUS_READY;
		return 0;
	}

	// write data buffer
	esp.write(buf, size);


	// wait for end of transmission
	networkAdapter->writeStatus = ESP8266NetworkAdapter::WRITE_STATUS_WAITING_FOR_SEND_OK;
	writeDeadline = millis() + WRITE_TIMEOUT;
	do {
		networkAdapter->readNextChar();
	} while (networkAdapter->writeStatus != ESP8266NetworkAdapter::WRITE_STATUS_READY && millis() < writeDeadline);

	if (networkAdapter->writeStatus != ESP8266NetworkAdapter::WRITE_STATUS_READY) {
		networkAdapter->writeStatus = ESP8266NetworkAdapter::WRITE_STATUS_READY;
		return 0;
	}

	networkAdapter->writeStatus = ESP8266NetworkAdapter::WRITE_STATUS_READY;

	return size;
}

int ESP8266Client::available() {
	return bufEn - bufSt;
}

int ESP8266Client::read() {
	if (bufSt == bufEn) {
		return -1;
	}
	return buf[bufSt++];
}

bool ESP8266Client::accumulate(uint8_t b) {
	if (bytesExpected == 0) {
		return false;
	}
	bytesExpected--;
	if (bufSt == bufEn) {
		// reset buffer indexes
		bufSt = 0;
		bufEn = 0;
	}
	if (bufEn < READ_BUFFER_SIZE) {
		buf[bufEn++] = b;
		return true;
	}
	return false;
}

void ESP8266Client::stop() {
	stopped = true;
	DB_P_ESP8266NetworkAdapter("* "); DB_P_ESP8266NetworkAdapter(clientId); DB_PLN_ESP8266NetworkAdapter(" stopped");
}

uint8_t ESP8266Client::connected() {
	return !closed;
}

// not implemented
int ESP8266Client::connect(IPAddress ip, uint16_t port) {
	return 0;
}
int ESP8266Client::connect(const char *host, uint16_t port) {
	return 0;
}
int ESP8266Client::read(uint8_t *buf, size_t size) {
	return 0;
}
int ESP8266Client::peek() {
	return 0;
}
void ESP8266Client::flush() {	}


CharSequenceParser ESP8266NetworkAdapter::readyParser("\r\nready\r\n");

CharSequenceParser ESP8266NetworkAdapter::okParser("OK\r\n");

CharSequenceParser ESP8266NetworkAdapter::connectParser("?,CONNECT");

CharSequenceParser ESP8266NetworkAdapter::disconnectParser("?,CLOSED");

CharSequenceParser ESP8266NetworkAdapter::okToWriteParser("OK\r\n> ");

CharSequenceParser ESP8266NetworkAdapter::sendOkParser("SEND OK\r\n");

CharSequenceParser ESP8266NetworkAdapter::ipdParser("+IPD,");


void ESP8266NetworkAdapter::setup() {
	for (uint8_t i = 0; i < MAX_CLIENTS; i++) {
		clients[i].clientId = i;
		clients[i].networkAdapter = this;
	}
	extraClientsSP = 0;

	delay(1000); // wait for ESP to boot up

	DB_PLN_ESP8266NetworkAdapter(F("ESP setup"));

	esp.begin(115200);
	char c;

	// reset ESP8266
	esp.println(F("AT+RST"));
	while ((c = esp.read()) == -1 || !readyParser.parseNextChar(c));

	// echo off
	esp.println(F("ATE0"));
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	// set WiFi mode
	esp.print(F("AT+CWMODE_CUR=")); esp.println(accessPointMode ? 2 : 1);
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

//	 DHCP settings (disable)
//	esp.println(F("AT+CWDHCP=0,0"));
//	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	// set static IP address
	esp.print(accessPointMode ? F("AT+CIPAP_CUR=\"") : F("AT+CIPSTA_CUR=\""));
	esp.print(ipAddress[0]); esp.print(".");
	esp.print(ipAddress[1]); esp.print(".");
	esp.print(ipAddress[2]); esp.print(".");
	esp.print(ipAddress[3]); esp.println("\"");
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	// access point mode setting (either set up as AP, or join an AP)
	esp.print(accessPointMode ? F("AT+CWSAP_CUR=\"") : F("AT+CWJAP_CUR=\""));
	esp.print(ssid); esp.print(F("\",\"")); esp.print(password); esp.print("\"");
	if (accessPointMode) {
		esp.print(",1,4"); // channel 1 works fine; other channels don't
	}
	esp.println();
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));



	// set mux mode to multi
	esp.println(F("AT+CIPMUX=1"));
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));

	//	// set server timeout (3s); we don't rely on this
	//	esp.println(F("AT+CIPSTO=3"));
	//	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c))  { if (c != -1) Serial.print(c); }
	//	DB_PLN_ESP8266NetworkAdapter(F("set timeout"));

	// open port (server socket)
	esp.print(F("AT+CIPSERVER=1,")); esp.print(protocolHandler->port); esp.println();
	while ((c = esp.read()) == -1 || !okParser.parseNextChar(c));


	DB_PLN_ESP8266NetworkAdapter(F("ESP setup ready"));

}

void ESP8266NetworkAdapter::loop() {
	#ifdef DEBUG_ESP8266NetworkAdapter
	while (Serial.available()) {
		esp.write(Serial.read());
	}
	#endif

	while (esp.available()) {
		readNextChar();
	}

	if (recvClient != NULL && millis() < readDeadlineTimestamp) {
		return;
	}

	closeInactiveClients();

	// dispatch events
	for (uint8_t i = 0; i < MAX_CLIENTS; i++) {
		ESP8266Client* client = &clients[i];
		if (client->available()) {
			if (client->connected()) {
				protocolHandler->processClientRequest(client);
			} else if (!client->stopped) {
				client->stop();
			}
		}
	}
}

int ESP8266NetworkAdapter::readNextChar() {
	if (!esp.available()) {
		return writeStatus;
	}

	char c = esp.read();

	#ifdef DEBUG_ESP8266NetworkAdapter
	if (recvClient == NULL) {
		Serial.write(c);
	}
	#endif

	if (okToWriteParser.parseNextChar(c)) {
		writeStatus = WRITE_STATUS_TRANSMIT_READY;
	}

	if (sendOkParser.parseNextChar(c)) {
		writeStatus = WRITE_STATUS_READY;
	}

	if (connectParser.parseNextChar(c)) {
		uint8_t clientId = connectParser.wildcardValue - '0';
		DB_P_ESP8266NetworkAdapter("* Client connected: "); DB_PLN_ESP8266NetworkAdapter(clientId);
		if (clientId < MAX_CLIENTS) {
			ESP8266Client* client = &clients[clientId];
			client->reset();
		} else {
			extraClientsStack[extraClientsSP++] = clientId;
		}
	}

	if (disconnectParser.parseNextChar(c)) {
		uint8_t clientId = disconnectParser.wildcardValue - '0';
		DB_P_ESP8266NetworkAdapter("* Connection closed: "); DB_PLN_ESP8266NetworkAdapter(clientId);
		if (clientId < MAX_CLIENTS) {
			ESP8266Client* client = &clients[clientId];
			client->closed = true;
		}
		writeStatus = WRITE_STATUS_READY;
	}

	if (ipdParser.parseNextChar(c)) {
		char params[8];
		uint8_t i = 0;
		// read +IPD params; tokenize
		while (c != ':') {
			while (!esp.available()); // wait for data to become available
			c = esp.read();
			params[i++] = (c != ',' && c != ':' ? c : '\0');
		}

		uint8_t clientId = atoi(params);
		int availableBytes = atoi(params + strlen(params) + 1);
		DB_P_ESP8266NetworkAdapter("--------> "); DB_P_ESP8266NetworkAdapter(clientId); DB_P_ESP8266NetworkAdapter(" "); DB_P_ESP8266NetworkAdapter(availableBytes); DB_P_ESP8266NetworkAdapter(" "); DB_PLN_ESP8266NetworkAdapter(writeStatus);
		ESP8266Client* client = clientId < MAX_CLIENTS ? &clients[clientId] : NULL;
		if (client) {
			recvClient = client;
			recvClient->bytesExpected = availableBytes;
			readDeadlineTimestamp = millis() + 1500;
			return writeStatus;
		}
	}

	if (recvClient != NULL) {
		recvClient->accumulate(c);
		if (recvClient->bytesExpected == 0) {
			recvClient = NULL;
		}
	}

	return writeStatus;
}

void ESP8266NetworkAdapter::closeInactiveClients() {
	// Close stopped clients
	for (uint8_t i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i].stopped && !clients[i].closed) {
			closeConnection(i);
			clients[i].closed = true;
		}
	}

	// Close extra clients
	while (extraClientsSP > 0) {
		uint8_t clientId = extraClientsStack[--extraClientsSP];
		closeConnection(clientId);
	}
}

void ESP8266NetworkAdapter::closeConnection(uint8_t clientId) {
	DB_P_ESP8266NetworkAdapter("* Closing connection "); DB_PLN_ESP8266NetworkAdapter(clientId);
	write_P(&esp, PSTR("AT+CIPCLOSE=")); esp.println(clientId);
	unsigned long t = millis() + WRITE_TIMEOUT;
	writeStatus = WRITE_STATUS_CLOSING_CONNECTION;
	while (readNextChar() != ESP8266NetworkAdapter::WRITE_STATUS_READY && millis() < t);
}

#endif
