#ifndef Fireworks_Tracks_CmsMagField_h
#define Fireworks_Tracks_CmsMagField_h
// -*- C++ -*-
// 
// Simplified model of the CMS detector magnetic field
// 
// $id$
#include "TEveTrackPropagator.h"

class CmsMagField: public TEveMagField
{
  bool magnetIsOn;
  bool reverse;
 public:
 CmsMagField():
  magnetIsOn(true),
    reverse(false) {}
  virtual ~CmsMagField(){}
  virtual TEveVector GetField(Float_t x, Float_t y, Float_t z) const;
  virtual Float_t    GetMaxFieldMag() const { return magnetIsOn ? 3.8 : 0.0; }
  void               setMagnetState( bool state ){ magnetIsOn = state; }
  bool               isMagnetOn(){ return magnetIsOn;}
  void               setReverseState( bool state ){ reverse = state; }
  bool               isReverse(){ return reverse;}
};

#endif
