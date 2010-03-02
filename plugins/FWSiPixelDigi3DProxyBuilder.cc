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

#include "DataFormats/SiPixelDigi/interface/PixelDigi.h"
#include "DataFormats/Common/interface/DetSetVector.h"

class FWSiPixelDigi3DProxyBuilder : public FW3DDataProxyBuilder
{
public:
  FWSiPixelDigi3DProxyBuilder() {}
  virtual ~FWSiPixelDigi3DProxyBuilder() {}
  REGISTER_PROXYBUILDER_METHODS();

private:
  virtual void build(const FWEventItem* iItem, TEveElementList** product);
  FWSiPixelDigi3DProxyBuilder(const FWSiPixelDigi3DProxyBuilder&);    
  const FWSiPixelDigi3DProxyBuilder& operator=(const FWSiPixelDigi3DProxyBuilder&);
};

void FWSiPixelDigi3DProxyBuilder::build(const FWEventItem* iItem, TEveElementList** product)
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

  const edm::DetSetVector<PixelDigi>* digis = 0;
  iItem->get(digis);

  if( 0 == digis ) 
    return;

  std::vector<TVector3> pixelDigiPoints;

  for ( edm::DetSetVector<PixelDigi>::const_iterator it = digis->begin(), end = digis->end();
        it != end; ++it )
  {
    TEveCompound* compound = new TEveCompound("si pixel digi compound", "siPixelDigis");
    compound->OpenCompound();
    tList->AddElement(compound);

    TEvePointSet* pointSet = new TEvePointSet();
    pointSet->SetMarkerSize(2);
    pointSet->SetMarkerStyle(2);
    pointSet->SetMarkerColor(2);
    compound->AddElement(pointSet);

    edm::DetSet<PixelDigi> ds = *it;
    
    if ( ds.data.size() )       
    {
      const uint32_t& detID = ds.id;
      DetId detid(detID);              
         
      for ( edm::DetSet<PixelDigi>::const_iterator idigi = ds.data.begin(), idigiEnd = ds.data.end();
            idigi != idigiEnd; ++idigi )
      {
        int adc = static_cast<int>((*idigi).adc());
        int row = static_cast<int>((*idigi).row());
        int column = static_cast<int>((*idigi).column());
        int channel = static_cast<int>((*idigi).channel());

        // This method, although called "local" seems to transform
        // the point to global coordinates. See TrackUtils.cc
        TVector3 point;
        fireworks::localSiPixel(point, row, column, detid, iItem);
        pointSet->SetNextPoint(point.x(), point.y(), point.z());
      
      } // end of iteration over digis in range   
    } // is there data?
  } // end of iteration over the DetSetVector
}

REGISTER_FW3DDATAPROXYBUILDER(FWSiPixelDigi3DProxyBuilder,edm::DetSetVector<PixelDigi>,"SiPixelDigi");
