#ifndef _DELIVERYDELEGATE_
#define _DELIVERYDELEGATE_

class DeliveryManager;


class DeliveryDelegate
{
public:
	virtual void onDeliverySuccess(DeliveryManager *deliveryManager) const = 0;
	virtual void onDeliveryFailure(DeliveryManager *deliveryManager) const = 0;
};

typedef std::shared_ptr<DeliveryDelegate> DeliveryDelegatePtr;

struct Delivery
{
	uint32 sequenceNumber = 0;
	double dispatchTime = 0.0;
	DeliveryDelegatePtr deliveryDelegate;
};

#endif // !_DELIVERYDELEGATE_
