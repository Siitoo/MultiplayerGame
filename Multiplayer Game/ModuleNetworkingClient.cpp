#include "Networks.h"

#include <string>

//////////////////////////////////////////////////////////////////////
// ModuleNetworkingClient public methods
//////////////////////////////////////////////////////////////////////


void ModuleNetworkingClient::setServerAddress(const char * pServerAddress, uint16 pServerPort)
{
	serverAddressStr = pServerAddress;
	serverPort = pServerPort;
}

void ModuleNetworkingClient::setPlayerInfo(const char * pPlayerName, uint8 pSpaceshipType)
{
	playerName = pPlayerName;
	spaceshipType = pSpaceshipType;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingClient::onStart()
{
	if (!createSocket()) return;

	if (!bindSocketToPort(0)) {
		disconnect();
		return;
	}

	// Create remote address
	serverAddress = {};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	int res = inet_pton(AF_INET, serverAddressStr.c_str(), &serverAddress.sin_addr);
	if (res == SOCKET_ERROR) {
		reportError("ModuleNetworkingClient::startClient() - inet_pton");
		disconnect();
		return;
	}

	state = ClientState::Start;

	inputDataFront = 0;
	inputDataBack = 0;

	secondsSinceLastInputDelivery = 0.0f;
	secondsSinceLastPing = 0.0f;
	lastPacketReceivedTime = Time.time;
}

void ModuleNetworkingClient::onGui()
{
	if (state == ClientState::Stopped) return;

	if (ImGui::CollapsingHeader("ModuleNetworkingClient", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (state == ClientState::WaitingWelcome)
		{
			ImGui::Text("Waiting for server response...");
		}
		else if (state == ClientState::Playing)
		{
			ImGui::Text("Connected to server");

			ImGui::Separator();

			ImGui::Text("Player info:");
			ImGui::Text(" - Id: %u", playerId);
			ImGui::Text(" - Name: %s", playerName.c_str());

			ImGui::Separator();

			ImGui::Text("Spaceship info:");
			ImGui::Text(" - Type: %u", spaceshipType);
			ImGui::Text(" - Network id: %u", networkId);

			vec2 playerPosition = {};
			GameObject *playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (playerGameObject != nullptr) {
				playerPosition = playerGameObject->position;
			}
			ImGui::Text(" - Coordinates: (%f, %f)", playerPosition.x, playerPosition.y);

			ImGui::Separator();

			ImGui::Text("Connection checking info:");
			ImGui::Text(" - Ping interval (s): %f", PING_INTERVAL_SECONDS);
			ImGui::Text(" - Disconnection timeout (s): %f", DISCONNECT_TIMEOUT_SECONDS);

			ImGui::Separator();

			ImGui::Text("Input:");
			ImGui::InputFloat("Delivery interval (s)", &inputDeliveryIntervalSeconds, 0.01f, 0.1f, 4);
		}
	}

	if (state == ClientState::Playing)
	{
		if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(networkId);
			if (go != nullptr)
			{
				std::string tlife = "Current Life: ";
				tlife += std::to_string(go->totalLife);

				std::string tkills = "Total Kills: ";
				tkills += std::to_string(go->totalKills);

				ImGui::Text(tlife.c_str());
				ImGui::Text(tkills.c_str());
			}

			ImGui::Text("LEADERBOARD:");

			for (int i = 0; i < leaderboard.size(); ++i)
			{
				ImGui::Text(leaderboard[i].c_str());
			}

		}
	}
	
}

void ModuleNetworkingClient::onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress)
{
	lastPacketReceivedTime = Time.time;

	ServerMessage message;
	packet >> message;

	if (state == ClientState::WaitingWelcome)
	{
		if (message == ServerMessage::Welcome)
		{
			packet >> playerId;
			packet >> networkId;
			replicationClient.network_id = networkId;
			LOG("ModuleNetworkingClient::onPacketReceived() - Welcome from server");
			state = ClientState::Playing;
		}
		else if (message == ServerMessage::Unwelcome)
		{
			WLOG("ModuleNetworkingClient::onPacketReceived() - Unwelcome from server :-(");
			disconnect();
		}
	}
	else if (state == ClientState::Playing)
	{
		// TODO(jesus): Handle incoming messages from server
		if (message == ServerMessage::Ping)
		{
			leaderboard.clear();
			size_t size;
			packet >> size;
			std::string tmp; 
			for (int i = 0; i < size; ++i)
			{
				packet >> tmp;

				leaderboard.push_back(tmp);
			}

			//GameObject* go = App->modLinkingContext->getNetworkGameObject(networkId);
			//if (go != nullptr)
			//{
				//packet >> go->totalLife;
				//packet >> go->totalKills;
			//}
			lastPacketReceivedTime = Time.time;
		}
		else if (message == ServerMessage::Replication)
		{
			if(deliveryManager.processSequenceNumber(packet))
				replicationClient.Read(packet);
		}
	}
}

