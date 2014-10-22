/*****************************************************************************/
/*                                                                           */
/*                                   RECT.CC                                 */
/*                                                                           */
/* (C) 1993-96  Ullrich von Bassewitz                                        */
/*              Wacholderweg 14                                              */
/*              D-70597 Stuttgart                                            */
/* EMail:       uz@ibb.schwaben.com                                          */
/*                                                                           */
/*****************************************************************************/



// $Id$
//
// $Log$
//
//



#include "machine.h"
#include "rect.h"



/*****************************************************************************/
/*                                class Point                                */
/*****************************************************************************/



void Point::Load (Stream& S)
{
    S >> X >> Y;
}



void Point::Store (Stream& S) const
{
    S << X << Y;
}



/*****************************************************************************/
/*                                class Rect                                 */
/*****************************************************************************/



void Rect::Load (Stream& S)
{
    S >> A >> B;
}



void Rect::Store (Stream& S) const
{
    S << A << B;
}



int Rect::Contains (const Rect& R) const
{
    return (R.A.X >= A.X && R.A.Y >= A.Y && R.B.X <= B.X && R.B.Y <= B.Y);
}



void Rect::Move (i16 dX, i16 dY)
// Change the position of the rectangle as follows:
// A.X += dX;  B.X += dX;
// A.Y += dY;  B.Y += dY;
// If one of the resulting values is less than 0, the rectangle is moved
// so that this value gets 0.
{
    i16 Min;

    A.X += dX;
    B.X += dX;
    A.Y += dY;
    B.Y += dY;

    Min = (A.X < B.X) ? A.X : B.X;
    if (Min < 0) {
        A.X -= Min;
        B.X -= Min;
    }

    Min = (A.Y < B.Y) ? A.Y : B.Y;
    if (Min < 0) {
        A.Y -= Min;
        B.Y -= Min;
    }
}



void Rect::Center (const Rect& R, u16 Option)
{
    // Calculate Size of surrounding rectangle
    Point S = Size ();

    // Center in X
    if (Option & cfCenterX) {
        // BC++ issues a warning if the cast to i16 is missing
        A.X = i16 (R.A.X + ((R.B.X - R.A.X - S.X) / 2));
        B.X = i16 (A.X + S.X);
    }

    // Center in Y
    if (Option & cfCenterY) {
        A.Y = i16 (R.A.Y + ((R.B.Y - R.A.Y - S.Y) / 2));
        B.Y = i16 (A.Y + S.Y);
    }
}



Rect Intersection (const Rect& R1, const Rect& R2)
{
    return Rect (R1.A.X > R2.A.X ? R1.A.X : R2.A.X,
                 R1.A.Y > R2.A.Y ? R1.A.Y : R2.A.Y,
                 R1.B.X < R2.B.X ? R1.B.X : R2.B.X,
                 R1.B.Y < R2.B.Y ? R1.B.Y : R2.B.Y);
}



Rect Union (const Rect& R1, const Rect& R2)
{
    return Rect (R1.A.X < R2.A.X ? R1.A.X : R2.A.X,
                 R1.A.Y < R2.A.Y ? R1.A.Y : R2.A.Y,
                 R1.B.X > R2.B.X ? R1.B.X : R2.B.X,
                 R1.B.Y > R2.B.Y ? R1.B.Y : R2.B.Y);
}



int Rect::OutOfRange (i16 MaxX, i16 MaxY) const
{
    return (A.X < 0 || A.X > MaxX ||
            B.X < 0 || B.X > MaxX ||
            A.Y < 0 || A.Y > MaxY ||
            B.Y < 0 || B.Y > MaxY);
}



Rect& Rect::operator = (const Rect& R)
{
    A = R.A;
    B = R.B;
    return *this;
}



