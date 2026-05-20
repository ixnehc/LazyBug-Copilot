#include "stdh.h"
#include "Event.h"

CEvent::CEvent()
{
	_id=0;
}

CEvent::~CEvent()
{
	_slot.Cleanup();
}
 
void CEvent::Subscribe(const Subscriber& slot)
{
	_slot = slot;
}

void CEvent::operator()(WindowEventArgs& args)
{
	args.handled |= _slot(args);
}