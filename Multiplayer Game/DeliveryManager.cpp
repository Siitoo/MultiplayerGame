#include "Networks.h"
#include "DeliveryManager.h"

//Delivery manager functions

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet) 
{
	Delivery delivery;

	delivery.sequenceNumber = nextSequenceNumber++;

	packet << delivery.sequenceNumber;

	if (server)
	{
		pendingDeliveries.push_back(delivery);
		return &delivery;
	}
	else
		return nullptr;

}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
	bool ret = false;

	uint32 sequenceNumber = 0;
	packet >> sequenceNumber;

	if (sequenceNumber == nextExpectedSequenceNumber)
	{
		nextExpectedSequenceNumber = sequenceNumber +1;

		if (!server)
			sequenceNumberPendingAck.push_back(sequenceNumber);
	}

	return ret;
}

bool DeliveryManager::hasSequenceNumbersPendingAck() const
{
	return sequenceNumberPendingAck.size() > 0;
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
	uint32 start = sequenceNumberPendingAck[0];
	bool AckSize = sequenceNumberPendingAck.size() > 1;
	
	packet << start;
	packet << AckSize;

	if(AckSize)
	{
		uint32 range = sequenceNumberPendingAck.size();
		packet << range - 1;
	}

	clear();

}

void DeliveryManager::processAckdSequenceNumbers(const InputMemoryStream& packet)
{
	uint32 start;
	packet >> start;
	bool AckSize;
	packet >> AckSize;
	int32 range = 1;

	if (AckSize)
	{
		packet >> range;
	}

	clear(start,range);
}

void DeliveryManager::processTimeOutPackets()
{
	if (pendingDeliveries.size() > 0)
	{
		for (std::vector<Delivery>::iterator it = pendingDeliveries.begin(); it != pendingDeliveries.end(); ++it)
		{
			if (it->dispatchTime > timeoutAck)
			{
				pendingDeliveries.erase(it);
			}
			else
			{
				it->dispatchTime += Time.deltaTime;
			}
		}
	}


}

void DeliveryManager::clear(uint32 start,uint32 size)
{
	if (server)
	{
		for (std::vector<Delivery>::iterator it = pendingDeliveries.begin(); it != pendingDeliveries.end(); ++it)
		{
			if (it->sequenceNumber == start)
			{
				for (uint32 i = 0; i < size; ++i)
				{
					//(*it)->deliveryDelegate->onDeliverySuccess();

					//Sito i don't know if need increment or not it++
					pendingDeliveries.erase(it);
					it++;
				}
				pendingDeliveries.shrink_to_fit();
				break;
			}
		}


	}
	else
	{
		sequenceNumberPendingAck.clear();
	}
}

