#include "EventDisplay.h"
#include "anatree.h"

int next(int entry = -1);

EventDisplay* gEvd = 0;
int gEntry = 0;
TTree* gTree = 0;
anatree* gEvent = 0;

int Scan(const char* fname, int entry = 0)
{
    if (!gEvd) {
	TChain *tree = new TChain("analysistree/anatree");
	tree->Add(fname);
	gEvent = new anatree(tree);

	gEvd = new EventDisplay(gEvent);
	gTree = tree;
    }

    next(entry);

    return 0;
}


int next(int entry)
{
    // loop over event until found one with true K length > 3cm and no
    // reco track
    if (entry != -1)
	gEntry = entry;

    /*
    while(true) {
	gEvd->GetEntry(gEntry++);

	// find true kaon
	int Kid = -10;
	int Kidx = -1;
	// find track id of the kaon
	for (int i = 0; i < gEvent->mc_npart; i++) {
	    if (gEvent->mc_pdg[i] == 321) {
		Kid = gEvent->mc_id[i];
		Kidx = i;
		break;
	    }
	}
	if (Kidx < 0) //did not find Kaon (bkg sample)
	    return -1;

	// how long is the track?
	if ( gEvent->mc_truthlength[Kidx] < 3. )
	    continue;

	break;
    }
    */
    gEvd->GetEntry(gEntry++);
    gEvd->plot();
    gEvd->PrintInfo();

    return 0;
}
