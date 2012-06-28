// -*- C++ -*-
//
// Package:     Tracks
// Class  :     FWTrackProxyBuilder
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Tue Nov 25 14:42:13 EST 2008
// $Id: FWTrackProxyBuilder.cc,v 1.13 2010/11/11 20:25:29 amraktad Exp $
//

// system include files
#include "TEveTrack.h"
#include "TEveGeoShape.h"
#include "TEveVector.h"
#include "TEveStraightLineSet.h"

// user include files
#include "Fireworks/Core/interface/FWSimpleProxyBuilderTemplate.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWMagField.h"
#include "Fireworks/Core/interface/FWGeometry.h"
#include "Fireworks/Tracks/interface/TrackUtils.h"
#include "Fireworks/Tracks/interface/estimate_field.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/SiStripCluster/interface/SiStripCluster.h"

class FWTrackProxyBuilder : public FWSimpleProxyBuilderTemplate<reco::Track> {

public:
   FWTrackProxyBuilder();
   virtual ~FWTrackProxyBuilder();

   REGISTER_PROXYBUILDER_METHODS();

private:
   FWTrackProxyBuilder(const FWTrackProxyBuilder&); // stop default

   const FWTrackProxyBuilder& operator=(const FWTrackProxyBuilder&); // stop default

   void build(const reco::Track& iData, unsigned int iIndex,TEveElement& oItemHolder, const FWViewContext*);
};

FWTrackProxyBuilder::FWTrackProxyBuilder()
{
}

FWTrackProxyBuilder::~FWTrackProxyBuilder()
{
}

void
FWTrackProxyBuilder::build( const reco::Track& iData, unsigned int iIndex,TEveElement& oItemHolder , const FWViewContext*) 
{
   if( context().getField()->getSource() == FWMagField::kNone ) {
      if( fabs( iData.eta() ) < 2.0 && iData.pt() > 0.5 && iData.pt() < 30 ) {
	 double estimate = fw::estimate_field( iData, true );
         if( estimate >= 0 ) context().getField()->guessField( estimate );
      }
   }

   TEveTrackPropagator* propagator = ( !iData.extra().isAvailable() ) ?  context().getTrackerTrackPropagator() : context().getTrackPropagator();


   TEveRecTrack t;
   t.fBeta = 1.;
   t.fSign = iData.charge();
   t.fP.Set(iData.px(), iData.py(), iData.pz());
   t.fV.Set(iData.vx(), iData.vy(), iData.vz());
   TEveTrack* trk = new TEveTrack( &t, propagator );

   if( iData.extra().isAvailable() )
   {
      std::vector<TVector3> points;
      const FWEventItem &iItem = *item();
      fireworks::pushPixelHits( points, iItem, iData );
    
      TEvePointSet* pointSet = new TEvePointSet();
      for( std::vector<TVector3>::const_iterator it = points.begin(), itEnd = points.end(); it != itEnd; ++it )
      {
         pointSet->SetNextPoint( it->x(), it->y(), it->z());
         // !!!
         trk->AddPathMark( TEvePathMark( TEvePathMark::kDaughter, TEveVector(it->x(), it->y(), it->z()) ));
      }
      setupAddElement(pointSet, &oItemHolder);


      // SiStrips      
      // AMT 
      //    int cluCNT = 0;
      for( trackingRecHit_iterator it = iData.recHitsBegin(), itEnd = iData.recHitsEnd(); it != itEnd; ++it )
      {
         unsigned int rawid = (*it)->geographicalId();

         // need shape for plane normal
         TEveGeoShape* shape = item()->getGeom()->getEveShape( rawid );
         setupAddElement(shape,  &oItemHolder);

     
         TEveTrans& trans = shape->RefMainTrans();
         TEveVector stripNormal(trans.GetBaseVec(3).x(), trans.GetBaseVec(3).y(), trans.GetBaseVec(3).z());
	 
         const float* pars = item()->getGeom()->getParameters( rawid );
         const SiStripCluster *cluster = fireworks::extractClusterFromTrackingRecHit( &(*(*it)) );
         if( cluster )
         {
            // printf("cluster %d \n", cluCNT++);
	    short firststrip = cluster->firstStrip();
            float localTop[3] = { 0.0, 0.0, 0.0 };
            float localBottom[3] = { 0.0, 0.0, 0.0 };
	    fireworks::localSiStrip( firststrip, localTop, localBottom, pars, rawid );
	    float globalTop[3];
	    float globalBottom[3];
	    item()->getGeom()->localToGlobal( rawid, localTop, globalTop, localBottom, globalBottom );


	    TEveStraightLineSet *scposition = new TEveStraightLineSet;
            scposition->AddLine( globalTop[0], globalTop[1], globalTop[2],
				 globalBottom[0], globalBottom[1], globalBottom[2] );
            TVector3 bv1 = trans.GetBaseVec(3);
            bv1 *= 50;
            scposition->AddLine( globalTop[0], globalTop[1], globalTop[2],
				 globalBottom[0] + bv1.X(), globalBottom[1]+ bv1.Y(), globalBottom[2] +  bv1.Z() );

            // !!!
            TEveVector stripVector( globalTop[0] - globalBottom[0], globalTop[1] - globalBottom[1],  globalTop[2] - globalBottom[2]);
            stripVector.Normalise();
            TEveVector stripPos(globalTop[0], globalTop[1], globalTop[2]);
            trk->AddPathMark( TEvePathMark( TEvePathMark::kCluster2D, stripPos, stripNormal, stripVector));


            setupAddElement(scposition, &oItemHolder);
            scposition->SetMainColor(kRed);
            scposition->SetPickable(false);
         }


      }
   }
   trk->MakeTrack();
   setupAddElement(trk, &oItemHolder);
}


   REGISTER_FWPROXYBUILDER(FWTrackProxyBuilder, reco::Track, "Tracks", FWViewType::kAll3DBits | FWViewType::kAllRPZBits);
