#ifndef EVENTDISPLAY_H
#define EVENTDISPLAY_H

#include "anatree_core.h"

#include "TDatabasePDG.h"
#include "TParticlePDG.h"

class EventDisplay
{
public:
    EventDisplay(anatree* anaSel);

    int next(const int entry = -1);
    int plot();
    void PrintInfo();
    int GetEntry(int entry);

    void AllowBranches(const std::vector<string>& br_list);

    std::vector<std::vector<TArrow*> > GetShowers();
    TMultiGraph** plotTracks ();
    TMultiGraph** plotTracksWV ();
    TH1* plot2Dgraphs(vector<TGraph2D*>& grv);

private:
    void reverseYaxix(TMultiGraph* mg);
    int getPdg(int trk);


private:
    anatree* mAnaSel;
    TTree* mTree;
    int mEntry;

    TCanvas* mCanvas; // 3D->2D projections
    TCanvas* mCanvasWv; // wire view
    TCanvas* mCanvas3d; // 3D view

    // U and V axis direction
    TVector3 u_dir;
    TVector3 v_dir;
};

EventDisplay::EventDisplay(anatree* anaSel) :
    mAnaSel(anaSel),
    mTree(anaSel->fChain),
    mEntry(0),
    mCanvas(0),
    mCanvas3d(0)
{
    (u_dir = TVector3(0,0,1)).RotateX(-35.7/180. * TMath::Pi());
    (v_dir = TVector3(0,0,1)).RotateX(35.7/180. * TMath::Pi());

    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);

    mCanvas = new TCanvas("c", "2D View", 1024,256);
    mCanvas->Divide(4,1);

    mCanvasWv = new TCanvas("cwv", "2D Wire-Time View", 450,500);
    mCanvasWv->Divide(1,3);

    // mCanvas3d = new TCanvas("c3d", "3D View", 300,300);

    mTree->SetBranchStatus("*", 0);
    // allow only necessary branches
    vector<string> allowed ({"run"
		, "subrun"
		, "event"
		, "ntracks_pmtrack"
		//, "n_cal_points_byplane"
		//, "track_calo_xyz_byplane"
		, "ntrkhits_pmtrack"
		, "trkxyz_pmtrack"
		, "trkstartx_pmtrack"
		, "trkstarty_pmtrack"
		, "trkstartz_pmtrack"
		, "trkendx_pmtrack"
		, "trkendy_pmtrack"
		, "trkendz_pmtrack"
		, "geant_list_size"
		, "pdg"
		, "TrackId"
		// Don't have points for MC trajectories in standard AnalysisTree
		/* , "mc_npoints" */
		/* , "mc_xyz" */
		, "Px"
		, "Py"
		, "Pz"
		, "StartPointx"
		, "StartPointy"
		, "StartPointz"
		, "EndPointx"
		, "EndPointy"
		, "EndPointz"
		, "pathlen"
		, "trkg4id_pmtrack"
		, "nshowers_emshower"
		, "shwr_startx_emshower"
		, "shwr_starty_emshower"
		, "shwr_startz_emshower"
		, "shwr_startdcosx_emshower"
		, "shwr_startdcosy_emshower"
		, "shwr_startdcosz_emshower"
		, "shwr_length_emshower"
		});
    AllowBranches(allowed);

    // next(mEntry);
}

void EventDisplay::PrintInfo()
{
    mTree->GetEntry(mEntry);
    cout<<"Entry "<<mEntry
	<<", Run No: "<<mAnaSel->run
	<<", Subrun No: "<<mAnaSel->subrun
	<<", Event No: "<<mAnaSel->event<<endl;
    // cout<<"Kaon track: "<<endl
    // 	<<"    KinE: "<<(mAnaSel->mc_startMomentum[1][3]-0.4937)*1e3<<" MeV"<<endl
    // 	<<"    Length: "<<mAnaSel->pathlen[1]<<" cm"<<endl;
    cout<<"N Reco Tracks: "<< mAnaSel->ntracks_pmtrack<<endl;

    // TPC with hits:

}

int EventDisplay::GetEntry(int entry)
{
    mEntry = entry;
    return mTree->GetEntry(mEntry);
}

int EventDisplay::next(const int entry)
{
    if (entry == -1)
	mEntry++;
    else
	mEntry = entry;

    PrintInfo();
    plot();

    return 0;
}

