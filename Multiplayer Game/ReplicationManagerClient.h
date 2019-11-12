#ifndef _REPLICATIONMANAGERCLIENT_
#define _REPLICATIONMANAGERCLIENT_

#include "ReplicationManagerServer.h"

enum class ReplicationAction;
struct ClientProxy;
struct ReplicationCommand;

class ReplicationManagerClient {

public:

	void Read(const InputMemoryStream &packet);

};

#endif

