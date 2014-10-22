/*****************************************************************************/
/*									     */
/*				    ICEI.CC				     */
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



// This is an interface module to connect other software to ESTIC. All
// important information regarding the Istec is routed through the event
// handler below. Just fill in your code...



#include "event.h"
#include "str.h"

#include "icevents.h"
#include "devstate.h"
#include "iccli.h"



/*****************************************************************************/
/*			    class ExternalInterface			     */
/*****************************************************************************/



class ExternalInterface: public EventHandler {

public:
    virtual void HandleEvent (Event& E);
    // Handle incoming events.

};



void ExternalInterface::HandleEvent (Event& E)
{
    // Switch on the type of event
    switch (E.What) {

	case evInit:
	    // This event is posted if the initialization of ESTIC is
	    // complete. All resources are already valid if this event
	    // comes in. Add your own stuff if you have to.
	    break;

	case evExit:
	    // This event is posted from the applications constructor if
	    // application is shutting down. All resources are still valid
	    // when this event happens.
	    break;

	case evAbort:
	    // This event is posted in an emergency situation if spunk
	    // detected an internal processing error. It is usually *not*
	    // followed by an evExit event.
	    // Beware: The application may be in an unstable state if this
	    // event happens, so you cannot rely on anything. Place critical
	    // cleanup code here but only if needed.
	    break;

	case evIdle:
	    // This event is posted if the application is idle. You cannot
	    // rely on any specific calling frequency, this event may or
	    // may not be posted. But if you have something, that must be
	    // done in a regular manner, do it here...
	    break;

	case evSecondChange:
	    // This event may be posted if the application is idle and a
	    // new second has begun. Used for clocks or other stuff.
	    break;

	case evMinuteChange:
	    // This event may be posted if the application is idle and a
	    // new minute has begun. Used for clocks or other stuff.
	    break;

	case evChargeUpdate:
	    // This event is posted if something is written to the Charges
	    // variable in module iccom. This does not mean that the charges
	    // have really changed, but on the other side, the charges do
	    // not change without posting this event.
	    break;

	case evMatrixChange:
	    // A device had a change in the connection matrix or the dialed
	    // number. E.Info.P points to the DevStateInfo object that had
	    // the change.
	    break;

	case evCallLogMsg:
	    // This event is posted after a call is complete. E.Info.O
	    // contains a pointer to a string with the complete log message.
	    // If you need more specific information, have a look at
	    // evCallComplete.
	    break;

	case evCallComplete:
	    // This event is posted after an external call is complete.
	    // E.Info.O is a pointer to a DevStateInfo object that contains
	    // complete information about the call.
	    break;

	case evIncomingCall:
	    // Posted when the istec detects an incoming call and sends the
	    // CLI message. E.Info.O is a pointer to a CLI object that
	    // contains the calling partys number and more information.
	    break;

    }

}



/*****************************************************************************/
/*				     Data				     */
/*****************************************************************************/



// Declare a static ExternalInterface object to have the event handler called
static ExternalInterface	EI;



