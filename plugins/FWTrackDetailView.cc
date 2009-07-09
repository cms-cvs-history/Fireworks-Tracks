#include "TEveElement.h"
#include "TEveGeoNode.h"
#include "TEveManager.h"
#include "TEvePointSet.h"
#include "TEveStraightLineSet.h"
#include "TEveTrack.h"
#include "TEveBoxSet.h"
#include "TEveTrackPropagator.h"
#include "TGLViewer.h"
#include "TGeoMatrix.h"
#include "TVector3.h"
#include "TGLUtil.h"
#include <TH2.h>
#include <TBox.h>
#include <TLine.h>
#include <TLatex.h>
#include <TPaveText.h>
#include <TCanvas.h>

#include "DataFormats/TrackReco/interface/Track.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/HitPattern.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"

#include "Fireworks/Core/interface/FWDetailView.h"

#include "Fireworks/Core/interface/DetIdToMatrix.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWModelId.h"
#include "Fireworks/Core/interface/FWDetailView.h"

using reco::Track;
using reco::TrackBase;
using reco::HitPattern;
using reco::TrackResiduals;

class FWTrackDetailView : public FWDetailView<reco::Track> {

public:
   FWTrackDetailView();
   virtual ~FWTrackDetailView();

   virtual TEveElement* build (const FWModelId &id,const reco::Track*);

protected:
   void getCenter( Double_t* vars )
   {
      vars[0] = rotationCenter()[0];
      vars[1] = rotationCenter()[1];
      vars[2] = rotationCenter()[2];
   }
   double getSignedResidual (const DetIdToMatrix *detIdToGeo, unsigned int id, double resX);

private:
   FWTrackDetailView(const FWTrackDetailView&); // stop default
   const FWTrackDetailView& operator=(const FWTrackDetailView&); // stop default

   static void drawBox(Float_t* pos, Color_t fillCol, Int_t fillType, bool bg=kTRUE);

   // ---------- member data --------------------------------
   void resetCenter() {
      rotationCenter()[0] = 0;
      rotationCenter()[1] = 0;
      rotationCenter()[2] = 0;
   }
};


FWTrackDetailView::FWTrackDetailView ()
{
   setUseGL(kFALSE);
}

FWTrackDetailView::~FWTrackDetailView ()
{
}


Int_t   resXFill = 3007;
Int_t   resYFill = 3006;
Int_t   stereoFill = 3004;
Int_t   invalidFill = 3001;

//______________________________________________________________________________
void FWTrackDetailView::drawBox(Float_t *pos, Color_t fillCol, Int_t fillType, bool bg)
{
   // background color
   if (bg)
   {
      TBox *b1 = new TBox(pos[0], pos[1], pos[2], pos[3]);
      b1->SetFillColor(fillCol);
      b1->Draw();
   }

   // fill style
   TBox *b2 = new TBox(pos[0], pos[1], pos[2], pos[3]);
   b2->SetFillStyle(fillType);
   b2->SetFillColor(kBlack);
   b2->Draw();

   //outline
   TBox *b3 = new TBox(pos[0], pos[1], pos[2], pos[3]);
   b3->SetFillStyle(0);
   b3->SetFillColor(1);
   b3->SetLineWidth(2);
   b3->Draw();
}

//______________________________________________________________________________
double FWTrackDetailView::getSignedResidual (const DetIdToMatrix *detIdToGeo, unsigned int id, double resX)
{
     double local1[3] = { 0, 0, 0 };
     double local2[3] = { resX, 0, 0 };
     double global1[3], global2[3];
     const TGeoHMatrix *m = detIdToGeo->getMatrix(id);
     assert(m != 0);
     m->LocalToMaster(local1, global1);
     m->LocalToMaster(local2, global2);
     TVector3 g1 = global1;
     TVector3 g2 = global2;
     if (g2.DeltaPhi(g1) > 0)
	  return resX;
     else return -resX;
}

