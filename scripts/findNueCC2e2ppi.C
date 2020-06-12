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
#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/MCParticle.h"

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


const double APA_X_POSITIONS[] = {-726.7681, 0., 726.7681};
const double CPA_X_POSITIONS[] = {-363.38405, 363.38405};
const double TPC_Z_SIZE = 232.39;


int whichTPC(double x, double z, double y);

void
findNueCC2e2ppi(std::string const& filename, size_t nevents, const char* recolabel = "pandoraTrack")
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

  size_t evcounter=-1;

  InputTag gen_tag("generator");
  InputTag mc_tag("largeant");
  InputTag reco_tag(recolabel);

  // Create a vector of length 1, containing the given filename.
  vector<string> filenames(1, filename);

  for (gallery::Event ev(filenames); !ev.atEnd(); ev.next()) {
      evcounter++;
      if (evcounter >= nevents) break;


      auto const& mctruth = *ev.getValidHandle<vector<simb::MCTruth> >(gen_tag);
      if (mctruth.empty()) continue;
      auto const& mcparts = *ev.getValidHandle<vector<simb::MCParticle> >(mc_tag);
      if (mcparts.empty()) continue;
      auto const& recotrks = *ev.getValidHandle<vector<recob::Track> >(reco_tag);
      if (recotrks.size() < 2) continue;

      auto genparts = mctruth[0];

      // count number of generated particles
      int n_generated = 0;
      vector<int> pdgs;
      for (int i = 0; i < genparts.NParticles(); i++)
	  if (genparts.GetParticle(i).StatusCode() == 1){
	      n_generated++;
	      pdgs.push_back(genparts.GetParticle(i).PdgCode());
	  }


      // find nu_e
      auto neutrino = genparts.GetNeutrino();
      if ( abs(neutrino.Lepton().PdgCode()) != 11 || // not nu_e
	   neutrino.CCNC() != simb::kCC // not CC interaction
	   )
	  continue;

      // see if produced particles contain electron 2 protons and a pi-
      int nel = 0;
      int nprot = 0;
      int npimin = 0;
      for (auto pdg: pdgs) {
	  if (pdg == 11)
	      nel++;
	  else if (pdg == 2212)
	      nprot++;
	  else if (pdg == -211)
	      npimin++;
      }

      if (nel != 1) continue;
      if (nprot != 2) continue;
      if (npimin != 1) continue;

      size_t nmcparts = mcparts.size();

      auto ipart = mcparts.begin();
      auto partstop = mcparts.end();


      // double Emu = muon->E(0) - muon->Mass();
      // double Ep = proton->E(0) - proton->Mass();

      unsigned long run = ev.eventAuxiliary().runID().run();
      unsigned long subRun = ev.eventAuxiliary().subRunID().subRun();
      unsigned long event = ev.eventAuxiliary().eventID().event();

      auto pos = neutrino.Lepton().Position(0);
      auto tpc = whichTPC( pos.X(), pos.Z(), pos.Y() );

      cout<<"Entry "<<evcounter
	  <<", run: "<<run
	  <<", subrun: "<<subRun
	  <<", event: "<<event
	  <<", TPC: "<<tpc
	  <<endl;
      cout<<"  N generated: "<<n_generated
	  <<", N particles: "<<nmcparts<<endl;
      // cout<<"    "<<Emu<<", "<<Ep<<endl;
      // cout<<"    "<<mulen<<", "<<plen<<endl;
  }
}


int whichTPC(double x, double z, double y) {
    int ix = x > 0;
    // for (; ix < 4; ++ix) {
    // 	int iapa = (ix+1)/2;
    // 	int icpa = ix/2;
    // 	double xapa = APA_X_POSITIONS[iapa];
    // 	double xcpa = CPA_X_POSITIONS[icpa];
    // 	if ( (x > xapa && x < xcpa) || (x < xapa && x > xcpa) )
    // 	    break;
    // }

    int iz = z / TPC_Z_SIZE;

    int iy = y > 0;

    int tpc = iz * 4 + iy * 2 + ix;

    return tpc;
}
