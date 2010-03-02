#include "TEveManager.h"
#include "TEveCompound.h"
#include "TEveGeoNode.h"
#include "TEveStraightLineSet.h"

#include "Fireworks/Tracks/interface/TrackUtils.h"
#include "Fireworks/Core/interface/FW3DDataProxyBuilder.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/BuilderUtils.h"
#include "Fireworks/Core/src/CmsShowMain.h"
#include "Fireworks/Core/src/changeElementAndChildren.h"

#include "DataFormats/SiStripDigi/interface/SiStripDigi.h"
#include "DataFormats/Common/interface/DetSetVector.h"

class FWSiStripDigi3DProxyBuilder : public FW3DDataProxyBuilder
{
public:
  FWSiStripDigi3DProxyBuilder() {}
  virtual ~FWSiStripDigi3DProxyBuilder() {}
  REGISTER_PROXYBUILDER_METHODS();

private:
  virtual void build(const FWEventItem* iItem, TEveElementList** product);
  FWSiStripDigi3DProxyBuilder(const FWSiStripDigi3DProxyBuilder&);    
  const FWSiStripDigi3DProxyBuilder& operator=(const FWSiStripDigi3DProxyBuilder&);
};

void FWSiStripDigi3DProxyBuilder::build(const FWEventItem* iItem, TEveElementList** product)
{
  TEveElementList* tList = *product;

  if( 0 == tList ) 
  {
    tList =  new TEveElementList(iItem->name().c_str(),"SiPixelDigi",true);
    *product = tList;
    tList->SetMainColor(iItem->defaultDisplayProperties().color());
    gEve->AddElement(tList);
  } 
  
  else 
  {
    tList->DestroyElements();
  }

  const edm::DetSetVector<SiStripDigi>* digis = 0;
  iItem->get(digis);

  if( 0 == digis ) 
    return;

  for ( edm::DetSetVector<SiStripDigi>::const_iterator it = digis->begin(), end = digis->end();
        it != end; ++it)     
  {     
    edm::DetSet<SiStripDigi> ds = *it;

    if ( ds.data.size() )   
    {
      const uint32_t& detID = ds.id;
      DetId detid(detID);
    
      if ( iItem->getGeom() )
      {
        const TGeoHMatrix* matrix = iItem->getGeom()->getMatrix(detid);
      }
      
      for ( edm::DetSet<SiStripDigi>::const_iterator idigi = ds.data.begin(), idigiEnd = ds.data.end();
            idigi != idigiEnd; ++idigi )        
      {
        int strip = static_cast<int>((*idigi).strip());
        int adc   = static_cast<int>((*idigi).adc());

        // Fireworks certainly doesn't like rendering all of these and making them pick-able
        // Perhaps we don't care about picking these digis and should just make one point set
        // and not bother with the compound...
        TEveCompound* compound = new TEveCompound("si strip digi compound", "siStripDigis");
        compound->OpenCompound();
        tList->AddElement(compound);

        TEvePointSet* pointSet = new TEvePointSet();
        pointSet->SetMarkerSize(2);
        pointSet->SetMarkerStyle(2);
        pointSet->SetMarkerColor(2);
        compound->AddElement(pointSet);

        // For now, take the center of the strip as the local position 
        const DetIdToMatrix* detIdToGeo = iItem->getGeom();
        const TGeoHMatrix* matrix = detIdToGeo->getMatrix(detid);
        double local[3]  = {0.0, 0.0, 0.0};
        double global[3] = {0.0, 0.0, 0.0};
        matrix->LocalToMaster(local, global);
        pointSet->SetNextPoint(global[0], global[1], global[2]);

      } // end of iteration over digis  
    } // is there data?
  } // end of iteratin over the DetSetVector
}

REGISTER_FW3DDATAPROXYBUILDER(FWSiStripDigi3DProxyBuilder,edm::DetSetVector<SiStripDigi>,"SiStripDigi");