//______________________________________________________________________________
TEveElement* FWTrackDetailView::build (const FWModelId &id, const reco::Track* track)
{
   //      following table pasted from HitPattern.h
   //
   //      +-----+-----+-----+-----+-----+-----+-----+-----+----------------+-----+-----+
   //      |tk/mu|  sub-structure  |   sub-sub-structure   |     stereo     |  hit type |
   //      +-----+-----+-----+-----+-----+-----+-----+-----+----------------+-----+-----+
   //      | 10  |   9    8     7  |   6    5     4     3  |        2       |  1     0  | bit
   //
   //      |tk = 1      PXB = 1            layer = 1-3                       hit type = 0-3
   //      |tk = 1      PXF = 2            disk  = 1-2                       hit type = 0-3
   //      |tk = 1      TIB = 3            layer = 1-4      0=rphi,1=stereo  hit type = 0-3
   //      |tk = 1      TID = 4            wheel = 1-3      0=rphi,1=stereo  hit type = 0-3
   //      |tk = 1      TOB = 5            layer = 1-6      0=rphi,1=stereo  hit type = 0-3
   //      |tk = 1      TEC = 6            wheel = 1-9      0=rphi,1=stereo  hit type = 0-3
   //      |mu = 0      DT  = 1            layer                             hit type = 0-3
   //      |mu = 0      CSC = 2            layer                             hit type = 0-3
   //      |mu = 0      RPC = 3            layer                             hit type = 0-3
   //
   //      hit type, see DataFormats/TrackingRecHit/interface/TrackingRecHit.h
   //      valid    = valid hit                                     = 0
   //      missing  = detector is good, but no rec hit found        = 1
   //      inactive = detector is off, so there was no hope         = 2
   //      bad      = there were many bad strips within the ellipse = 3
   //

   bool debug = kFALSE;

   Color_t resXCol   = kGreen-9;
   Color_t resYCol   = kWhite;
   Color_t stereoCol = kCyan-9;
   Color_t invalidCol = kRed;

   int nhits;
   float res[2][64];
   int hittype[64];
   int stereo[64];
   int substruct[64];
   int subsubstruct[64];
   int detector[64];

   const static char* det_tracker_str[6]={"PXB","PXF","TIB","TID","TOB","TEC"};
   HitPattern hitpat = track->hitPattern();
   TrackResiduals residuals = track->residuals();

   const DetIdToMatrix *detIdToGeo = id.item()->getGeom();
   assert(detIdToGeo != 0);
   nhits=hitpat.numberOfHits();
   for (int i = 0; i < nhits; ++i) {
      //   	printf("there are %d hits in the pattern, %d in the vector, this is %u\n",
      //   	       nhits, track->recHitsEnd() - track->recHitsBegin(), (*(track->recHitsBegin() + i))->geographicalId().rawId());
      hittype[i] = 0x3 & hitpat.getHitPattern(i);
      stereo[i] = 0x1 & hitpat.getHitPattern(i) >> 2;
      subsubstruct[i] = 0xf & hitpat.getHitPattern(i) >> 3;
      substruct[i] = 0x7 & hitpat.getHitPattern(i) >> 7;
      detector[i] = 0x01 & hitpat.getHitPattern(i) >> 10;
      if ((*(track->recHitsBegin() + i))->isValid()) {
         res[0][i] = getSignedResidual(detIdToGeo,
                                       (*(track->recHitsBegin() + i))->geographicalId().rawId(),
                                       residuals.residualX(i, hitpat));
      } else {
         res[0][i] = 0;
      }
      res[1][i] = residuals.residualY(i, hitpat);
      if (debug) printf("%s, %i\n",det_tracker_str[substruct[i]-1],subsubstruct[i]);
   }

   int ndet=0;
   int det[64];
   det[0]=0;
   for(int j=0; j < nhits-1;) {
      int k=j+1;
      for(; k<nhits ; k++) {
         if(substruct[j]==substruct[k]  && subsubstruct[j]==subsubstruct[k]) {
            if(k==(nhits-1)) j=k;
         }
         else {
            ndet++;
            j=k;
            det[ndet]=j;
            break;
         }
      }
   }
   ndet++;
   det[ndet]=nhits;

   if (debug)
   {
      for(int i=0; i<ndet; i++)
      {
         std::cout <<"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
         std::cout << "idx " << i << " det[idx] " <<  det[i] << std::endl;
         std::cout << "det idx " << det[i] <<   std::endl;
         std::cout << "det_tracker_str idx " << substruct[det[i]]-1  << std::endl;
         printf("det[idx] %i det_tracker_str %s substruct %i\n",det[i], det_tracker_str[substruct[det[i]]-1], subsubstruct[det[i]]);
      }
   }
   //______________________________________________________________________________
   // text canvas

   char mytext[256];
   int nvalid=0;
   int npix=0;
   int nstrip=0;
   for(int i=0; i<nhits; i++) {
      if(hittype[i]==0) nvalid++;
      if(substruct[i]<3) npix++;
      else nstrip++;
   }

   textCanvas()->cd();

   TLatex* latex = new TLatex(0.02, 0.970, "");
   latex->SetTextSize(0.06);
   float_t x0 = 0.02;
   float   y = 0.83;
   float fontsize = 0.03;

   // top text
   {
      sprintf(mytext,"valid/total Si hits: %i/%i", nvalid, nhits);
      latex->DrawLatex(x0, y, mytext);
      y -= fontsize;

      sprintf(mytext,"valid Si pixel (strip) hits: %i (%i)", npix, nstrip);
      latex->DrawLatex(x0, y, mytext);
      y -= fontsize;

      sprintf(mytext,"Number of layers hit: %i", ndet);
      latex->DrawLatex(x0, y, mytext);
   }
   // legend
   {
      Float_t pos[4];
      pos[0] = 0.4;
      pos[2] = 0.55;
      Float_t boxH = 0.015;

      //_________________________
      y -= fontsize*2;
      sprintf(mytext,"X hit");
      latex->DrawLatex(x0, y, mytext);
      pos[1] = y; pos[3] = pos[1] + boxH;
      drawBox(pos, resXCol, resXFill);

      //_________________________
      y -=  boxH*2;
      sprintf(mytext,"Y hit");
      latex->DrawLatex(x0, y, mytext);
      pos[1] = y; pos[3] = pos[1] + boxH;
      drawBox(pos, resYCol, resYFill);

      //--------------
      y -= boxH*2;
      sprintf(mytext, "stereo hit");
      latex->DrawLatex(x0, y, mytext);
      pos[1] = y; pos[3] = pos[1] + boxH;
      drawBox(pos, stereoCol, stereoFill);

      //--------------
      y -= boxH*2;
      sprintf(mytext, "invalid hit");
      latex->DrawLatex(x0, y, mytext);
      pos[1] = y; pos[3] = pos[1] + boxH;
      drawBox(pos, invalidCol, invalidFill);

      y -= boxH*2;
   }

   // explanation
   y -= fontsize;
   sprintf(mytext,"Residual: ");
   latex->DrawLatex(x0, y, mytext);
   y -= fontsize;
   sprintf(mytext, "(X_{hit} - X_{traj}) #/ #sqrt{#sigma^{2}_{hit} + #sigma^{2}_{traj}}");
   latex->DrawLatex(x0, y, mytext);
   y -= fontsize;
   sprintf(mytext,"Y is detector local coordinate");
   latex->DrawLatex(x0, y, mytext);
   y -= fontsize;
   sprintf(mytext,"X is detector local coordinate");
   latex->DrawLatex(x0, y, mytext);
   y -= fontsize;
   sprintf(mytext,"signed by sgn(#hat{X}#bullet#hat{#phi})");
   latex->DrawLatex(x0, y, mytext);
   y -= fontsize;


   //______________________________________________________________________________
   // draw histogram
   TCanvas *canvas = FWTrackDetailView::viewCanvas();
   canvas->cd();

   TPad* padX = new TPad("pad1","pad1",0.1,0.05,0.9,0.95);
   padX->SetBorderMode(0);
   canvas->cd();
   padX->Draw();

   TH2F* h_res = new TH2F("h_resx","h_resx",10,-5.5,5.5,ndet,0,ndet);
   char* res_str= "residuals in Si detector local x-y coord.";
   char* title = "residual";
   padX->cd();
   padX->SetLeftMargin(0.2);
   padX->SetRightMargin(0.2);
   padX->Range(-7.7,-1.5,6.966667,13.5);
   padX->SetFrameLineWidth(0);
   padX->Modified();
   h_res->SetDirectory(0);
   h_res->SetTitle("");
   h_res->SetStats(kFALSE);
   h_res->SetTitleSize(0.04);
   h_res->GetYaxis()->SetRangeUser(0,ndet+1);
   h_res->GetYaxis()->SetLabelSize(0.06);
   h_res->SetXTitle(title);
   h_res->GetXaxis()->SetTickLength(0);
   h_res->GetYaxis()->SetTickLength(0);
   h_res->GetXaxis()->SetNdivisions(20);
   h_res->Draw();
   padX->SetGridy();

   TPaveText *pt = new TPaveText(0.05,0.915,0.95,0.995,"blNDC");
   pt->SetName("mytitle");
   pt->SetBorderSize(0);
   pt->AddText(res_str);
   pt->Draw();

   canvas->cd();

   float larray[9]={0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.5, 4.5, 5.5};
   float larray2[8];
   for(int l=0; l<8; l++) {
      float diff2=(larray[l+1]-larray[l])/2;
      larray2[l]=larray[l]+diff2;
      //  printf("(%.1f,%.1f),",larray[i],larray[i+1]);
   }

   int resi[2][64];
   for(int l=0; l<nhits; l++) {
      for(int k=0; k<8; k++) {
         if(fabs(res[0][l])==larray2[k])
            resi[0][l]=k;
         if(fabs(res[1][l])==larray2[k])
            resi[1][l]=k;
      }
   }

   TLine* lines[17];
   for(int l=0; l<17; l++) {
      int ix=l%9;
      int sign=1;
      sign = (l>8) ? -1 : 1;
      lines[l] = new TLine(sign*larray[ix],0,sign*larray[ix],ndet);
      if(l!=9)
         lines[l]->SetLineStyle(3);
      padX->cd();
      lines[l]->Draw();
   }

   float width=0.25;
   int filltype;
   Color_t color;
   Float_t box[4];
   padX->cd();

   for(int h=0; h<2; h++) {
      float height1=0;
      for(int j=0; j<ndet; j++) {
         // take only X res and Y pixel residals
         if (strcmp(det_tracker_str[substruct[det[j]]-1], "PXB") && h)
            continue;

         char det_str2[256];
         sprintf(det_str2,"%s/%i",det_tracker_str[substruct[det[j]]-1],subsubstruct[det[j]]);
         h_res->GetYaxis()->SetBinLabel(j+1,det_str2);

         int diff=det[j+1]-det[j];
         int k=0;
         width=1.0/diff;

         for(int l=det[j]; l<(det[j]+diff); l++) {
            //      g->SetPoint(l,resx[l],j+0.5);
            //	printf("%i, %f %f %f\n",l,resx[l],sign*larray[resxi[l]],sign*larray[resxi[l]+1]);
            int sign = (res[h][l]<0) ? -1 : 1;
            box[0] = (hittype[l]==0) ? sign*larray[resi[h][l]] : -5.5;
            box[2] = (hittype[l]==0) ? sign*larray[resi[h][l]+1] : 5.5;
            box[1] = height1+width*k;
            box[3] = height1+width*(k+1);

            if(stereo[l]==1) {
	       color    = stereoCol;
               filltype = stereoFill;
            }
            else if(hittype[l]!=0) {
               color    = invalidCol;
               filltype = invalidFill;
            }
            else {
               filltype = h ? resYFill : resXFill;
               color    = h ? resYCol  : resXCol;
            }

            drawBox(box, color, filltype, h<1);
            k++;
         }
         height1 +=1;
      }
   }

   viewCanvas()->cd();
   return 0;
}




REGISTER_FWDETAILVIEW(FWTrackDetailView);