void ModuleNetworkingClient::onUpdate()
{
	if (state == ClientState::Stopped) return;

	if (state == ClientState::Start)
	{
		// Send the hello packet with player data

		OutputMemoryStream stream;
		stream << ClientMessage::Hello;
		stream << playerName;
		stream << spaceshipType;

		sendPacket(stream, serverAddress);

		state = ClientState::WaitingWelcome;
	}
	else if (state == ClientState::WaitingWelcome)
	{
	}
	else if (state == ClientState::Playing)
	{
		if (secondsSinceLastPing > PING_INTERVAL_SECONDS)
		{
			secondsSinceLastPing = 0.0f;
			OutputMemoryStream packet;
			packet << ClientMessage::Ping;
			if(deliveryManager.hasSequenceNumbersPendingAck())
				deliveryManager.writeSequenceNumbersPendingAck(packet);
			sendPacket(packet, serverAddress);
		}
		else
			secondsSinceLastPing += Time.deltaTime;

		secondsSinceLastInputDelivery += Time.deltaTime;

		if (inputDataBack - inputDataFront < ArrayCount(inputData))
		{
			uint32 currentInputData = inputDataBack++;
			InputPacketData &inputPacketData = inputData[currentInputData % ArrayCount(inputData)];
			inputPacketData.sequenceNumber = currentInputData;
			inputPacketData.horizontalAxis = Input.horizontalAxis;
			inputPacketData.verticalAxis = Input.verticalAxis;
			inputPacketData.buttonBits = packInputControllerButtons(Input);

			// Create packet (if there's input and the input delivery interval exceeded)
			if (secondsSinceLastInputDelivery > inputDeliveryIntervalSeconds)
			{
				secondsSinceLastInputDelivery = 0.0f;

				OutputMemoryStream packet;
				packet << ClientMessage::Input;

				for (uint32 i = inputDataFront; i < inputDataBack; ++i)
				{
					InputPacketData &inputPacketData = inputData[i % ArrayCount(inputData)];
					packet << inputPacketData.sequenceNumber;
					packet << inputPacketData.horizontalAxis;
					packet << inputPacketData.verticalAxis;
					packet << inputPacketData.buttonBits;
				}

				//Delete this for save all inputs
				// Clear the queue
				//inputDataFront = inputDataBack;

				sendPacket(packet, serverAddress);
			}
			//Sito this is for the movement of game object in local
			GameObject* go = App->modLinkingContext->getNetworkGameObject(networkId);

			if (go != nullptr)
			{
				go->behaviour->onInput(Input,false);
			}
		}
	}

	// Make the camera focus the player game object
	GameObject *playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
	if (playerGameObject != nullptr)
	{
		App->modRender->cameraPosition = playerGameObject->position;
	}

	if (lastPacketReceivedTime < Time.time - DISCONNECT_TIMEOUT_SECONDS)
	{
		disconnect();
	}
}

void ModuleNetworkingClient::onConnectionReset(const sockaddr_in & fromAddress)
{
	disconnect();
}

void ModuleNetworkingClient::onDisconnect()
{
	state = ClientState::Stopped;

	// Get all network objects and clear the linking context
	uint16 networkGameObjectsCount;
	GameObject *networkGameObjects[MAX_NETWORK_OBJECTS] = {};
	App->modLinkingContext->getNetworkGameObjects(networkGameObjects, &networkGameObjectsCount);
	App->modLinkingContext->clear();

	// Destroy all network objects
	for (uint32 i = 0; i < networkGameObjectsCount; ++i)
	{
		Destroy(networkGameObjects[i]);
	}

	App->modRender->cameraPosition = {};
	deliveryManager.clear();

	disconnect();
	
}

void ModuleNetworkingClient::SetLastInput(uint32 last,float angle, vec2 position)
{
	GameObject* go = App->modLinkingContext->getNetworkGameObject(networkId);

	if (angle != 0 || position.x != 0 || position.y != 0)
	{
		for (uint32 i = last; i < inputDataBack; ++i)
		{
			InputPacketData &inputPacketData = inputData[i % ArrayCount(inputData)];

			if (go != nullptr)
			{
				InputController controller;
				controller.horizontalAxis = inputPacketData.horizontalAxis;
				controller.verticalAxis = inputPacketData.verticalAxis;
				unpackInputControllerButtons(inputPacketData.buttonBits, controller);
				go->behaviour->onFakeInput(controller, angle, position);
			}
		}

		if (go->position.x != position.x || go->position.y != position.y)
			go->position = position;
		if (go->angle != angle)
			go->angle = angle;

	}
	inputDataFront = last;
}