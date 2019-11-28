#ifndef _REPLICATIONMANAGERCLIENT_
#define _REPLICATIONMANAGERCLIENT_


class ReplicationManagerClient {

public:

	void Read(const InputMemoryStream &packet);
	uint32 network_id = 0;
};

#endif



