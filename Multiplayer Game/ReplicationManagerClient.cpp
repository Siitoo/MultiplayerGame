#include "Networks.h"
#include "ReplicationManagerServer.h"
#include "ModuleGameObject.h"

void ReplicationManagerClient::Read(const InputMemoryStream &packet)
{
	ReplicationCommand replication_packet;
	
	packet >> replication_packet.networkId;
	packet >> replication_packet.action;

	switch (replication_packet.action)
	{
	case ReplicationAction::Create:

		GameObject go;
		packet >> go.position.x;
		packet >> go.position.y;
		packet >> go.size.x;
		packet >> go.size.y;
		packet >> go.angle;
		packet >> go.tag;

		//instantiate and add it to the list


		App->modGameObject->Instantiate();
		App->modGameObject->goCount += 1;
		App->modGameObject->AddGOtoArray(go);

		break;

	case ReplicationAction::Update:

		packet >> go.position.x;
		packet >> go.position.y;
		packet >> go.angle;



		break;

	case ReplicationAction::Destroy:

		break;

	
}