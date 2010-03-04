// -*- C++ -*-
//
// Package:     Core
// Class  :     FWItemTrackAccessor
//
// Implementation:
//     An example of how to write a plugin based FWItemAccessorBase derived class.
//
// Original Author:  Giulio Eulisse
//         Created:  Thu Feb 18 15:19:44 EDT 2008
// $Id: FWItemTrackAccessors.cc,v 1.1 2010/03/01 09:46:33 eulisse Exp $
//

// system include files
#include <assert.h>
#include "Reflex/Object.h"
#include "TClass.h"

// user include files
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/SiPixelDigi/interface/PixelDigi.h"
#include "DataFormats/SiStripDigi/interface/SiStripDigi.h"
#include "Fireworks/Core/interface/FWItemRandomAccessor.h"

REGISTER_TEMPLATE_FWITEMACCESSOR(FWItemDetSetAccessor<edm::DetSetVector<SiStripDigi> >,edm::DetSetVector<SiStripDigi>, "SiStripDigiCollectionAccessor");
REGISTER_TEMPLATE_FWITEMACCESSOR(FWItemDetSetAccessor<edm::DetSetVector<PixelDigi> >, edm::DetSetVector<PixelDigi>, "SiPixelDigiCollectionAccessor");

