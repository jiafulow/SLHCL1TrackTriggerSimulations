#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTRoadReader.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/Helper.h"
using namespace slhcl1tt;


// _____________________________________________________________________________
TTRoadReader::TTRoadReader(int verbose)
: vr_nSuperstrips      (0),
  vr_bankIndex         (0),
  //
  vr_hitRs             (0),
  vr_hitPhis           (0),
  vr_hitZs             (0),
  vr_hitRErrors        (0),
  vr_hitPhiErrors      (0),
  vr_hitZErrors        (0),
  vr_hitClusWidths     (0),
  vr_hitStubWidths     (0),
  vr_hitSuperstripIds  (0),
  vr_hitTrkIds         (0),
  //
  vp_pt                (0),
  vp_eta               (0),
  vp_phi               (0),
  vp_vx                (0),
  vp_vy                (0),
  vp_vz                (0),
  vp_charge            (0),
  //
  verbose_(verbose) {}

TTRoadReader::~TTRoadReader() {
    if (tchain)  delete tchain;
}

int TTRoadReader::init(TString src, TString prefix, TString suffix) {
    if (!src.EndsWith(".root") && !src.EndsWith(".txt")) {
        std::cout << Error() << "Input source must be either .root or .txt" << std::endl;
        return 1;
    }

    if (verbose_)  std::cout << Info() << "Opening " << src << std::endl;
    tchain = new TChain("ntupler/tree");

    if (src.EndsWith(".root")) {
        if (tchain->Add(src) ) {
            if (verbose_)  std::cout << Info() << "Successfully read " << src << std::endl;
        } else {
            std::cout << Error() << "Failed to read " << src << std::endl;
            return 1;
        }

    } else if (src.EndsWith(".txt")) {
        TFileCollection fc("fileinfolist", "", src);
        if (tchain->AddFileInfoList((TCollection*) fc.GetList()) ) {
            if (verbose_)  std::cout << Info() << "Successfully read " << src << std::endl;
        } else {
            std::cout << Error() << "Failed to read " << src << std::endl;
            return 1;
        }
    }

    assert(tchain != 0);
    treenumber = tchain->GetTreeNumber();

    tchain->SetBranchAddress(prefix + "nSuperstrips"      + suffix, &(vr_nSuperstrips));
    tchain->SetBranchAddress(prefix + "bankIndex"         + suffix, &(vr_bankIndex));
    //
    tchain->SetBranchAddress(prefix + "hitRs"             + suffix, &(vr_hitRs));
    tchain->SetBranchAddress(prefix + "hitPhis"           + suffix, &(vr_hitPhis));
    tchain->SetBranchAddress(prefix + "hitZs"             + suffix, &(vr_hitZs));
    tchain->SetBranchAddress(prefix + "hitRErrors"        + suffix, &(vr_hitRErrors));
    tchain->SetBranchAddress(prefix + "hitPhiErrors"      + suffix, &(vr_hitPhiErrors));
    tchain->SetBranchAddress(prefix + "hitZErrors"        + suffix, &(vr_hitZErrors));
    tchain->SetBranchAddress(prefix + "hitClusWidths"     + suffix, &(vr_hitClusWidths));
    tchain->SetBranchAddress(prefix + "hitStubWidths"     + suffix, &(vr_hitStubWidths));
    tchain->SetBranchAddress(prefix + "hitSuperstripIds"  + suffix, &(vr_hitSuperstripIds));
    tchain->SetBranchAddress(prefix + "hitTrkIds"         + suffix, &(vr_hitTrkIds));
    //
    tchain->SetBranchAddress("genParts_pt"      , &(vp_pt));
    tchain->SetBranchAddress("genParts_eta"     , &(vp_eta));
    tchain->SetBranchAddress("genParts_phi"     , &(vp_phi));
    tchain->SetBranchAddress("genParts_vx"      , &(vp_vx));
    tchain->SetBranchAddress("genParts_vy"      , &(vp_vy));
    tchain->SetBranchAddress("genParts_vz"      , &(vp_vz));
    tchain->SetBranchAddress("genParts_charge"  , &(vp_charge));
    return 0;
}

