/*****************************************************************************/
/*									     */
/*				    OBJECT.H				     */
/*									     */
/* (C) 1993,94	Ullrich von Bassewitz					     */
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



#ifndef __OBJECT_H
#define __OBJECT_H



#include <stdlib.h>



/*****************************************************************************/
/*				 class Object				     */
/*****************************************************************************/



class	Object {

private:
    // Hide the copy constructor and the '=' operator to discourage
    // creating one instance from another. If needed, you have to override
    // this.
    Object (const Object&);
    Object& operator = (const Object&);

public:
    Object ();
    virtual ~Object ();

};



inline Object::Object ()
{
}



// End of OBJECT.H

#endif
