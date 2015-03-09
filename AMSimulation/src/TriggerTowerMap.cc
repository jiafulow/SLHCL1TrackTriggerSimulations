#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TriggerTowerMap.h"
using namespace slhcl1tt;

#include <algorithm>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

static const unsigned NTOWERS_ETA = 6;
static const unsigned NTOWERS_PHI = 8;

namespace {
bool ends_with(const std::string& value, const std::string & ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}
}


// _____________________________________________________________________________
TriggerTowerMap::TriggerTowerMap() {
    // Intentionally left blank
}

// _____________________________________________________________________________
void TriggerTowerMap::readTriggerTowerMap(TString csvfile) {

    if (!csvfile.EndsWith(".csv"))
        throw std::invalid_argument("Incorrect filename.");

    // Read trigger tower map
    std::string line, line2;
    std::ifstream ifs(csvfile.Data());  // open file

    ttmap_.clear();
    unsigned i = 0, tt = 0;

    while (std::getline(ifs, line)) {  // split by line break
        std::istringstream iss(line);

        if (i != 0) {  // skip the first line
            std::vector<unsigned> values;

            unsigned j = 0;
            while (std::getline(iss, line2, ',')) {  // split by comma
                if (j == 0) {
                    assert(int(tt / NTOWERS_PHI) + 1 == std::stoi(line2));
                } else if (j == 1) {
                    assert(int(tt % NTOWERS_PHI) + 1 == std::stoi(line2));
                } else {
                    values.push_back(std::stoi(line2));
                }
                ++j;
            }
            if (ifs.eof())
                break;

            assert(!values.empty() || iss.eof());
            assert(std::is_sorted(values.begin(), values.end()));
            ttmap_.insert(std::make_pair(tt, values));
            ++tt;
        }
        ++i;
    }
    assert(tt == NTOWERS_ETA * NTOWERS_PHI);

    // Make trigger tower reverse map
    //ttrmap_.clear();
    //for (std::map<unsigned, std::vector<unsigned> >::const_iterator itmap = ttmap_.begin();
    //     itmap != ttmap_.end(); ++itmap) {
    //
    //    for (std::vector<unsigned>::const_iterator itvec = itmap->second.begin();
    //         itvec != itmap->second.end(); ++itvec) {
    //        ttrmap_[*itvec].push_back(itmap->first);
    //    }
    //}
}

// _____________________________________________________________________________
void TriggerTowerMap::readTriggerTowerBoundaries(TString csvfile) {

    if (!csvfile.EndsWith(".csv"))
        throw std::invalid_argument("Incorrect filename.");

    // Read trigger tower map
    std::string line, line2;
    std::ifstream ifs(csvfile.Data());  // open file

    ttboundaries_.clear();
    unsigned i = 0, tt = 0;

    while (std::getline(ifs, line)) {  // split by line break
        std::istringstream iss(line);

        if (i != 0) {  // skip the first line
            unsigned id = 99;
            std::vector<float> values;

            unsigned j = 0;
            while (std::getline(iss, line2, ',')) {  // split by comma
                if (j == 0) {
                    if (int(tt) != std::stoi(line2))
                        ++tt;
                } else if (j == 1) {
                    id = std::stoi(line2);
                } else {
                    values.push_back(std::stof(line2));
                }
                ++j;
            }
            if (ifs.eof()) {
                ++tt;
                break;
            }

            assert(values.size() == 4 || iss.eof());

            LayerBounds lb;
            lb.set(id, values);
            assert(lb.phiMin <= lb.phiMax && lb.zMin <= lb.zMax);
            ttboundaries_[tt].push_back(lb);
        }
        ++i;
    }
    assert(tt == NTOWERS_ETA * NTOWERS_PHI);
}

// _____________________________________________________________________________
std::map<unsigned, bool> TriggerTowerMap::getTriggerTowerReverseMap(unsigned tt) const {
    std::map<unsigned, std::vector<unsigned> >::const_iterator found = ttmap_.find(tt);
    if (found == ttmap_.end())
        throw std::invalid_argument("Incorrect trigger tower.");

    std::map<unsigned, bool> reversemap;

    const std::vector<unsigned>& moduleIds = found->second;
    for (std::vector<unsigned>::const_iterator it = moduleIds.begin();
        it != moduleIds.end(); ++it) {
        reversemap[*it] = true;
    }
    return reversemap;
}

// _____________________________________________________________________________
std::vector<unsigned> TriggerTowerMap::getTriggerTowerModules(unsigned tt) const {
    std::map<unsigned, std::vector<unsigned> >::const_iterator found = ttmap_.find(tt);
    if (found == ttmap_.end())
        throw std::invalid_argument("Incorrect trigger tower.");

    return found->second;
}

// _____________________________________________________________________________
std::vector<LayerBounds> TriggerTowerMap::getTriggerTowerBoundaries(unsigned tt) const {
    std::map<unsigned, std::vector<LayerBounds> >::const_iterator found = ttboundaries_.find(tt);
    if (found == ttboundaries_.end())
        throw std::invalid_argument("Incorrect trigger tower.");

    return found->second;
}

// _____________________________________________________________________________
void TriggerTowerMap::print() {
    //for (auto it: ttmap_)
    //    std::cout << "Tower " << it.first << " has " << it.second.size() << " modules." << std::endl;

    //for (auto it: ttrmap_)
    //    std::cout << "Module " << it.first << " is in " << it.second.size() << " towers." << std::endl;

    //for (auto it: ttboundaries_)
    //    for (auto it2: it.second)
    //        std::cout << "Tower " << it.first << ", layer " << it2.layer
    //                << ", phi=[" << it2.phiMin << "," << it2.phiMax << "]"
    //                << ", z=[" << it2.zMin << "," << it2.zMax << "]." << std::endl;
}