int EventDisplay::plot()
{
    // plot calorimetric points by tracks
    static TMultiGraph** mg_rec = 0;
    static TMultiGraph** mg_rec_wv = 0;

    // clean up previous event
    if (mg_rec)
	delete[] mg_rec;
    if (mg_rec_wv)
	delete[] mg_rec_wv;

    // get tgraphs for orthogonal views and plane views
    mg_rec = plotTracks();
    mg_rec_wv = plotTracksWV();

    //vector<vector<TArrow*> > showers = GetShowers();

    // plot graphs in respective views
    for (int i = 0; i < 3; i++) {
    	mCanvas -> cd(i+2);
	gPad->Clear();
    	mg_rec[i] -> Draw("AP PMC PLC");
	gPad->Update();

	mCanvasWv->cd(i+1);
	gPad->Clear();
	mg_rec_wv[i] -> Draw("AP PMC PLC");
	gPad->Update();
    }

    // build an info legend
    int ngraphs = 0;
    if ( mg_rec[0]->GetListOfGraphs() )
	ngraphs = mg_rec[0]->GetListOfGraphs()->GetSize();
    else
	ngraphs = 0;
    int ncolumns = (ngraphs+4)/5;
    int nrows = (ngraphs<5)?ngraphs:(ngraphs/ncolumns + ngraphs%ncolumns);
    TLegend* legreco = new TLegend(0.98-ncolumns*0.22,0.5-nrows*0.06-0.06,0.98,0.5, "Reco");
    if (ngraphs)
	for ( auto gr: *(mg_rec[0]->GetListOfGraphs()) ) legreco->AddEntry(gr, gr->GetTitle(), "lp");
    legreco->SetNColumns(ncolumns);
    legreco->SetFillStyle(0);

    cout<<"Plot Reco legend: "<<endl
        <<"  ngraphs: "<<ngraphs<<endl
        <<"  ncolumns: "<<ncolumns<<endl
        <<"  nrows: "<<nrows<<endl;

    mCanvas->cd(1);
    gPad->Clear();
    legreco->Draw();

    mCanvas->Update();
    return 0;
}

void EventDisplay::AllowBranches(const std::vector<string>& br_list)
{
    for (auto it = br_list.begin(); it != br_list.end(); it++) {
	mTree -> SetBranchStatus(it->c_str(),1);
    }
    return;
}


TH1* EventDisplay::plot2Dgraphs(vector<TGraph2D*>& grv)
{
    mCanvas3d -> cd();

    vector<double> xmins, ymins, zmins, xmaxs, ymaxs, zmaxs;
    int size = grv.size();
    if (size == 0) return 0;
    for(int i = 0; i < size; i++) {
	xmins.push_back(grv[i]->GetXmin());
	ymins.push_back(grv[i]->GetYmin());
	zmins.push_back(grv[i]->GetZmin());
	xmaxs.push_back(grv[i]->GetXmax());
	ymaxs.push_back(grv[i]->GetYmax());
	zmaxs.push_back(grv[i]->GetZmax());

    }

    double xmin = TMath::MinElement(xmins.size(), xmins.data());
    double xmax = TMath::MaxElement(xmaxs.size(), xmaxs.data());
    double ymin = TMath::MinElement(ymins.size(), ymins.data());
    double ymax = TMath::MaxElement(ymaxs.size(), ymaxs.data());
    double zmin = TMath::MinElement(zmins.size(), zmins.data());
    double zmax = TMath::MaxElement(zmaxs.size(), zmaxs.data());

    // cout<<xmin<<", "<<xmax<<endl
    // 	<<ymin<<", "<<ymax<<endl
    // 	<<zmin<<", "<<zmax<<endl;

    TH1* hold = (TH1*)gDirectory->Get("hgr2d");
    if (hold) delete hold;

    auto hnew = new TH3F("hgr2d","", 1, xmin, xmax, 1, ymin, ymax, 1, zmin, zmax);
    hnew->SetStats(0);
    hnew->Draw();
    for(int i = 0; i < size; i++) {
	    grv[i]->Draw("p line same pmc plc");
    }

    hnew->SetTitle(";z [cm];x [cm]; y [cm]");
    hnew->GetXaxis()->SetTitleOffset(1.2);
    hnew->GetYaxis()->SetTitleOffset(1.2);
    hnew->GetZaxis()->SetTitleOffset(1.2);

    //cout<<grv[0]->GetHistogram()->GetName()<<endl;

    //gPad->BuildLegend();

    return hnew;
}

