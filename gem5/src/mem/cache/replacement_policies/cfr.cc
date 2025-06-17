#include "mem/cache/replacement_policies/cfr.hh"
#include "params/CFRRP.hh"
#include "base/logging.hh"
#include "mem/cache/base.hh"

CFRRP::CFRRP(const Params* p) : BaseReplacementPolicy(p)
{
}

void
CFRRP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // No specific invalidation logic needed
}

void
CFRRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::static_pointer_cast<CFRReplData>(replacement_data)->lastTouchTick = curTick();
}

void
CFRRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::static_pointer_cast<CFRReplData>(replacement_data)->lastTouchTick = curTick();
}

std::shared_ptr<ReplacementData>
CFRRP::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new CFRReplData());
}

ReplaceableEntry*
CFRRP::getVictim(const ReplacementCandidates& candidates) const
{
    assert(!candidates.empty());

    ReplaceableEntry* victim = nullptr;
    Tick oldestTick = std::numeric_limits<Tick>::max();

    // Step 1: Try to find oldest CLEAN block
    for (const auto& entry : candidates) {
	CacheBlk* blk = static_cast<CacheBlk*>(entry);
        if (!blk->isDirty()) {
            auto data = std::static_pointer_cast<CFRReplData>(
                entry->replacementData);
            if (data->lastTouchTick < oldestTick) {
                oldestTick = data->lastTouchTick;
                victim = entry;
            }
        }
    }

    if (victim != nullptr)
        return victim;

    // Step 2: Fallback to LRU among all (dirty) blocks
    oldestTick = std::numeric_limits<Tick>::max();
    for (const auto& entry : candidates) {
        auto data = std::static_pointer_cast<CFRReplData>(
            entry->replacementData);
        if (data->lastTouchTick < oldestTick) {
            oldestTick = data->lastTouchTick;
            victim = entry;
        }
    }

    assert(victim != nullptr);
    return victim;
}


CFRRP* CFRRPParams::create()
{
    return new CFRRP(this);
}
