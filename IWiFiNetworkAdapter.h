/*
 *  Author: Claudiu Matei
 */

#ifndef IWiFiNetworkAdapter_h
#define IWiFiNetworkAdapter_h

#include <INetworkAdapter.h>

class IWiFiNetworkAdapter : public INetworkAdapter {
public:

	const char* ssid;

	const char* password;

};

#endif
