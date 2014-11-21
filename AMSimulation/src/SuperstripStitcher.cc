#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/SuperstripStitcher.h"
using namespace slhcl1tt;

#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>


SuperstripStitcher::SuperstripStitcher(unsigned nLayers, unsigned nFakers)
    : nLayers_(nLayers), nFakers_(nFakers),
      randEngine_(15428), randNumber_(0,65535) {

    if (!(6 <= nLayers_ && nLayers_ <= 8))
        throw std::invalid_argument("Incorrect nLayers is given.");
    if (!(nFakers_ <= 3))
        throw std::invalid_argument("Incorrect nFakers is given.");


    seven_choose_six_ = {  // 7 * 6 elements
        0, 1, 2, 3, 4, 5,
        0, 1, 2, 3, 4, 6,
        0, 1, 2, 3, 5, 6,
        0, 1, 2, 4, 5, 6,
        0, 1, 3, 4, 5, 6,
        0, 2, 3, 4, 5, 6,
        1, 2, 3, 4, 5, 6
    };

    eight_choose_six_ = {  // 8 * 7 / 2 * 6 elements
        0, 1, 2, 3, 4, 5,
        0, 1, 2, 3, 4, 6,
        0, 1, 2, 3, 4, 7,
        0, 1, 2, 3, 5, 6,
        0, 1, 2, 3, 5, 7,
        0, 1, 2, 3, 6, 7,
        0, 1, 2, 4, 5, 6,
        0, 1, 2, 4, 5, 7,
        0, 1, 2, 4, 6, 7,
        0, 1, 2, 5, 6, 7,
        0, 1, 3, 4, 5, 6,
        0, 1, 3, 4, 5, 7,
        0, 1, 3, 4, 6, 7,
        0, 1, 3, 5, 6, 7,
        0, 1, 4, 5, 6, 7,
        0, 2, 3, 4, 5, 6,
        0, 2, 3, 4, 5, 7,
        0, 2, 3, 4, 6, 7,
        0, 2, 3, 5, 6, 7,
        0, 2, 4, 5, 6, 7,
        0, 3, 4, 5, 6, 7,
        1, 2, 3, 4, 5, 6,
        1, 2, 3, 4, 5, 7,
        1, 2, 3, 4, 6, 7,
        1, 2, 3, 5, 6, 7,
        1, 2, 4, 5, 6, 7,
        1, 3, 4, 5, 6, 7,
        2, 3, 4, 5, 6, 7
    };

    eight_choose_seven_ = {  // 8 * 7 elements
        0, 1, 2, 3, 4, 5, 6,
        0, 1, 2, 3, 4, 5, 7,
        0, 1, 2, 3, 4, 6, 7,
        0, 1, 2, 3, 5, 6, 7,
        0, 1, 2, 4, 5, 6, 7,
        0, 1, 3, 4, 5, 6, 7,
        0, 2, 3, 4, 5, 6, 7,
        1, 2, 3, 4, 5, 6, 7
    };

    std::map<unsigned, unsigned> layermap_six = {
        {5,0}, {6,1}, {7,2}, {8,3}, {9,4}, {10,5},
        {11,5}, {12,4}, {13,3}, {14,2}, {15,1},
        {18,5}, {19,4}, {20,3}, {21,2}, {22,1}
    };

    std::map<unsigned, unsigned> layermap_seven = {
        {5,0}, {6,1}, {7,2}, {8,3}, {9,4}, {10,5},
        {11,6}, {12,5}, {13,4}, {14,3}, {15,2},
        {18,6}, {19,5}, {20,4}, {21,3}, {22,2}
    };

    std::map<unsigned, unsigned> layermap_eight = {
        {5,0}, {6,1}, {7,2}, {8,3}, {9,4}, {10,5},
        {11,7}, {12,6}, {13,5}, {14,4}, {15,3},
        {18,7}, {19,6}, {20,5}, {21,4}, {22,3}
    };

    // Assign
    if (nLayers_ == 6)
        layermap_.swap(layermap_six);
    else if (nLayers_ == 7)
        layermap_.swap(layermap_seven);
    else if (nLayers_ == 8)
        layermap_.swap(layermap_eight);
    assert(layermap_.size() == 16);
}

