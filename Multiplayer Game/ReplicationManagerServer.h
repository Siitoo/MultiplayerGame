#ifndef _REPLICATIONMANAGERSERVER_
#define _REPLICATIONMANAGERSERVER_

enum class ReplicationAction { None, Create, Update, Destroy };
struct ClientProxy;

struct ReplicationCommand
{
	ReplicationAction action = ReplicationAction::None;
	uint32 networkId;
	bool input = false;
};

class ReplicationManagerServer
{
public:

	void Create(uint32 networkId);
	void Update(uint32 networkId);
	void Destroy(uint32 networkId);
	void InputNumber(uint32 networkId);
	void Write(OutputMemoryStream &packet, Delivery& delivery);

	void HandleCreate(uint32 networkId);
	void SuccesDestroy(uint32 networkId);

	void SetClientId(uint32 clientId);
	uint32 GetClientId();

	bool HasCommands();

private:

	std::vector<ReplicationCommand> commands;

	uint32 clientId;

};

#endif // !_REPLICATIONMANAGERSERVER_



