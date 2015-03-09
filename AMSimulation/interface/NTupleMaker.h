#ifndef AMSimulation_NTupleMaker_h_
#define AMSimulation_NTupleMaker_h_

#include "TChain.h"
#include "TFile.h"
#include "TFileCollection.h"
#include "TROOT.h"
#include "TString.h"


// Reimplemented from SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleMaker.h
// without EDM structure

class NTupleMaker {
  public:
    // NOTE: STRING is not implemented
    enum LeafType {
        CHAR_T=1 , UCHAR_T   , SHORT_T   , USHORT_T   , INT_T   , UINT_T  ,
        FLOAT_T  , DOUBLE_T  , LONG64_T  , ULONG64_T  , BOOL_T  ,
        CHAR_V   , UCHAR_V   , SHORT_V   , USHORT_V   , INT_V   , UINT_V  ,
        FLOAT_V  , DOUBLE_V  , LONG64_V  , ULONG64_V  , BOOL_V  ,
        CHAR_VV  , UCHAR_VV  , SHORT_VV  , USHORT_VV  , INT_VV  , UINT_VV ,
        FLOAT_VV , DOUBLE_VV , LONG64_VV , ULONG64_VV , BOOL_VV ,
        CHAR_VVV , UCHAR_VVV , SHORT_VVV , USHORT_VVV , INT_VVV , UINT_VVV,
        FLOAT_VVV, DOUBLE_VVV, LONG64_VVV, ULONG64_VVV, BOOL_VVV,
        STRING,
        NumLeafTypes
    };

    // Constructor
    NTupleMaker()
    : nEvents_(999999999), trim_(true),
      verbose_(1) {

        makeLeafMap();
    }

    // Destructor
    ~NTupleMaker() {}


    // Setters
    void setNEvents(long long n)  { if (n != -1)  nEvents_ = n > 0 ? n : 0; }
    void setTrim(bool b)          { trim_ = b; }
    void setVerbosity(int v)      { verbose_ = v; }

    // Getters
    // none

    // Functions
    int readRoads(TString src);

    int readTracks(TString src);

    int readFile(TString src);

    int writeTree(TString out);

    // Main driver
    int run(TString src, TString roadfile, TString trackfile, TString out);

  private:
    // Connect branch
    class BranchConnector {
      public:
        BranchConnector() {}
        virtual ~BranchConnector() {};
        virtual void connect(TTree*) = 0;
    };

    template <class T>
    class TypedBranchConnector : public BranchConnector {
      public:
        TypedBranchConnector(TString, TString, TTree*);
        void connect(TTree*);

      private:
        TString instanceName_;
        T object_;
        T* ptr_object_;
    };

    // Private functions
    void makeLeafMap();
    void makeConnector(const TBranch*, TTree*);


  private:
    // Program options
    long long nEvents_;
    bool trim_;  // do not keep every branch
    int verbose_;

    // Containers
    TChain * chain_;
    TChain * chain_roads_;
    TChain * chain_tracks_;

    std::map<TString, LeafType> leafmap;
    std::vector<BranchConnector*> connectors;
};

#endif