TMultiGraph** EventDisplay::plotTracks() {
    static TDatabasePDG* pdgdb = TDatabasePDG::Instance();

    int nTracks  = mAnaSel->ntracks_pmtrack;

    TMultiGraph **mg = new TMultiGraph*[3];

    const char* titles[3] = {" - Front;X [cm];Y [cm]", "- Top;X [cm];Z [cm]", " - Side;Z [cm];Y [cm]"};

    for (int i = 0; i < 3; i++) {
	mg[i] = new TMultiGraph(Form("mgTracks%d",i), Form("Reco Tracks%s", titles[i]));
    }

    for (int itrk = 0; itrk < nTracks; itrk++) {
	vector<double> x[3], y[3]; // projections xy, xz, zy
	for (int ipln = 0; ipln < 3; ipln++) {
	    Float_t* reco_pts_xyz = (Float_t*)mAnaSel->trkxyz_pmtrack[itrk][ipln];
	    int npoints = mAnaSel->ntrkhits_pmtrack[itrk][ipln];
	    for (int ipts = 0; ipts < npoints; ipts++) {
		x[0].push_back(reco_pts_xyz[ipts*3]);
		y[0].push_back(reco_pts_xyz[ipts*3 + 1]);
		x[1].push_back(reco_pts_xyz[ipts*3]);
		y[1].push_back(reco_pts_xyz[ipts*3 + 2]);
		x[2].push_back(reco_pts_xyz[ipts*3 + 2]);
		y[2].push_back(reco_pts_xyz[ipts*3 + 1]);
	    }
	}

	// create and plot the 3 graphs
	static const int viewX[3] = {0, 0, 2};
	static const int viewY[3] = {1, 2, 1};
	// double* vertex = mAnaSel->track_vtx[itrk];
	// double* end = mAnaSel->track_end[itrk];
	int pdg = getPdg(itrk);
	//cout<<"Dealing with track "<<itrk<<", got G4 track ID: "<<pdg<<endl;
	auto pdgpart = pdgdb->GetParticle(pdg);
	TString name = "none";
	if (pdgpart)
	    name = pdgpart->GetName();

	// cout<<"Track "<<itrk<<" vertex: "
	//     <<vertex[0]<<", "<<vertex[1]<<", "<<vertex[2]<<", "
	//     <<"end: "
	//     <<end[0]<<", "<<end[1]<<", "<<end[2]<<endl;
	if (name.Contains("gamma"))
	    name.ReplaceAll("gamma", "#gamma");
	for (int i = 0; i < 3; i++) {
	    auto gr = new TGraph(x[i].size(), x[i].data(), y[i].data());
	    gr->SetName(Form("grTrack%d_p%d", itrk, i));
	    gr->SetTitle(name);
	    //gr->SetTitle(Form("%d", pdg));
	    gr->SetMarkerSize(0.3);
	    gr->SetMarkerStyle(kOpenSquare);
	    gr->SetMarkerColor(itrk+2);
	    gr->SetLineColor(itrk+2);

	    // // add tracks direction
	    // auto arrow = new TArrow(vertex[viewX[i]], vertex[viewY[i]],
	    // 			    end[viewX[i]], end[viewY[i]], 0.015, "|-|>");
	    // arrow->SetNDC(kFALSE);
	    // gr->GetListOfFunctions()->Add(arrow);

	    mg[i]->Add(gr, "p");
	}
    }

    return mg;
}

