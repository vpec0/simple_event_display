#define FOR(i, size) for(unsigned int i = 0; i < size; ++i)


void plotBetter(const char *fname)
/**
 * Takes a root file as an input. This needs to be created from
 * standard larsoft event display and needs to contain the canvas
 * renamed to evd_canvas.
 *
 * evd [0] auto c = (TCanvas*)fWireProjP2->GetMother()
 * evd [1] c->SetName("evd_canvas")
 * evd [2] c->SaveAs("test_canvas.root")
**/
{ // gROOT->ForceStyle(1);

    int font = 43;

    float label_size = 13;
    float label_offset = 0.02;
    float title_size = 15;
    float pad_margin = 0.1;
    float pad_left_margin = 0.15;

    float c_height = 500;
    float c_width = 400;
    float pad_offset = 0.05;
    float pad_height = (1-pad_offset)/3.;
    float pad_width = 1.;

    // gStyle->SetOptStat(0);

    // gStyle->SetHistLineColor(kBlack);

    // gStyle->SetLabelSize(0.08, "xy");
    // gStyle->SetLabelOffset(0.02,"xy");


    // prepare names of subpads
    vector<TString> pad_names;
    FOR(i, 3) {
	pad_names.push_back(Form("fWireProjP%d", i));
    }


    // prepare name of output pdf file
    TString inname = fname;
    auto outname = inname;
    outname.ReplaceAll(".root", "_better.pdf");


    // open input file
    auto f = TFile::Open(inname, "read");
    auto c_in = (TCanvas*) f->Get("evd_canvas");

    // retrieve all 3 pads representing each view
    vector<TPad*> pads;
    for (auto name : pad_names)
	pads.push_back((TPad*)c_in->GetPrimitive(name));


    // create a new output canvas
    auto c = new TCanvas("c", "", 400, 500);

    // draw the pads
    int ipad = 0;
    for (auto pad: pads) {
	// make the pads to take 1/3 of the canvas height... leave
	// some margin for the bottom pad to add axis title
	pad -> SetPad(0., ipad*pad_height + pad_offset,
		      pad_width, (ipad+1)*pad_height + pad_offset);

	pad->Draw();

	// adjust pad margins
	pad->SetLeftMargin(pad_left_margin);
	if (ipad > 0)
	    pad->SetTopMargin(pad_margin);
	else
	    pad->SetTopMargin(0.01);

	// adjust plotted axes
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

	// deal with the bottom pad
	if (ipad == 0) {
	    h->GetXaxis()->SetTitle("Wire Segment");
	    pad -> SetPad(0., ipad*pad_height,
			  pad_width, (ipad+1)*pad_height + pad_offset);
	    pad->SetBottomMargin(0.2);
	}

	// insert title to vertical axis (centered vertically, hence using only the central pad)
	if (ipad == 1) {
	    h->GetYaxis()->SetTitle("TDC");
	}

	// remove track numbering from the pad
	for ( auto primitive: *pad->GetListOfPrimitives() ) {
	    if ( strstr(primitive->ClassName(), "TText") )
		pad->GetListOfPrimitives()->Remove(primitive);
	}

	// some debugging printouts
	cout<<"X: "<<pad->GetAbsXlowNDC()
	    <<", "<<pad->GetAbsWNDC()<<endl;
	cout<<"Y: "<<pad->GetAbsYlowNDC()
	    <<", "<<pad->GetAbsHNDC()<<endl;

	ipad++;
    }

    // draw the canvas and save in the output pdf
    c->Draw();
    c->SaveAs(outname);
}
