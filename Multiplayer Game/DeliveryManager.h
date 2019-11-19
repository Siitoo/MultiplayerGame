#ifndef _DELIVERYMANAGER_
#define _DELIVERYMANAGER_

class DeliveryDelegate;
struct Delivery;

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

	void onDeliverySuccess(Delivery *delivery);
	void onDeliveryFailure(Delivery *delivery);

	bool server = false;

private:
	uint32 nextSequenceNumber = 0;
	std::vector<Delivery> pendingDeliveries;

	uint32 nextExpectedSequenceNumber = 0;
	std::vector<uint32> sequenceNumberPendingAck;

	float timeoutAck = 0.8f;

};






#endif // !_DELIVERYMANAGER_

