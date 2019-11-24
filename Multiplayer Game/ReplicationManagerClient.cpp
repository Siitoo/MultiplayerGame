#include "Networks.h"

#include "ReplicationManagerServer.h"


void ReplicationManagerClient::Read(const InputMemoryStream &packet)
{
	while (packet.RemainingByteCount() > 0)
	{
		ReplicationCommand replication_packet;

		packet >> replication_packet.networkId;
		packet >> replication_packet.action;

		GameObject* tmp_go = App->modLinkingContext->getNetworkGameObject(replication_packet.networkId);

		switch (replication_packet.action)
		{
		case ReplicationAction::Create:
		{
			
			if (tmp_go == nullptr)
			{
				GameObject* go = Instantiate();

				go->networkId = replication_packet.networkId;

				packet >> go->position.x;
				packet >> go->position.y;
				packet >> go->size.x;
				packet >> go->size.y;
				packet >> go->angle;
				packet >> go->tag;
				packet >> go->parent_tag;

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
				else
					go->texture = App->modResources->laser;

				if (go->tag < 3)
				{
					go->collider = App->modCollision->addCollider(ColliderType::Player, go);
					go->behaviour = new Spaceship;
					App->modLinkingContext->registerNetworkGameObject(go);
				}
				else
				{
					go->collider = App->modCollision->addCollider(ColliderType::Laser, go);
					go->behaviour = new Laser;
					App->modLinkingContext->registerNetworkGameObjectWithNetworkId(go, replication_packet.networkId);
				}


				go->behaviour->gameObject = go;
				tmp_go = go;
			}
			else
			{
				vec2 fakePosition;
				vec2 fakeSize;
				float fakeAngle;

				packet >> fakePosition.x;
				packet >> fakePosition.y;
			
				packet >> fakeSize.x;
				packet >> fakeSize.y;
				packet >> fakeAngle;
				packet >> tmp_go->tag;
				packet >> tmp_go->parent_tag;
			}
			break;
		}
		case ReplicationAction::Update:
		{

			//Sito this is for laser and crash, well
			if (tmp_go != nullptr)
			{
				packet >> tmp_go->position.x;
				packet >> tmp_go->position.y;
				packet >> tmp_go->angle;
			}
			else
			{
				vec2 fakePosition;
				float fakeAngle;
				packet >> fakePosition.x;
				packet >> fakePosition.y;
				packet >> fakeAngle;
			}

			break;
		}
		case ReplicationAction::Destroy:
		{
			
			if (tmp_go != nullptr)
			{
				App->modLinkingContext->unregisterNetworkGameObject(tmp_go);
				Destroy(tmp_go);
			}
			break;
		}
		}

		if (replication_packet.action != ReplicationAction::Destroy && tmp_go != nullptr)
		{
			packet >> tmp_go->totalLife;
			packet >> tmp_go->totalKills;
		}

		bool input = false;
		packet >> input;
		if (input)
		{
			uint32 lastInput = 0;
			packet >> lastInput;
			App->modNetClient->SetLastInput(lastInput);
		}
	}
}