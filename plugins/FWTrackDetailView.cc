// -*- C++ -*-
//
// Package:     Calo
// Class  :     FWTrackDetailView
// $Id: FWTrackDetailView.cc,v 1.24 2009/10/12 17:54:06 amraktad Exp $
//

#include "TEveLegoEventHandler.h"

// ROOT includes
#include "TGPack.h"
#include "TEveScene.h"
#include "TEveViewer.h"
#include "TGLViewer.h"
#include "TEveManager.h"

// CMSSW includes
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/EgammaReco/interface/BasicCluster.h"

// Fireworks includes
#include "Fireworks/Core/interface/FWGUISubviewArea.h"
#include "Fireworks/Core/interface/FWModelId.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Tracks/plugins/FWTrackDetailView.h"
#include "Fireworks/Tracks/interface/FWTrackResidualDetailView.h"
#include "Fireworks/Tracks/interface/FWTrackHitsDetailView.h"

//
// constructors and destructor
//
FWTrackDetailView::FWTrackDetailView():
m_hitsView(0)
{
}

FWTrackDetailView::~FWTrackDetailView()
{
   getEveWindow()->DestroyWindow();
   delete m_hitsView;
}

//
// member functions
//
void FWTrackDetailView::build(const FWModelId &id, const reco::Track* iTrack, TEveWindowSlot* base)
{
   if(0==iTrack) return;
   TEveWindowPack* eveWindow = base->MakePack();
   eveWindow->SetShowTitleBar(kFALSE);
   eveWindow->SetHorizontal();
   FWDetailViewBase::setEveWindow(eveWindow);

   TEveWindowSlot* slot;
   ////////////////////////////////////////////////////////////////////////
   //                              Sub-view 1
   ///////////////////////////////////////////////////////////////////////
  
   // prepare window
   {
      slot = eveWindow->NewSlotWithWeight(40);
      FWTrackResidualDetailView builder1;
      builder1.build(id,iTrack,slot);
      FWGUISubviewArea* a = FWGUISubviewArea::getToolBarFromWindow(builder1.getEveWindow());
      a->setSwapIcon(kFALSE);
      a->goingToBeDestroyed_.connect(boost::bind(&FWTrackDetailView::hideWindow, this, _1));
   }
 
   ////////////////////////////////////////////////////////////////////////
   //                              Sub-view 2
   ///////////////////////////////////////////////////////////////////////
   // this view has to  be on heap, since it has to live to 
   // handle action signals
   {
      slot = eveWindow->NewSlotWithWeight(60);
      m_hitsView = new FWTrackHitsDetailView();
      m_hitsView->build(id,iTrack,slot);
      FWGUISubviewArea* a = FWGUISubviewArea::getToolBarFromWindow(m_hitsView->getEveWindow());
      a->setSwapIcon(kFALSE);
      a->goingToBeDestroyed_.connect(boost::bind(&FWTrackDetailView::hideWindow, this, _1));
   }
}

void
FWTrackDetailView::hideWindow(FWGUISubviewArea* sva)
{
   TEveWindow* w = sva->getEveWindow();
   TEveWindowPack* wp = (TEveWindowPack*)getEveWindow();
   TGPack* pack = wp->GetPack();
   pack->HideFrame(w->GetEveFrame());
}

void
FWTrackDetailView::setBackgroundColor(Color_t col)
{
   m_hitsView->setBackgroundColor(col);
}

REGISTER_FWDETAILVIEW(FWTrackDetailView,Track);
