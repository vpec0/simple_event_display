#define FOR(i, size) for(unsigned int i = 0; i < size; ++i)

void setMargin(TAxis* ax);
void setMargin(TH1* h);

void plotBetter(const char *fname)
{
    // gROOT->ForceStyle(1);

    int font = 43;

    float label_size = 13;
    float label_offset = 0.02;
    float title_size = 15;
    float pad_margin = 0.1;
    float pad_left_margin = 0.15;
    float pad_right_margin = 0.05;

    float c_height = 500;
    float c_width = 400;
    float pad_offset = 0.05;
    float pad_height = (1.-pad_offset)/3.;
    float pad_width = 1.;

    //vector<double> x_range_low = {}


    // gStyle->SetOptStat(0);

    // gStyle->SetHistLineColor(kBlack);

    // gStyle->SetLabelSize(0.08, "xy");
    // gStyle->SetLabelOffset(0.02,"xy");



    vector<TString> pad_names;
    FOR(i, 3) {
	pad_names.push_back(Form("fWireProjP%d", i));
    }


    TString inname = fname;
    auto outname = inname;
    outname.ReplaceAll(".root", "_better.pdf");

    auto f = TFile::Open(inname, "read");
    auto c_in = (TCanvas*) f->Get("evd_canvas");

    vector<TPad*> pads;
    for (auto name : pad_names)
	pads.push_back((TPad*)c_in->GetPrimitive(name));


    auto c = new TCanvas("c", "", 400, 500);

    int ipad = 0;
    for (auto pad: pads) {
	cout<<"Dealing with pad "<<pad->GetName()<<endl
	    <<"  It has "<<pad->GetListOfPrimitives()->GetSize()<<" primitives"<<endl;
	pad -> SetPad(0., ipad*pad_height + pad_offset,
		      pad_width, (ipad+1)*pad_height + pad_offset);

	pad->Draw();

	pad->SetLeftMargin(pad_left_margin);
	pad->SetRightMargin(pad_right_margin);
	if (ipad > 0)
	    pad->SetTopMargin(pad_margin);
	else
	    pad->SetTopMargin(0.01);

	auto h = (TH1*)pad->GetPrimitive("hframe");
	h->GetXaxis()->SetLabelSize(label_size);
	h->GetYaxis()->SetLabelSize(label_size);

	h->GetXaxis()->SetLabelOffset(label_offset);
	//h->GetYaxis()->SetLabelOffset(label_offset);

	h->GetXaxis()->SetTitleSize(title_size);
	h->GetYaxis()->SetTitleSize(title_size);
	h->GetXaxis()->SetTitleOffset(2.8);
	h->GetYaxis()->SetTitleOffset(2.2);

	h->GetXaxis()->SetLabelFont(font);
	h->GetYaxis()->SetLabelFont(font);

	h->GetXaxis()->SetTitleFont(font);
	h->GetYaxis()->SetTitleFont(font);

	if (ipad == 0) {
	    h->GetXaxis()->SetTitle("Wire Segment");
	    pad -> SetPad(0., ipad*pad_height,
			  pad_width, (ipad+1)*pad_height + pad_offset);
	    pad->SetBottomMargin(0.2);
	}

	if (ipad == 1) {
	    h->GetYaxis()->SetTitle("TDC");
	}

	// remove track numbering from the pad
	for ( auto primitive: *pad->GetListOfPrimitives() ) {
	    if ( strstr(primitive->ClassName(), "TText") )
		pad->GetListOfPrimitives()->Remove(primitive);
	    if ( strstr(primitive->ClassName(), "TMarker") )
		pad->GetListOfPrimitives()->Remove(primitive);
	}


	// set axis range to allow for margins around plotted objects
	c->Update();
	auto xaxis = h->GetXaxis();
	auto yaxis = h->GetXaxis();

	//setMargin(xaxis);
	setMargin(h);


	cout<<"X: "<<pad->GetAbsXlowNDC()
	    <<", "<<pad->GetAbsWNDC()<<endl;
	cout<<"Y: "<<pad->GetAbsYlowNDC()
	    <<", "<<pad->GetAbsHNDC()<<endl;

	ipad++;
    }
    c->Draw();

    c->SaveAs(outname);

    // save also as a root file
    outname.ReplaceAll(".pdf", ".root");
    c->SaveAs(outname);
}


void setMargin(TAxis* ax) {
    // tries to add margin to the axis' range
    double low = ax -> GetXmin();
    double hi = ax -> GetXmax();

    cout<<"Axis range: "<<low<<" "<<hi<<endl;

    ax->SetRangeUser(low, hi);
}

void setMargin(TH1* h) {
    // tries to add margin to the histogram's range
    double low = h->GetMinimum();
    double hi = h->GetMaximum();

    h->SetMinimum(low - 0.1*(hi-low));
    h->SetMaximum(hi + 0.1*(hi-low));
}
