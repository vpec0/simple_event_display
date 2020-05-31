#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "canvas/Utilities/InputTag.h"
#include "gallery/Event.h"

#include "TFile.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TGraph.h"
#include "TH2I.h"
#include "TStyle.h"
#include "TColor.h"
#include "lardataobj/RecoBase/Hit.h"

using namespace art;
using namespace std;

// Viktor Pec, May 2020
// Based on TJ's evd_rawdigits.C in dune/GalleryScripts

// make a poor-man's event display of raw::RawDigits for the ievcount'th event in the file using gallery
// limitation:  does not call the raw::Uncompress method, and so zero-suppressed rawdigits cannot be used.

// Tom Junk, September 2017
// Oct 2017 -- use channel indexes from raw::RawDigits instead of which rawdigit it is
// arguments:  filename -- input file, larsoft formatted
// ievcount:  which event to display.  This is the tree index in the file and not the event number
// autoped:  true if you want to subtract the average of the adc values of a channel before displaying
// minval, maxval -- in order to color the plot well.  Minval: white; Maxval: black
// inputtag:  use "daq" for 3x1x1 data or MC, and SplitterInput:TPC for split 35t data

// Example invocation for a 3x1x1 imported rootfile, make an event display for the first event in the file.
// root [0] .L evd_rawdigits.C++
// root [1] evd_rawdigits("/pnfs/dune/tape_backed/dunepro/test-data/dune/raw/01/85/12/09/wa105_r842_s32_1501156823.root",0);



typedef pair<float,float> hit_t;
typedef vector<hit_t> vhit_t;
typedef map<size_t, vhit_t> mwiretovhit_t;
typedef map<unsigned short, mwiretovhit_t[3]> mtpctowires_t;

const int MAX_WIRES[3] = {1300, 1300, 500};
const int MAX_TICKS = 4000;

TCanvas* createCanvas(int tpc);
TH2* getHitsHist(mwiretovhit_t& wires, int tpc, int plane);


void
evd_hits(std::string const& filename, size_t ievcount, bool autoped=true, int minval=0, int maxval=30, std::string const& inputtag="gaushit")
{
  //gStyle->SetPalette(kGreyScale);
  //gStyle->SetPalette(kInvertedDarkBodyRadiator);
  //TColor::InvertPalette();


  gStyle->SetOptStat(0);
  // Int_t MyPalette[100];
  // Double_t Red[]    = {1., 0.0};
  // Double_t Green[]  = {1., 0.0};
  // Double_t Blue[]   = {1., 0.0};
  // Double_t Length[] = {0., 1.0};
  // Int_t FI = TColor::CreateGradientColorTable(2, Length, Red, Green, Blue, 100);
  // for (int i=0;i<100;i++) MyPalette[i] = FI+i;
  // gStyle->SetPalette(100, MyPalette);

  size_t evcounter=0;

  InputTag hit_tag(inputtag);

  // Create a vector of length 1, containing the given filename.
  vector<string> filenames(1, filename);

  for (gallery::Event ev(filenames); !ev.atEnd(); ev.next()) {
      if (evcounter == ievcount) {

	  auto const& hits = *ev.getValidHandle<vector<recob::Hit> >(hit_tag);

	  if (hits.empty()) break;

	  mtpctowires_t hitcontainer;

	  // fill the hit container
	  size_t nhits = hits.size();
	  for (size_t i=0; i<nhits; ++i) {
	      auto wid = hits[i].WireID();

	      auto wire = wid.Wire;
	      auto plane = wid.Plane;
	      auto tpc = wid.TPC;

	      auto view = hits[i].View();

	      float tdc = hits[i].PeakTime();
	      float adc = hits[i].SummedADC();

	      //cout<<"Plane: "<<plane<<", view: "<<view<<endl;

	      hitcontainer[tpc][plane][wire].push_back(hit_t(tdc,adc));
	  }

	  cout<<"Number of stored TPCs: "<<hitcontainer.size()<<endl;

	  for (auto tpcpair: hitcontainer) {
	      auto tpc = tpcpair.first;
	      auto c = createCanvas(tpc);
	      cout<<"TPC "<<tpc<<": "<<endl;
	      for (int iplane = 0; iplane < 3; ++iplane) {
		  cout<<"  Plane "<<iplane<<" with "
		      <<tpcpair.second[iplane].size()
		      <<" wires"<<endl;


		  auto h = getHitsHist(tpcpair.second[iplane], tpc, iplane);

		  if (!h) continue;

		  c->cd(iplane+1);
		  h->Draw("colz");
	      }
	  }



	  break;
      }
      ++evcounter;
  }
}


TCanvas* createCanvas(int tpc)
{
    auto c = new TCanvas(Form("c%d",tpc),
			 Form("TPC %d", tpc),
			 300, 900
			 );

    c->Divide(1,3);

    return  c;
}

TH2* getHitsHist(mwiretovhit_t& wires, int tpc, int plane)
{
    if (!wires.size()) return 0;

    int wire_low = wires.begin()->first;
    int wire_hi = wires.rbegin()->first;

    auto h = new TH2F(Form("h%d_%d", tpc, plane),
		      ";TDC;Wire;ADC",
		      MAX_TICKS,
		      0, MAX_TICKS,
		      MAX_WIRES[plane],
		      0, MAX_WIRES[plane]
		      );
    h->SetDirectory(gROOT);

    int tdc_low = 4000;
    int tdc_hi = 0;
    for (auto wirepair: wires) {
	auto wire = wirepair.first;
	for (auto tdcadcpair: wirepair.second) {
	    auto tdc = tdcadcpair.first;
	    auto adc = tdcadcpair.first;

	    if (tdc_low > tdc) tdc_low = tdc;
	    if (tdc_hi < tdc) tdc_hi = tdc;

	    h->SetBinContent((int)tdc, wire, adc);
	}
    }

    int margin_tdc = (tdc_hi - tdc_low) * 0.15;
    int margin_wire = (wire_hi - wire_low) * 0.15;
    h->GetXaxis()->SetRange(tdc_low-margin_tdc, tdc_hi + margin_tdc);
    h->GetYaxis()->SetRange(wire_low-margin_wire, wire_hi + margin_wire);

    return h;
}
