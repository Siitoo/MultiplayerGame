#include "Networks.h"

#include "ReplicationManagerServer.h"


void ReplicationManagerClient::Read(const InputMemoryStream &packet)
{
	ReplicationCommand replication_packet;

	packet >> replication_packet.networkId;
	packet >> replication_packet.action;

	switch (replication_packet.action)
	{
	case ReplicationAction::Create:
	{
		GameObject* go = Instantiate();

		go->networkId = replication_packet.networkId;

		packet >> go->position.x;
		packet >> go->position.y;
		packet >> go->size.x;
		packet >> go->size.y;
		packet >> go->angle;
		packet >> go->tag;

		if (go->tag == 0)
		{
			go->texture = App->modResources->spacecraft1;
		}
		else if (go->tag == 1)
		{
			go->texture = App->modResources->spacecraft2;
		}
		else if (go->tag == 2)
		{
			go->texture = App->modResources->spacecraft3;
		}

		go->collider = App->modCollision->addCollider(ColliderType::Player, go);
		go->behaviour = new Spaceship;
		go->behaviour->gameObject = go;
		App->modLinkingContext->registerNetworkGameObject(go);
		break;
	}
	case ReplicationAction::Update:
	{

		GameObject* go = App->modLinkingContext->getNetworkGameObject(replication_packet.networkId);

		packet >> go->position.x;
		packet >> go->position.y;
		packet >> go->angle;
		break;
	}
	case ReplicationAction::Destroy:
	{
		break;
	}
	}
}