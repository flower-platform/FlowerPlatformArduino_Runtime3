/*
 *  Author: Claudiu Matei
 */

#ifndef IWiFiNetworkAdapter_h
#define IWiFiNetworkAdapter_h

#include <INetworkAdapter.h>

class IWiFiNetworkAdapter : INetworkAdapter {
public:

	char* ssid;

	const char* password;

};

#endif
