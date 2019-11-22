#ifndef _DELIVERYDELEGATE_
#define _DELIVERYDELEGATE_

class DeliveryManager;


class DeliveryDelegate
{
public:
	virtual void onDeliverySuccess(DeliveryManager *deliveryManager) const = 0;
	virtual void onDeliveryFailure(DeliveryManager *deliveryManager) const = 0;
};

struct Delivery
{
	uint32 sequenceNumber = 0;
	double dispatchTime = 0.0;
	DeliveryDelegate* deliveryDelegate = nullptr;
};

#endif // !_DELIVERYDELEGATE_
