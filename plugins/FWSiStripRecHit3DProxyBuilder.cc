#include "Fireworks/Core/interface/register_dataproxybuilder_macro.h"
#include "Fireworks/Core/interface/FW3DSimpleProxyBuilderTemplate.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/DetIdToMatrix.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2D.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2DCollection.h"
#include "TEveManager.h"
#include "TEveElement.h"
#include "TEveCompound.h"
#include "TEvePointSet.h"

class FWSiStripRecHit2DRecHit3DProxyBuilder : public FW3DDataProxyBuilder
{
public:
   FWSiStripRecHit2DRecHit3DProxyBuilder(void) 
    {}
  
   virtual ~FWSiStripRecHit2DRecHit3DProxyBuilder(void) 
    {}

   REGISTER_PROXYBUILDER_METHODS();

private:
   virtual void build(const FWEventItem* iItem,
                      TEveElementList** product);

   FWSiStripRecHit2DRecHit3DProxyBuilder(const FWSiStripRecHit2DRecHit3DProxyBuilder&); 			// stop default
   const FWSiStripRecHit2DRecHit3DProxyBuilder& operator=(const FWSiStripRecHit2DRecHit3DProxyBuilder&); 	// stop default
};

void
FWSiStripRecHit2DRecHit3DProxyBuilder::build(const FWEventItem* iItem, TEveElementList** product)
{
   TEveElementList* tList = *product;

   if(0 == tList) {
      tList =  new TEveElementList(iItem->name().c_str(), "siStripRechits", true);
      *product = tList;
      tList->SetMainColor(iItem->defaultDisplayProperties().color());
      gEve->AddElement(tList);
   } else {
      tList->DestroyElements();
   }

   const SiStripRecHit2DCollection* collection = 0;
   iItem->get(collection);

   if(0 == collection)
   {
      return;
   }
   TEveCompound* compund = new TEveCompound("siStrip compound", "siStripRechits");
   compund->OpenCompound();

   TEvePointSet *rechitSet = new TEvePointSet("SiStripRecHit2D rechit");
   rechitSet->SetMarkerSize(3);
   rechitSet->SetMainColor(item()->defaultDisplayProperties().color());
   rechitSet->SetRnrSelf(item()->defaultDisplayProperties().isVisible());
   rechitSet->SetRnrChildren(item()->defaultDisplayProperties().isVisible());

   for(SiStripRecHit2DCollection::DataContainer::const_iterator it = collection->data().begin(), itEnd = collection->data().end();
       it != itEnd; ++it)
   {
      const TGeoHMatrix* matrix = item()->getGeom()->getMatrix((*it).geographicalId());
      if ( !matrix ) {
	 std::cout << "ERROR: failed get geometry of SiStripRecHit2D rechit with det id: " <<
	   (*it).geographicalId() << std::endl;
	 return;
      }
   
      Double_t localPoint[3] = { (*it).localPosition().x(), (*it).localPosition().y(), (*it).localPosition().z()};
      Double_t globalPoint[3];
   
      matrix->LocalToMaster(localPoint, globalPoint);
      rechitSet->SetNextPoint(globalPoint[0], globalPoint[1], globalPoint[2]);
   }
   tList->AddElement(compund);
}

REGISTER_FW3DDATAPROXYBUILDER(FWSiStripRecHit2DRecHit3DProxyBuilder, SiStripRecHit2DCollection, "SiStripRecHit2D RecHit");
