/*****************************************************************************/
/*									     */
/*				     EVENT.CC				     */
/*									     */
/* (C) 1996	Ullrich von Bassewitz					     */
/*		Zwehrenbuehlstrasse 33					     */
/*		D-72070 Tuebingen					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#include "event.h"



/*****************************************************************************/
/*			Explicit template instantiation			     */
/*****************************************************************************/



#ifdef EXPLICIT_TEMPLATES
template class CircularBuffer<Event*, 32>;
template class ListNode<EventHandler>;
#endif



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// The root of the event handler list. Don't try to replace this by a
// listnode item, it simplifies the code but unfortunately does not work!
ListNode<EventHandler>* EventHandler::EventHandlerList;



/*****************************************************************************/
/*				  class Event				     */
/*****************************************************************************/



Event::Event (unsigned aWhat):
    What (aWhat),
    Handled (0)
{
    // Clear the object pointer since the destructor will try to delete it
    Info.O = 0;
}



Event::Event (unsigned aWhat, unsigned long InfoData):
    What (aWhat),
    Handled (0)
{
    Info.O = 0;
    Info.U = InfoData;
}



Event::Event (unsigned aWhat, void* InfoData):
    What (aWhat),
    Handled (0)
{
    Info.O = 0;
    Info.P = InfoData;
}



Event::Event (unsigned aWhat, Object* InfoData):
    What (aWhat),
    Handled (0)
{
    Info.O = InfoData;
}



Event::~Event ()
{
    delete Info.O;
}



/*****************************************************************************/
/*			      class EventHandler			     */
/*****************************************************************************/



EventHandler::EventHandler ():
    EventHandlerNode (this)
// Establish the event handler
{
    // Insert this handler into the list of handlers
    if (EventHandlerList) {
	EventHandlerNode.InsertAfter (EventHandlerList);
    } else {
	EventHandlerList = &EventHandlerNode;
    }
}



EventHandler::~EventHandler ()
// Delete the event handler
{
    // Unlink the node, but be shure to correct the list pointer
    if (EventHandlerList == &EventHandlerNode) {
	// The list pointer points to this node
	if (EventHandlerNode.IsEmpty ()) {
	    // The list is empty
	    EventHandlerList = NULL;
	} else {
	    EventHandlerList = EventHandlerNode.Next ();
	    // Unlink is done by the destructor
	}
    }
}



void EventHandler::HandleEvent (Event& /*E*/)
// Handle an incoming event. Default is to do nothing.
{
}



