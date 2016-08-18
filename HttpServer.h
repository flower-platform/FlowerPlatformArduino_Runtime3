/*
 *  Author: Claudiu Matei
 */

#ifndef HttpServer_h
#define HttpServer_h

#include <Client.h>
#include <FlowerPlatformArduinoRuntime.h>
#include <IProtocolHandler.h>


#ifdef DEBUG_HttpServer
#define DB_P_HttpServer(text) Serial.print(text)
#define DB_PLN_HttpServer(text) Serial.println(text)
#else
#define DB_P_HttpServer(text)
#define DB_PLN_HttpServer(text)
#endif

#define LINE_BUFFER_SIZE 256

class HttpServer;

class HttpCommandEvent {
public:

	const char* url;

	HttpServer* server;

	Client* client;

};

class HttpServer : public IProtocolHandler {
protected:

	Client* activeClient = NULL;

	void dispatchEvent(const char* requestMethod, const char* requestUrl, Client* client);

public:

	static const int CONTENT_TYPE_JSON = 0;

	static const int CONTENT_TYPE_HTML = 1;

	Callback<HttpCommandEvent>* onCommandReceived = NULL;

	/*
	 * @flower { constructorVariant="Default" }
	 */
	HttpServer(int port = 80);

	void processClientRequest(Client* client);

	void httpSuccess(Print* print, int contentType = CONTENT_TYPE_JSON, bool bzip = false);

	void httpError404(Print* print);

	void getCommandFromUrl(const char* url, char* command);

	void getStringParameterValue(const char* url, const char* param, char* value);

	int getIntParameterValue(const char* url, const char* param);

	bool getBoolParameterValue(const char* url, const char* param);

	void setup() { };

	void loop() { };

};

HttpServer::HttpServer(int port) : IProtocolHandler(port) { };

void HttpServer::processClientRequest(Client* client) {
	activeClient = client;

	DB_P_HttpServer("data received");
	DB_P_HttpServer(' '); DB_P_HttpServer(client->available());
	DB_P_HttpServer(' '); DB_PLN_HttpServer(client->connected());

	char currentLine[LINE_BUFFER_SIZE];
	uint8_t lineSize = 0;

	while (client->connected() && client->available()) {
		char c = client->read();
		DB_P_HttpServer("|"); DB_P_HttpServer(c);
		if (c == '\n') {
			currentLine[lineSize] = '\0';

			DB_P_HttpServer(F(">> ")); DB_PLN_HttpServer(currentLine);

			if (strncmp(currentLine, "GET", 3) == 0 || strncmp(currentLine, "POST", 4 == 0)) {
				// empty input buffer
				while (client->available()) {
					client->read();
				}
				char* k = strchr(currentLine, ' ');
				k[0] = '\0'; // break string
				k+=2; // skip space and leading url slash
				char* requestUrl = k;
				k = strchr(requestUrl, ' ');
				k[0] = 0;
				DB_PLN_HttpServer(F("httpserver dispatch... "));
				dispatchEvent(currentLine, requestUrl, activeClient);
				DB_PLN_HttpServer(F("httpserver return from dispatch... "));
				break;
			}
		}
		else if (c != '\r' && lineSize < LINE_BUFFER_SIZE - 1) {
			currentLine[lineSize++] = c;
		}
	}

	DB_PLN_HttpServer(F("closing... "));
	// empty input buffer
	while (client->available()) {
		client->read();
	}
	client->stop();
	DB_P_HttpServer(F("closed: ")); DB_PLN_HttpServer(client->connected());
}

void HttpServer::dispatchEvent(const char* requestMethod, const char* requestUrl, Client* client) {

	DB_P_HttpServer(F("HttpServer.dispatchEvent: ")); DB_P_HttpServer(requestMethod); DB_P_HttpServer(F(" * ")); DB_P_HttpServer(requestUrl); DB_PLN_HttpServer();

	if (onCommandReceived == NULL) {
		return;
	}

	HttpCommandEvent event;
	event.url = requestUrl;
	event.server = this;
	event.client = client;
	(*onCommandReceived)(&event);

}

void HttpServer::httpSuccess(Print* print, int contentType, bool gzip) {
	write_P(print, PSTR("HTTP/1.1 200 OK\r\nContent-Type: "));
	write_P(print, contentType == CONTENT_TYPE_HTML ? PSTR("text/html") : PSTR("application/json"));
	if (gzip) {
		write_P(print, PSTR("\r\nContent-Encoding: gzip"));
	}
	write_P(print, PSTR("\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n"));  // the connection will be closed after completion of the response
}

void HttpServer::httpError404(Print* print) {
	write_P(print, PSTR("HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n"));  // the connection will be closed after completion of the response
}

void HttpServer::getCommandFromUrl(const char* url, char* command) {
	char* p = strchr(url, '?');
	if (p == NULL) {
		strcpy(command, url);
	} else {
		uint8_t n = p - url;
		strncpy(command, url, n);
		command[n] = '\0';
	}
}

void HttpServer::getStringParameterValue(const char* url, const char* param, char* value) {
	int paramLength = strlen(param);
	int urlLength = strlen(url);

	// look for param string start in url
	const char* st = strchr(url, '?');
	while (st != NULL && ( !(*(st - 1) == '?' || *(st - 1) == '&') || st + paramLength > url + urlLength || *(st + paramLength) != '=')) {
		st = strstr(st + 1, param);
	}
	if (st == NULL) {
		value[0] = '\0';
		return;
	}

	// get value
	st += paramLength + 1; // value start
	char* en = strchr(st, '&'); // look for end of value string
	if (en == NULL) {
		strcpy(value, st);
	} else {
		uint8_t n = en - st;
		strncpy(value, st, n);
		value[n] = '\0';
	}
}

int HttpServer::getIntParameterValue(const char* url, const char* param) {
	char valStr[8];
	getStringParameterValue(url, param, valStr);
	return atoi(valStr);
}

bool HttpServer::getBoolParameterValue(const char* url, const char* param) {
	char valStr[8];
	getStringParameterValue(url, param, valStr);
	return strcmp(valStr, "true") == 0;
}

#endif
