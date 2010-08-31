
// -*- C++ -*-
//
// Package:     Tracks
// Class  :     FWSiPixelClusterProxyBuilder
//
//
// Original Author:
//         Created:  Thu Dec  6 18:01:21 PST 2007
// $Id: FWSiPixelClusterProxyBuilder.cc,v 1.14 2010/08/23 15:26:40 yana Exp $
//

#include "TEvePointSet.h"

#include "Fireworks/Core/interface/FWProxyBuilderBase.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/DetIdToMatrix.h"
#include "Fireworks/Core/interface/fwLog.h"
#include "Fireworks/Tracks/interface/TrackUtils.h"

#include "DataFormats/SiPixelCluster/interface/SiPixelCluster.h"

class FWSiPixelClusterProxyBuilder : public FWProxyBuilderBase
{
public:
  FWSiPixelClusterProxyBuilder( void ) {}
  virtual ~FWSiPixelClusterProxyBuilder( void ) {}

  REGISTER_PROXYBUILDER_METHODS();

private:
  // Disable default copy constructor
  FWSiPixelClusterProxyBuilder( const FWSiPixelClusterProxyBuilder& );
  // Disable default assignment operator
  const FWSiPixelClusterProxyBuilder& operator=( const FWSiPixelClusterProxyBuilder& );

  virtual void build( const FWEventItem* iItem, TEveElementList* product, const FWViewContext* );
};

void
FWSiPixelClusterProxyBuilder::build( const FWEventItem* iItem, TEveElementList* product , const FWViewContext* )
{
  const SiPixelClusterCollectionNew* pixels = 0;
  
  iItem->get( pixels );
  
  if( ! pixels ) 
  {    
    fwLog( fwlog::kWarning ) << "failed get SiPixelDigis" << std::endl;
    return;
  }

  for( SiPixelClusterCollectionNew::const_iterator set = pixels->begin(), setEnd = pixels->end();
       set != setEnd; ++set ) 
  {    
    unsigned int id = set->detId();

    const DetIdToMatrix *geom = iItem->getGeom();
    const TGeoMatrix* matrix = geom->getMatrix( id );
    const float* pars = geom->getParameters( id );

    const edmNew::DetSet<SiPixelCluster> & clusters = *set;
      
    for( edmNew::DetSet<SiPixelCluster>::const_iterator itc = clusters.begin(), edc = clusters.end(); 
         itc != edc; ++itc ) 
    {
      TEvePointSet* pointSet = new TEvePointSet;
      setupAddElement( pointSet, product );
      
      if( ! matrix || pars == 0 ) 
      {
	fwLog( fwlog::kWarning ) 
	  << "failed get geometry of SiPixelCluster with detid: "
	  << id << std::endl;
	continue;
      }

      double localPoint[3] = 
        {     
          fireworks::pixelLocalX(( *itc ).minPixelRow(), ( int )pars[0] ),
	  fireworks::pixelLocalY(( *itc ).minPixelCol(), ( int )pars[1] ),
	  0.0
        };

      double globalPoint[3];
        
      matrix->LocalToMaster( localPoint, globalPoint );
      
      pointSet->SetNextPoint( globalPoint[0], globalPoint[1], globalPoint[2] );
    }
  }    
}

REGISTER_FWPROXYBUILDER( FWSiPixelClusterProxyBuilder, SiPixelClusterCollectionNew, "SiPixelCluster", FWViewType::kAll3DBits | FWViewType::kAllRPZBits );
