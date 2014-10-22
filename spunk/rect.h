/*****************************************************************************/
/*                                                                           */
/*                                   RECT.H                                  */
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



#ifndef _RECT_H
#define _RECT_H



#include "machine.h"
#include "stream.h"



// flags for centering a rect object
static const u16 cfCenterX    = 0x0001;    // center in X direction
static const u16 cfCenterY    = 0x0002;    // center in Y direction
static const u16 cfCenterAll  = 0x0003;    // center in both directions




/*****************************************************************************/
/*                                class Point                                */
/*****************************************************************************/



// a point in nowhere
class Point: public Streamable {

public:
    i16 X, Y;

    Point ();
    Point (i16 A, i16 B);
    Point (const Point& P);

    virtual void Load (Stream&);
    virtual void Store (Stream&) const;

    Point& operator = (const Point& P);
    Point& operator += (const Point& P);
    Point& operator -= (const Point& P);

    friend inline Point operator + (const Point& P1, const Point& P2);
    friend inline Point operator - (const Point& P1, const Point& P2);
    friend inline int operator == (const Point& P1, const Point& P2);
    friend inline int operator != (const Point& P1, const Point& P2);

};



inline Point::Point ()
{
}



inline Point::Point (i16 A, i16 B) :
    X (A),
    Y (B)
{
}



inline Point::Point (const Point& P):
    X (P.X),
    Y (P.Y)
{
}



inline Point& Point::operator = (const Point& P)
{
    X = P.X;
    Y = P.Y;
    return *this;
}



inline Point& Point::operator += (const Point& P)
{
    X += P.X;
    Y += P.Y;
    return *this;
}



inline Point& Point::operator -= (const Point& P)
{
    X -= P.X;
    Y -= P.Y;
    return *this;
}



inline Point operator + (const Point& P1, const Point& P2)
{
    return Point (i16 (P1.X + P2.X), i16 (P1.Y + P2.Y));
}



inline Point operator - (const Point& P1, const Point& P2)
{
    return Point (i16 (P1.X - P2.X), i16 (P1.Y - P2.Y));
}



inline int operator == (const Point& P1, const Point& P2)
{
    return (P1.X == P2.X && P1.Y == P2.Y);
}



inline int operator != (const Point& P1, const Point& P2)
{
    return (P1.X != P2.X || P1.Y != P2.Y);
}



/*****************************************************************************/
/*                                class Rect                                 */
/*****************************************************************************/



// A rectangle. FYI: all elements of a struct are public by default
class Rect: public Streamable {

public:
    Point A;            // upper left corner
    Point B;            // lower right corner


    // constructors
    Rect ();
    Rect (const Point& Origin, const Point& Size);
    Rect (i16 X1, i16 Y1, i16 X2, i16 Y2);
    Rect (const Rect& R);

    // Stream stuff
    virtual void Load (Stream&);
    virtual void Store (Stream&) const;

    // methods
    void Assign (i16 X1, i16 Y1, i16 X2, i16 Y2);
    void Assign (const Point& Origin, const Point& Size);
    void Move (i16 dX, i16 dY);
    void Grow (i16 dX, i16 dY);
    Point Size () const;
    int XSize () const;
    int YSize () const;
    unsigned long Chars () const;
    int Contains (const Point& P) const;
    int Contains (const Rect& R) const;
    int IsEmpty () const;
    int OutOfRange (i16 MaxX, i16 MaxY) const;
    void Center (const Rect& R, u16 Option);

    Rect& operator = (const Rect& R);

    friend Rect Intersection (const Rect& R1, const Rect& R2);
    friend Rect Union (const Rect& R1, const Rect& R2);
    friend inline int operator == (const Rect& R1, const Rect& R2);
    friend inline int operator != (const Rect& R1, const Rect& R2);

};



inline Rect::Rect ()
{
}



inline Rect::Rect (const Point& Origin, const Point& Size) :
    A (Origin.X, Origin.Y),
    B (i16 (Origin.X + Size.X), i16 (Origin.Y + Size.Y))
{
}



inline Rect::Rect (i16 X1, i16 Y1, i16 X2, i16 Y2) :
    A (X1, Y1),
    B (X2, Y2)
{
}



inline Rect::Rect (const Rect& R):
    A (R.A),
    B (R.B)
{
}



inline void Rect::Assign (const Point& Origin, const Point& Size)
{
    A.X = Origin.X;
    B.X = i16 (Origin.X + Size.X);
    A.Y = Origin.Y;
    B.Y = i16 (Origin.Y + Size.Y);
}



inline void Rect::Assign (i16 X1, i16 Y1, i16 X2, i16 Y2)
{
    A.X = X1;   A.Y = Y1;
    B.X = X2;   B.Y = Y2;
}



inline int Rect::XSize () const
{
    return B.X - A.X;
}



inline int Rect::YSize () const
{
    return B.Y - A.Y;
}



inline Point Rect::Size () const
{
    return Point ((u16) XSize (), (u16) YSize ());
}



inline unsigned long Rect::Chars () const
{
    return ( (unsigned long) (B.X - A.X) * (unsigned long) (B.Y - A.Y) );
}



inline int Rect::Contains (const Point& P) const
{
    return (P.X >= A.X && P.X < B.X && P.Y >= A.Y && P.Y < B.Y);
}



inline int operator == (const Rect& R1, const Rect& R2)
{
    return (R1.A.X == R2.A.X &&
            R1.A.Y == R2.A.Y &&
            R1.B.X == R2.B.X &&
            R1.B.Y == R2.B.Y);
}



inline int operator != (const Rect& R1, const Rect& R2)
{
    return (R1.A.X != R2.A.X ||
            R1.A.Y != R2.A.Y ||
            R1.B.X != R2.B.X ||
            R1.B.Y != R2.B.Y);
}



inline int Rect::IsEmpty () const
{
    return (A.X >= B.X || A.Y >= B.Y);
}



inline void Rect::Grow (i16 dX, i16 dY)
// Change the size of the rectangle as follows:
// A.X -= dX;  B.X += dX;
// A.Y -= dY;  B.Y += dY;
{
    A.X -= dX;  B.X += dX;
    A.Y -= dY;  B.Y += dY;
}



// End of RECT.H

#endif
