/*****************************************************************************/
/*                                                                           */
/*                                ICINTCON.CC                                */
/*                                                                           */
/* (C) 1996     Ullrich von Bassewitz                                        */
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



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



// The diag data for tone dial is somewhat weird: When using pulse dial, the
// diag message from the istec contains the device number. When using tone
// dial, the number of the internal connection is transmitted instead. To
// show the device number in both cases, we hold the device that uses an
// internal connection in the array below
unsigned IntCon [3];



