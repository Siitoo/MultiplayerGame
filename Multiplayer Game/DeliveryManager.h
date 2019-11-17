#ifndef _DELIVERYMANAGER_
#define _DELIVERYMANAGER_

class DeliveryDelegate;
struct Delivery
{
	uint32 sequenceNumber = 0;
	double dispatchTime = 0.0;
	DeliveryDelegate *deliveryDelegate = nullptr;
};

class DeliveryManager
{
public:

	Delivery* writeSequenceNumber(OutputMemoryStream& packet);

	bool processSequenceNumber(const InputMemoryStream& packet);

	bool hasSequenceNumbersPendingAck() const;
	void writeSequenceNumbersPendingAck(OutputMemoryStream& packet);

	void processAckdSequenceNumbers(const InputMemoryStream& packet);
	void processTimeOutPackets();

	void clear(uint32 start = 0, uint32 size = 0);

private:
	uint32 nextSequenceNumber = 0;
	std::vector<Delivery> pendingDeliveries;

	//Detect if i'm are a server or client
	bool server = false;

	uint32 nextExpectedSequenceNumber = 0;
	std::vector<uint32> sequenceNumberPendingAck;

	float timeoutAck = 0.8f;

};



class DeliveryDelegate
{
public:
	virtual void onDeliverySuccess(DeliveryManager *deliveryManager) = 0;
	virtual void onDeliveryFailure(DeliveryManager *deliveryManager) = 0;
};




#endif // !_DELIVERYMANAGER_