Long64_t TTRoadReader::loadTree(Long64_t entry) {
    //if (!tchain) return -5;
    return tchain->LoadTree(entry);
}

Int_t TTRoadReader::getEntry(Long64_t entry) {
   //if (!tchain) return 0;
   return tchain->GetEntry(entry);
}


// _____________________________________________________________________________
TTRoadWriter::TTRoadWriter(int verbose)
: vr_nSuperstrips     (new std::vector<count_type>()),
  vr_bankIndex        (new std::vector<id_type>()),
  //
  vr_hitRs            (new std::vector<std::vector<float> >()),
  vr_hitPhis          (new std::vector<std::vector<float> >()),
  vr_hitZs            (new std::vector<std::vector<float> >()),
  vr_hitRErrors       (new std::vector<std::vector<float> >()),
  vr_hitPhiErrors     (new std::vector<std::vector<float> >()),
  vr_hitZErrors       (new std::vector<std::vector<float> >()),
  vr_hitClusWidths    (new std::vector<std::vector<float> >()),
  vr_hitStubWidths    (new std::vector<std::vector<float> >()),
  vr_hitSuperstripIds (new std::vector<std::vector<id_type> >()),
  vr_hitTrkIds        (new std::vector<std::vector<int> >()),
  //
  vp_pt               (new std::vector<float>()),
  vp_eta              (new std::vector<float>()),
  vp_phi              (new std::vector<float>()),
  vp_vx               (new std::vector<float>()),
  vp_vy               (new std::vector<float>()),
  vp_vz               (new std::vector<float>()),
  vp_charge           (new std::vector<int>()),
  //
  verbose_(verbose) {}

TTRoadWriter::~TTRoadWriter() {
    if (ttree)  delete ttree;
    if (tfile)  delete tfile;
}

int TTRoadWriter::init(TString out, TString prefix, TString suffix) {
    gROOT->ProcessLine("#include <vector>");  // how is it not loaded?

    if (!out.EndsWith(".root")) {
        std::cout << Error() << "Output filename must be .root" << std::endl;
        return 1;
    }

    if (verbose_)  std::cout << Info() << "Opening " << out << std::endl;
    tfile = TFile::Open(out, "RECREATE");

    if (tfile) {
        if (verbose_)  std::cout << Info() << "Successfully recreated " << out << std::endl;
    } else {
        std::cout << Error() << "Failed to recreate " << out << std::endl;
        return 1;
    }

    tfile->mkdir("ntupler")->cd();
    ttree = new TTree("tree", "");

    ttree->Branch(prefix + "nSuperstrips"      + suffix, &(*vr_nSuperstrips));
    ttree->Branch(prefix + "bankIndex"         + suffix, &(*vr_bankIndex));
    //
    ttree->Branch(prefix + "hitRs"             + suffix, &(*vr_hitRs));
    ttree->Branch(prefix + "hitPhis"           + suffix, &(*vr_hitPhis));
    ttree->Branch(prefix + "hitZs"             + suffix, &(*vr_hitZs));
    ttree->Branch(prefix + "hitRErrors"        + suffix, &(*vr_hitRErrors));
    ttree->Branch(prefix + "hitPhiErrors"      + suffix, &(*vr_hitPhiErrors));
    ttree->Branch(prefix + "hitZErrors"        + suffix, &(*vr_hitZErrors));
    ttree->Branch(prefix + "hitClusWidths"     + suffix, &(*vr_hitClusWidths));
    ttree->Branch(prefix + "hitStubWidths"     + suffix, &(*vr_hitStubWidths));
    ttree->Branch(prefix + "hitSuperstripIds"  + suffix, &(*vr_hitSuperstripIds));
    ttree->Branch(prefix + "hitTrkIds"         + suffix, &(*vr_hitTrkIds));
    //
    ttree->Branch("genParts_pt"    , &(*vp_pt));
    ttree->Branch("genParts_eta"   , &(*vp_eta));
    ttree->Branch("genParts_phi"   , &(*vp_phi));
    ttree->Branch("genParts_vx"    , &(*vp_vx));
    ttree->Branch("genParts_vy"    , &(*vp_vy));
    ttree->Branch("genParts_vz"    , &(*vp_vz));
    ttree->Branch("genParts_charge", &(*vp_charge));
    return 0;
}

