/*****************************************************************************/
/*									     */
/*				      MSG.H				     */
/*									     */
/* (C) 1993,94	Ullrich von Bassewitz					     */
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



#ifndef _MSG_H
#define _MSG_H



#include "machine.h"
#include "object.h"
#include "strmable.h"
#include "stream.h"
#include "str.h"




/*****************************************************************************/
/*				   class Msg				     */
/*****************************************************************************/




class Msg : public String {

    friend class MsgCollection;

private:
    u16		MsgNum;

public:
    Msg (u16 Num, u16 Size = 0);
    Msg (StreamableInit);
    Msg (u16 Num, const char* S);
    Msg (const Msg& M);
    Msg (u16 Num, const String& M);

    // Derived from class Streamable
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;
    virtual u16 StreamableID () const;
    static Streamable* Build ();

    // New member functions
    u16 GetNum () const;
    void SetNum (u16 Num);

    Msg& operator = (const Msg& S);

};



inline Msg::Msg (u16 Num, u16 Size) :
    String (Size), MsgNum (Num)
{
}



inline Msg::Msg (StreamableInit) :
    String (Empty)
{
}



inline Msg::Msg (u16 Num, const char* S) :
    String (S), MsgNum (Num)
{
}



inline Msg::Msg (const Msg& M) :
	String (M), MsgNum (M.MsgNum)
{
}



inline Msg::Msg (u16 Num, const String& M) :
	String (M), MsgNum (Num)
{
}



inline u16 Msg::GetNum () const
{
    return MsgNum;
}



inline void Msg::SetNum (u16 Num)
{
    MsgNum = Num;
}



// End of MSG.H

#endif


