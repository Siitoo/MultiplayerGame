#include "Networks.h"
#include "DeliveryDelegate.h"
#include "ReplicationManagerServer.h"

class DeliveryReplicationCommand : public DeliveryDelegate
{
public:
	

	void onDeliverySuccess(DeliveryManager* deliveryManager) const
	{
		ReplicationManagerServer* rs = App->modNetServer->GetReplicationServerForProxyId(clientId);
		if (rs == nullptr)
			return;
		for (int i = 0; i < deliveryReplicationCommands.size(); ++i)
		{
			ReplicationCommand rc = deliveryReplicationCommands[i];

			switch (rc.action)
			{
			case ReplicationAction::Create:
			{
				break;
			}
			case ReplicationAction::Update:
			{
				break;
			}
			case ReplicationAction::Destroy:
			{
				rs->SuccesDestroy(rc.networkId);
				break;
			}
			}

		}
	}

	void onDeliveryFailure(DeliveryManager* deliveryManager) const
	{
		ReplicationManagerServer* rs = App->modNetServer->GetReplicationServerForProxyId(clientId);
		if (rs == nullptr)
			return;
		for (int i = 0; i < deliveryReplicationCommands.size(); ++i)
		{
			ReplicationCommand rc = deliveryReplicationCommands[i];
			GameObject* go = App->modLinkingContext->getNetworkGameObject(rc.networkId);


			switch (rc.action)
			{
			case ReplicationAction::Create:
			{
				if (go != nullptr)
				{
					rs->HandleCreate(rc.networkId);
				}
				break;
			}
			case ReplicationAction::Update:
			{
				if (go!= nullptr)
				{
					rs->Update(rc.networkId);
				}
				break;
			}
			case ReplicationAction::Destroy:
			{
				rs->Destroy(rc.networkId);
				break;
			}
			}
		}
	}

	uint32 clientId = 0;
	std::vector<ReplicationCommand> deliveryReplicationCommands;
};

void ReplicationManagerServer::Create(uint32 networkId)
{
	ReplicationCommand command;
	command.networkId = networkId;
	command.action = ReplicationAction::Create;
	commands.push_back(command);
}

void ReplicationManagerServer::HandleCreate(uint32 networkId)
{
	for (std::vector<ReplicationCommand>::iterator it = commands.begin(); it != commands.end(); ++it)
	{
		if ((*it).networkId == networkId)
		{
			if((*it).action != ReplicationAction::Destroy)
				(*it).action = ReplicationAction::Create;

			break;
		}
	}
}


void ReplicationManagerServer::Update(uint32 networkId)
{
	for (std::vector<ReplicationCommand>::iterator it = commands.begin(); it != commands.end(); ++it)
	{
		if ((*it).networkId == networkId)
		{
			if((*it).action != ReplicationAction::Create)
				(*it).action = ReplicationAction::Update;

			break;
		}
	}
}

void ReplicationManagerServer::Destroy(uint32 networkId)
{
	for (std::vector<ReplicationCommand>::iterator it = commands.begin(); it != commands.end(); ++it)
	{
		if (it->networkId == networkId)
		{
			it->action = ReplicationAction::Destroy;
			break;
		}
	}
}

void ReplicationManagerServer::SuccesDestroy(uint32 networkId)
{
	for (std::vector<ReplicationCommand>::iterator it = commands.begin(); it != commands.end(); ++it)
	{
		if ((*it).networkId == networkId)
		{
			commands.erase(it);
			break;
		}
	}
}

void ReplicationManagerServer::InputNumber(uint32 networkId )
{
	for (std::vector<ReplicationCommand>::iterator it = commands.begin(); it != commands.end(); ++it)
	{
		if (it->networkId == networkId)
		{
			it->input = true;
			break;
		}
	}
}

void ReplicationManagerServer::Write(OutputMemoryStream &packet, Delivery& delivery)
{
	delivery.deliveryDelegate = new DeliveryReplicationCommand();
	((DeliveryReplicationCommand*)delivery.deliveryDelegate)->clientId = clientId;
	for (std::vector<ReplicationCommand>::iterator it = commands.begin(); it != commands.end(); ++it)
	{
		packet << it->networkId;
		packet << it->action;

		GameObject* go = App->modLinkingContext->getNetworkGameObject(it->networkId);

		switch (it->action)
		{

		case ReplicationAction::Create:
		{

			packet << go->position.x;
			packet << go->position.y;
			//packet << go->pivot;
			packet << go->size.x;
			packet << go->size.y;
			packet << go->angle;
			packet << go->tag;
			packet << go->parent_tag;
			//packet << go->texture;
			//packet << go->order;
			//packet << go->collider;
			//packet << go->behaviour;
			break;
		}

		case ReplicationAction::Update:
		{

			packet << go->position.x;
			packet << go->position.y;
			packet << go->angle;
			
			break;
		}
		}

		packet << it->input;
		if (it->input)
		{
			packet << App->modNetServer->GetLastInputSequenceNumberById(it->networkId);
			it->input = false;
		}

		((DeliveryReplicationCommand*)delivery.deliveryDelegate)->deliveryReplicationCommands.push_back(*it);

		it->action = ReplicationAction::None;
	}

}

bool ReplicationManagerServer::HasCommands()
{
	for (int i = 0; i < commands.size(); ++i)
	{
		if (commands[i].action != ReplicationAction::None || commands[i].input)
			return true;
	}


	return false;
}

void ReplicationManagerServer::SetClientId(uint32 clientId)
{
	this->clientId = clientId;
}

uint32 ReplicationManagerServer::GetClientId()
{
	return clientId;
}

