void saveEvdPads(const char* fname)
{
    //auto f = TFile::Open(fname, "recreate");

    //auto c = new TCanvas("c", "", 400, 500);

    // for (int i = 0; i < 3; i++) {
    // 	auto pad = (TPad*)gROOT->FindObject(Form("fWireProjP%d",i));
    // 	pad->Write();
    // }

    auto pad = (TPad*)gROOT->FindObject(Form("fWireProjP%d",0));
    auto c = (TCanvas*)pad->GetMother();

    c->SetName("evd_canvas");

    c->SaveAs(fname);
    //f->Close();
}
