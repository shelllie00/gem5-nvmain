#include "mem/cache/replacement_policies/fb_rp.hh"

#include <cassert>
#include <memory>

#include "params/FBRP.hh"

FBRP::FBRP(const Params* p)
    : BaseReplacementPolicy(p) {
}

void
FBRP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
const {
    // Reset reference count	
    std::static_pointer_cast<FBReplData>(replacement_data)->refCount = 0;
    // Reset last touch timestamp
    std::static_pointer_cast<FBReplData>(replacement_data)->lastTouchTick = Tick(0);
}

void
FBRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const {
    // Update reference count
    std::static_pointer_cast<FBReplData>(replacement_data)->refCount++;
    // Update last touch timestamp
    std::static_pointer_cast<FBReplData>(
        replacement_data)->lastTouchTick = curTick();
}

void
FBRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const {
    // Reset reference count
    std::static_pointer_cast<FBReplData>(replacement_data)->refCount = 1;
    // Set last touch timestamp
    std::static_pointer_cast<FBReplData>(
        replacement_data)->lastTouchTick = curTick();
}

ReplaceableEntry*
FBRP::getVictim(const ReplacementCandidates& candidates) const {
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    // Visit all candidates to find victim
    ReplaceableEntry* victim = candidates[0];
    for (const auto& candidate : candidates) {
        // Update victim entry if necessary
        if (std::static_pointer_cast<FBReplData>(
            candidate->replacementData)->refCount <
            std::static_pointer_cast<FBReplData>(
                victim->replacementData)->refCount) {
            victim = candidate;
        }
        else if (std::static_pointer_cast<FBReplData>(
            candidate->replacementData)->refCount ==
            std::static_pointer_cast<FBReplData>(
                victim->replacementData)->refCount &&
            std::static_pointer_cast<FBReplData>(
                candidate->replacementData)->lastTouchTick <
            std::static_pointer_cast<FBReplData>(
                victim->replacementData)->lastTouchTick) {
            victim = candidate;
        }
    }

    return victim;
}

std::shared_ptr<ReplacementData>
FBRP::instantiateEntry() {
    return std::shared_ptr<ReplacementData>(new FBReplData());
}

FBRP*
FBRPParams::create() {
    return new FBRP(this);
}
