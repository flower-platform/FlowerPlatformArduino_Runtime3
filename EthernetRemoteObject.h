/**
 * @author Claudiu Matei
 *
 */

#ifndef ETHERNETNETREMOTEOBJECT_H_
#define ETHERNETNETREMOTEOBJECT_H_

#include <HttpRemoteObject.h>
#include <utility/w5100.h>

class EthernetRemoteObject : public HttpRemoteObject {
public:

	EthernetRemoteObject(const char* nodeIdPSTR, const char* objectNamePSTR, const char* securityTokenPSTR, const char* remoteAddressPSTR, uint16_t remotePort, Client* client)
			: HttpRemoteObject(nodeIdPSTR, objectNamePSTR, securityTokenPSTR, remoteAddressPSTR, remotePort, client) {

		W5100.setRetransmissionTime(0x1388); // 500ms
		W5100.setRetransmissionCount(3);
	}

};

#endif /* ETHERNETNETREMOTEOBJECT_H_ */
