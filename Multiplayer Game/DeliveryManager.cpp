#include "Networks.h"
#include "DeliveryDelegate.h"
#include "DeliveryManager.h"
//Delivery manager functions

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet) 
{
	Delivery delivery;

	delivery.dispatchTime = Time.deltaTime;
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

		ret = true;
	}

	return ret;
}

bool DeliveryManager::hasSequenceNumbersPendingAck() const
{
	bool ret = false;

	if (server)
		ret = pendingDeliveries.size() > 0;
	else
		ret = sequenceNumberPendingAck.size() > 0;

	return ret;
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

		for (uint32 i = 0; i < pendingDeliveries.size(); ++i)
		{
			for (uint32 j = start; j < start + range; ++j)
			{
				if (pendingDeliveries.size() > 0)
				{
					if (j == pendingDeliveries[i].sequenceNumber)
					{
						onDeliverySuccess(&pendingDeliveries[i]);
						pendingDeliveries.erase(pendingDeliveries.begin() + i);
						i = 0;
					}
					else if (j < pendingDeliveries[i].sequenceNumber)
					{
						onDeliveryFailure(&pendingDeliveries[i]);
						pendingDeliveries.erase(pendingDeliveries.begin() + i);
						i = 0;
					}
				}
			}
		}

	}

	//clear(start,range);
}

void DeliveryManager::processTimeOutPackets()
{
	if (pendingDeliveries.size() > 0)
	{
		for (std::vector<Delivery>::iterator it = pendingDeliveries.begin(); it != pendingDeliveries.end(); ++it)
		{
			if (it->dispatchTime > timeoutAck)
			{
				onDeliveryFailure(it._Ptr);
				pendingDeliveries.erase(it);

				if (pendingDeliveries.size() > 0)
					it = pendingDeliveries.begin();
				else
					break;
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

//Question, no tengo ni idea
void DeliveryManager::onDeliverySuccess(Delivery* delivery)
{
	//delivery->deliveryDelegate->onDeliverySuccess(this);
}

void DeliveryManager::onDeliveryFailure(Delivery* delivery)
{
	//delivery->deliveryDelegate->onDeliveryFailure(this);
}