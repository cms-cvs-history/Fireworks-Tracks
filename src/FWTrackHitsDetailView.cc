// ROOT includes
#include "TLatex.h"
#include "TEveCalo.h"
#include "TEveStraightLineSet.h"
#include "TEvePointSet.h"
#include "TEveScene.h"
#include "TEveViewer.h"
#include "TGLViewer.h"
#include "TEveManager.h"
#include "TCanvas.h" 
#include "TEveCaloLegoOverlay.h"
#include "TRootEmbeddedCanvas.h"
#include "TEveLegoEventHandler.h"
#include "TEveTrack.h"

// CMSSW includes
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

// Fireworks includes
#include "Fireworks/Core/interface/FWModelId.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWDetailView.h"
#include "Fireworks/Core/interface/DetIdToMatrix.h"
#include "Fireworks/Tracks/interface/FWTrackHitsDetailView.h"
#include "Fireworks/Tracks/plugins/TracksRecHitsUtil.h"
#include "Fireworks/Core/src/CmsShowMain.h"
#include "Fireworks/Tracks/interface/prepareTrack.h"
#include "Fireworks/Tracks/interface/CmsMagField.h"

FWTrackHitsDetailView::FWTrackHitsDetailView ()
{
}

FWTrackHitsDetailView::~FWTrackHitsDetailView ()
{
}

void
FWTrackHitsDetailView::build (const FWModelId &id, const reco::Track* track, TEveWindowSlot* slot)
{
   TEveScene*      scene(0);
   TEveViewer*     viewer(0);
   TGVerticalFrame* ediFrame(0);

   TEveWindow* ew = FWDetailViewBase::makePackViewer(slot, ediFrame, viewer, scene);
   ew->SetElementName("Track hit view");
   FWDetailViewBase::setEveWindow(ew);

   TracksRecHitsUtil::addHits(*track, id.item(), scene);
   CmsMagField* cmsMagField = new CmsMagField;
   cmsMagField->setReverseState( true );
   cmsMagField->setMagnetState( CmsShowMain::getMagneticField() > 0 );

   TEveTrackPropagator* propagator = new TEveTrackPropagator;
   propagator->SetMagFieldObj( cmsMagField );
   propagator->SetStepper(TEveTrackPropagator::kRungeKutta);
   propagator->SetStepper(TEveTrackPropagator::kRungeKutta);
   propagator->SetMaxR(123);
   propagator->SetMaxZ(300);
   TEveTrack* trk = fireworks::prepareTrack( *track, propagator, id.item()->defaultDisplayProperties().color() );
   trk->MakeTrack();
   scene->AddElement(trk);

   scene->Repaint(true);
   viewer->GetGLViewer()->UpdateScene();
   viewer->GetGLViewer()->CurrentCamera().Reset();
   viewer->GetGLViewer()->RequestDraw(TGLRnrCtx::kLODHigh);
   gEve->Redraw3D();
}
REGISTER_FWDETAILVIEW(FWTrackHitsDetailView);