void TTRoadWriter::fill(const std::vector<TTRoad>& roads, const std::vector<GenParticle>& genParts) {
    vr_nSuperstrips     ->clear();
    vr_bankIndex        ->clear();
    //
    vr_hitRs            ->clear();
    vr_hitPhis          ->clear();
    vr_hitZs            ->clear();
    vr_hitRErrors       ->clear();
    vr_hitPhiErrors     ->clear();
    vr_hitZErrors       ->clear();
    vr_hitClusWidths    ->clear();
    vr_hitStubWidths    ->clear();
    vr_hitSuperstripIds ->clear();
    vr_hitTrkIds        ->clear();
    //
    vp_pt               ->clear();
    vp_eta              ->clear();
    vp_phi              ->clear();
    vp_vx               ->clear();
    vp_vy               ->clear();
    vp_vz               ->clear();
    vp_charge           ->clear();

    const unsigned nroads = roads.size();
    for (unsigned i=0, j=0; i<nroads; ++i) {
        hitRs            .clear();
        hitPhis          .clear();
        hitZs            .clear();
        hitRErrors       .clear();
        hitPhiErrors     .clear();
        hitZErrors       .clear();
        hitClusWidths    .clear();
        hitStubWidths    .clear();
        hitSuperstripIds .clear();
        hitTrkIds        .clear();

        const TTRoad&             road = roads.at(i);
        const std::vector<TTHit>& hits = road.getHits();

        for (j=0; j<hits.size(); ++j) {
            const TTHit& hit = hits.at(j);
            hitRs.push_back(hit.r);
            hitPhis.push_back(hit.phi);
            hitZs.push_back(hit.z);
            hitRErrors.push_back(hit.rError);
            hitPhiErrors.push_back(hit.phiError);
            hitZErrors.push_back(hit.zError);
            hitClusWidths.push_back(hit.clusWidth);
            hitStubWidths.push_back(hit.stubWidth);
            hitSuperstripIds.push_back(hit.superstripId);
            hitTrkIds.push_back(hit.trkId);
        }

        vr_nSuperstrips     ->push_back(road.nSuperstrips());
        vr_bankIndex        ->push_back(road.bankIndex());
        //
        vr_hitRs            ->push_back(hitRs);
        vr_hitPhis          ->push_back(hitPhis);
        vr_hitZs            ->push_back(hitZs);
        vr_hitRErrors       ->push_back(hitRErrors);
        vr_hitPhiErrors     ->push_back(hitPhiErrors);
        vr_hitZErrors       ->push_back(hitZErrors);
        vr_hitClusWidths    ->push_back(hitClusWidths);
        vr_hitStubWidths    ->push_back(hitStubWidths);
        vr_hitSuperstripIds ->push_back(hitSuperstripIds);
        vr_hitTrkIds        ->push_back(hitTrkIds);
    }

    const unsigned nparts = genParts.size();
    for (unsigned i=0; i<nparts; ++i) {
        const GenParticle& part = genParts.at(i);
        vp_pt    ->push_back(part.pt);
        vp_eta   ->push_back(part.eta);
        vp_phi   ->push_back(part.phi);
        vp_vx    ->push_back(part.vx);
        vp_vy    ->push_back(part.vy);
        vp_vz    ->push_back(part.vz);
        vp_charge->push_back(part.charge);
    }

    ttree->Fill();
    assert(vr_hitRs->size() == nroads);
    assert(vp_pt->size() == nparts);
}

Long64_t TTRoadWriter::write() {
    Long64_t nentries = ttree->GetEntries();
    tfile->Write();
    //tfile->Close();
    return nentries;
}