static std::vector<unsigned> ret;
std::vector<unsigned> SuperstripStitcher::stitch(const std::vector<unsigned>& layers) {
    // let n = nLayers_, f = nFakers_, k = # of input module ids
    // Case 1:        k <  n-f, return an empty vector
    // Case 2: n-f <= k <= n  , return [0,1,...,k-1] + [999999] * (n-k)
    // Case 3: n   <  k <= 8  , return a randomly picked combination
    // Case 4: 8   <  k       , go to Case 3 using the first 8 module ids

    // Case 1
    ret.clear();
    if (layers.size() < nLayers_ - nFakers_)
        return ret;

    // Case 2
    if (layers.size() <= nLayers_) {
        unsigned i;
        for (i=0; i<layers.size(); ++i)
            ret.push_back(i);
        for (; i<nLayers_; ++i)
            ret.push_back(999999);  // fakes
        return ret;
    }

    // Case 3, 4
    const std::vector<unsigned>* n_choose_k_ptr = NULL;
    if (layers.size() == 7 && nLayers_ == 6) {
        n_choose_k_ptr = &seven_choose_six_;
    } else if (layers.size() == 8 && nLayers_ == 6) {
        n_choose_k_ptr = &eight_choose_six_;
    } else if (layers.size() == 8 && nLayers_ == 7) {
        n_choose_k_ptr = &eight_choose_seven_;
    }
    assert(n_choose_k_ptr != NULL);

    unsigned ncombo = n_choose_k_ptr->size() / nLayers_;
    unsigned icombo = randNumber_(randEngine_) % ncombo;
    ret.insert(ret.end(), n_choose_k_ptr->at(icombo * nLayers_), n_choose_k_ptr->at((icombo+1) * nLayers_));
    return ret;
}

std::vector<unsigned> SuperstripStitcher::stitch_layermap(const std::vector<unsigned>& layers) {
    // let n = nLayers_, f = nFakers_, k = # of input module ids
    // Case 1:        k <  n-f, return an empty vector
    // Case 2: n-f <= k       , return [a_0, a_1, ..., a_n]
    //         where a_i is 999999 if no layers are in the layer
    //         or one of the randomly picked indices if there are >=1 layers
    // Case 2.1: due to merging, final number of a_i == 999999 can be
    //           greater than f. In which case, we treat it as Case 1

    // Case 1
    ret.clear();
    if (layers.size() < nLayers_ - nFakers_)
        return ret;

    // Case 2
    ret.resize(nLayers_, 999999);

    //unsigned lay, mlay;
    //for (unsigned i=0; i<layers.size(); ++i) {
    //    lay = layers.at(i);
    //    mlay = layermap_.at(lay);
    //    if (ret.at(mlay) == 999999) {
    //        ret.at(mlay) = i;
    //    } else {
    //        if (randNumber_(randEngine_) & 1)  // is odd
    //            ret.at(mlay) = i;
    //    }
    //}

    std::vector<unsigned> sortedLayers = layers;
    std::sort(sortedLayers.begin(), sortedLayers.end());
    unsigned lay, mlay;
    for (unsigned i=0; i<sortedLayers.size(); ++i) {
        lay = sortedLayers.at(i);
        mlay = layermap_.at(lay);
        ret.at(mlay) = i;  // always prefer endcap, as sortedLayers is used
    }

    // Case 2.1
    unsigned countFakers = std::count(ret.begin(), ret.end(), 999999);
    if (countFakers > nFakers_)
        ret.clear();

    return ret;
}

void SuperstripStitcher::print() {
    std::cout << "nLayers: " << nLayers_ << " nFakers: " << nFakers_ << std::endl;
}
