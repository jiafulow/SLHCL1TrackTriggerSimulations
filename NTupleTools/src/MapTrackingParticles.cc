#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/MapTrackingParticles.h"

void MapTrackingParticles::setup(const edm::Handle<TrackingParticleCollection>& tphandle) {
    if (tphandle.isValid()) {
        for (unsigned itp=0; itp<tphandle->size(); ++itp) {
            const TrackingParticleRef tpref(tphandle, itp);
            for (TrackingParticle::g4t_iterator itrk = tpref->g4Track_begin(); itrk != tpref->g4Track_end(); ++itrk) {
                const MixedSimTrackId trkId(itrk->trackId(), tpref->eventId());
                mapping.insert(std::make_pair(trkId, tpref));
            }
        }
    }
}

int MapTrackingParticles::get(const unsigned trackId, const EncodedEventId eventId) {
    const MixedSimTrackId trkId(trackId, eventId);
    std::map<MixedSimTrackId, TrackingParticleRef>::const_iterator found = mapping.find(trkId);
    if (found != mapping.end()) {
        return found->second.key();
    }
    return -1;
}

