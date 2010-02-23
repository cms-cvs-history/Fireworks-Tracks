#include "Fireworks/Core/interface/register_dataproxybuilder_macro.h"
#include "Fireworks/Core/interface/FW3DSimpleProxyBuilderTemplate.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/DetIdToMatrix.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include "TEveCompound.h"
#include "TEvePointSet.h"

class FWTrackingRecHit3DProxyBuilder : public FW3DSimpleProxyBuilderTemplate<TrackingRecHit>
{
public:
   FWTrackingRecHit3DProxyBuilder(void) 
    {}
  
   virtual ~FWTrackingRecHit3DProxyBuilder(void) 
    {}

   REGISTER_PROXYBUILDER_METHODS();

private:
   FWTrackingRecHit3DProxyBuilder(const FWTrackingRecHit3DProxyBuilder&); 			// stop default
   const FWTrackingRecHit3DProxyBuilder& operator=(const FWTrackingRecHit3DProxyBuilder&); 	// stop default

   void build(const TrackingRecHit& iData, unsigned int iIndex, TEveElement& oItemHolder) const;
};

void
FWTrackingRecHit3DProxyBuilder::build(const TrackingRecHit& iData, unsigned int iIndex, TEveElement& oItemHolder) const
{
   // FIXME: This is what I need, but it's not there in the geometry file.
   //
   //      std::vector<TEveVector> corners = item()->getGeom()->getPoints(iData.id());
   //       if( corners.empty() ) {
   //          std::cout << "ERROR: failed get geometry of ECAL rechit with det id: " <<
   //          iData.id() << std::endl;
   //          return;
   //       }
   const TGeoHMatrix* matrix = item()->getGeom()->getMatrix(iData.geographicalId());
   if ( !matrix ) {
     std::cout << "ERROR: failed get geometry of Tracking rechit with det id: " <<
       iData.geographicalId() << std::endl;
     return;
   }	 

   TEvePointSet *rechitSet = new TEvePointSet("Tracking rechit");
   rechitSet->SetMarkerSize(3);
   rechitSet->SetMainColor(item()->defaultDisplayProperties().color());
   rechitSet->SetRnrSelf(item()->defaultDisplayProperties().isVisible());
   rechitSet->SetRnrChildren(item()->defaultDisplayProperties().isVisible());
   
   Double_t localPoint[3] = { iData.localPosition().x(),  iData.localPosition().y(), iData.localPosition().z()};
   Double_t globalPoint[3];
   
   matrix->LocalToMaster(localPoint, globalPoint);

   oItemHolder.AddElement(rechitSet);
}

REGISTER_FW3DDATAPROXYBUILDER(FWTrackingRecHit3DProxyBuilder, TrackingRecHit, "Tracking RecHit");
