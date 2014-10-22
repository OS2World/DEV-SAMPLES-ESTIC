/*****************************************************************************/
/*									     */
/*				     EVENT.H				     */
/*									     */
/* (C) 1996	Ullrich von Bassewitz					     */
/*		Wacholderweg 14						     */
/*		D-70597 Stuttgart					     */
/* EMail:	uz@ibb.schwaben.com					     */
/*									     */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#ifndef _EVENT_H
#define _EVENT_H



#include "machine.h"
#include "object.h"
#include "circbuf.h"
#include "listnode.h"



/*****************************************************************************/
/*				  class Event				     */
/*****************************************************************************/



class Event: public Object {

public:
    unsigned		What;		// Which event is it?
    struct {				// Additional event info
	unsigned long	U;
	double		F;
	void*		P;		// Destructor will *not* free P
	Object*		O;		// Destrcutor *will* free O
    } Info;
    int			Handled;	// True if event is handled

public:
    Event (unsigned aWhat);
    Event (unsigned aWhat, unsigned long InfoData);
    Event (unsigned aWhat, void* InfoData);
    Event (unsigned aWhat, Object* InfoData);

    virtual ~Event ();

};



/*****************************************************************************/
/*			       class EventQueue				     */
/*****************************************************************************/



class EventQueue: public CircularBuffer<class Event*, 32> {

};



/*****************************************************************************/
/*			      class EventHandler			     */
/*****************************************************************************/



class EventHandler {

    friend class Program;		// Must be a friend to deliver events

private:
    static ListNode<EventHandler>*	EventHandlerList;
    ListNode<EventHandler>		EventHandlerNode;

public:
    EventHandler ();
    // Establish the event handler

    virtual ~EventHandler ();
    // Delete the event handler

    virtual void HandleEvent (Event& E);
    // Handle an incoming event. Default is to do nothing.
};



// End of EVENT.H

#endif