TMultiGraph** EventDisplay::plotTracksWV() {
    static TDatabasePDG* pdgdb = TDatabasePDG::Instance();

    int nTracks  = mAnaSel->ntracks_pmtrack;

    TMultiGraph **mg = new TMultiGraph*[3];

    //const char* titles[3] = {" - Front;X [cm];Y [cm]", "- Top;X [cm];Z [cm]", " - Side;Z [cm];Y [cm]"};
    const char* titles[3] = {";Z direction [cm];X [cm]", ";U direction [cm];X [cm]", ";V direction [cm];X [cm]"};

    for (int i = 0; i < 3; i++) {
	mg[i] = new TMultiGraph(Form("mgTracksWv%d",i), Form("Reco Tracks%s", titles[i]));
    }

    for (int itrk = 0; itrk < nTracks; itrk++) {
	vector<double> x[3], y[3]; // projections xy, xz, zy
	for (int ipln = 0; ipln < 3; ipln++) {
	    Float_t* reco_pts_xyz = (Float_t*)mAnaSel->trkxyz_pmtrack[itrk][ipln];
	    int npoints = mAnaSel->ntrkhits_pmtrack[itrk][ipln];
	    for (int ipts = 0; ipts < npoints; ipts++) {
		Float_t* coord = reco_pts_xyz + ipts*3;
		TVector3 space_point(coord);
		double u = space_point.Dot(u_dir);
		double v = space_point.Dot(v_dir);
		double z = coord[2];
		double drift = coord[0];
		// if (coord[0] > 0) {
		// 	double tmp = u;
		// 	u = v;
		// 	v = tmp;
		// } else
		// 	drift = -drift;

		x[0].push_back(coord[2]);
		y[0].push_back(drift);
		x[1].push_back(u);
		y[1].push_back(drift);
		x[2].push_back(v);
		y[2].push_back(drift);
	    }
	}

	// create and plot the 3 graphs
	static const int viewX[3] = {0, 0, 2};
	static const int viewY[3] = {1, 2, 1};
	// double* vertex = mAnaSel->track_vtx[itrk];
	// double* end = mAnaSel->track_end[itrk];
	int pdg = getPdg(itrk);
	auto pdgpart = pdgdb->GetParticle(pdg);
	TString name = "none";
	if (pdgpart)
	    name = pdgpart->GetName();

	// cout<<"Track "<<itrk<<" vertex: "
	//     <<vertex[0]<<", "<<vertex[1]<<", "<<vertex[2]<<", "
	//     <<"end: "
	//     <<end[0]<<", "<<end[1]<<", "<<end[2]<<endl;
	if (name.Contains("gamma"))
	    name.ReplaceAll("gamma", "#gamma");
	for (int i = 0; i < 3; i++) {
	    auto gr = new TGraph(x[i].size(), x[i].data(), y[i].data());
	    gr->SetName(Form("grTrack%d_p%d", itrk, i));
	    gr->SetTitle(name);
	    //gr->SetTitle(Form("%d", pdg));
	    gr->SetMarkerSize(.3);
	    gr->SetMarkerStyle(kOpenSquare);
	    gr->SetMarkerColor(itrk+2);
	    gr->SetLineColor(itrk+2);

	    // // add tracks direction
	    // auto arrow = new TArrow(vertex[viewX[i]], vertex[viewY[i]],
	    // 			    end[viewX[i]], end[viewY[i]], 0.015, "|-|>");
	    // arrow->SetNDC(kFALSE);
	    // gr->GetListOfFunctions()->Add(arrow);

	    mg[i]->Add(gr, "p");
	}
    }

    return mg;
}

std::vector<std::vector<TArrow*> > EventDisplay::GetShowers()
{
    std::vector<std::vector<TArrow*> > showers;
    showers.resize(3);

    int size = mAnaSel->nshowers_emshower;
    if (size >  MAX_SHOWERS)
	size = MAX_SHOWERS;

    //cout<<"Will go through "<<size<<" showers"<<endl;

    for (int ishwr = 0; ishwr < size ; ishwr++) {
	TVector3 shower(mAnaSel->shwr_startdcosx_emshower[ishwr],
			mAnaSel->shwr_startdcosy_emshower[ishwr],
			mAnaSel->shwr_startdcosz_emshower[ishwr]);
	shower.SetMag(mAnaSel->shwr_length_emshower[ishwr]);
	TVector3 start(mAnaSel->shwr_startx_emshower[ishwr],
		       mAnaSel->shwr_starty_emshower[ishwr],
		       mAnaSel->shwr_startz_emshower[ishwr]);
	TVector3 end(shower+start);

	// create 3 views, xy, xz, zy
	double startX[3] = {start.X(), start.X(), start.Z()};
	double startY[3] = {start.Y(), start.Z(), start.Y()};
	double endX[3] = {end.X(), end.X(), end.Z()};
	double endY[3] = {end.Y(), end.Z(), end.Y()};

	//cout<<"Will fill views for shower "<<ishwr<<endl;

	for (int iview = 0; iview < 3; iview++) {
	    auto arrow = new TArrow(startX[iview], startY[iview],
				    endX[iview], endY[iview], 0.015, "|-|>");
	    arrow->SetLineWidth(2);
	    showers[iview].push_back(arrow);
	}
    }

    return showers;
}

void EventDisplay::reverseYaxix(TMultiGraph* mg)
{
    TAxis* ax_orig = mg->GetYaxis();
    ax_orig->SetLabelOffset(999);
    ax_orig->SetTickLength(0);

    gPad->Update();
    auto ax_new = new TGaxis(gPad->GetUxmin(),
			     gPad->GetUymax(),
			     gPad->GetUxmin()-0.001,
			     gPad->GetUymin(),
			     -ax_orig->GetXmax(),
			     -ax_orig->GetXmin(),
			     510,"+");
    ax_new->SetLabelOffset(-0.03);
    ax_new->Draw();
}

int EventDisplay::getPdg(int trk)
{
    int g4trkid = mAnaSel->trkg4id_pmtrack[trk];
    // loop over true particles to find the corresponding track id and index to get PDG code
    int idx = 0;
    while (idx < mAnaSel->geant_list_size) {
	if (g4trkid == mAnaSel->TrackId[idx])
	    break;
	idx++;
    }

    if (idx == mAnaSel->geant_list_size)
	return -99;

    return mAnaSel->pdg[idx];
}




// void ::Loop()
// {}

#endif
