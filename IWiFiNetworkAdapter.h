/*
 *  Author: Claudiu Matei
 */

#ifndef IWiFiNetworkAdapter_h
#define IWiFiNetworkAdapter_h

#include <INetworkAdapter.h>
#include <WString.h>

class IWiFiNetworkAdapter : public INetworkAdapter {
protected:

	String ssid;

	String password;

public:

	IWiFiNetworkAdapter(String ipAddress, String ssid, String password) : INetworkAdapter(ipAddress) {
		this->ssid = ssid;
		this->password = password;
	};

};

#endif
