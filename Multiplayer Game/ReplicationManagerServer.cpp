#include "Networks.h"
#include "ReplicationManagerServer.h"

void ReplicationManagerServer::Create(uint32 networkId)
{
	ReplicationCommand command;
	command.networkId = networkId;
	command.action = ReplicationAction::Create;
	commands.push_back(command);
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

void ReplicationManagerServer::Write(OutputMemoryStream &packet)
{
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

		it->action = ReplicationAction::None;
	}

}

bool ReplicationManagerServer::HasCommands()
{
	for (int i = 0; i < commands.size(); ++i)
	{
		if (commands[i].action != ReplicationAction::None)
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

