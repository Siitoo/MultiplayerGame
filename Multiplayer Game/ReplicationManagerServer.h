#ifndef _REPLICATIONMANAGERSERVER_
#define _REPLICATIONMANAGERSERVER_

enum class ReplicationAction {None, Create, Update, Destroy};
struct ClientProxy;

struct ReplicationCommand
{
	ReplicationAction action = ReplicationAction::None;
	uint32 networkId;
};

class ReplicationManagerServer
{
public:

	void Create(uint32 networkId);
	void Update(uint32 networkId);
	void Destroy(uint32 networkId);

	void Write(OutputMemoryStream &packet);

	void SetClientId(uint32 clientId);
	uint32 GetClientId();

	bool HasCommands();

private:

	std::vector<ReplicationCommand> commands;

	uint32 clientId;

};

#endif // !_REPLICATIONMANAGERSERVER_


