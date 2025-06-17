#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_CFR_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_CFR_RP_HH__

#include "mem/cache/replacement_policies/base.hh"
#include "params/CFRRP.hh"

struct CFRRPParams;

class CFRRP : public BaseReplacementPolicy {
protected:
    /** Clean-First replacement data structure */
    struct CFRReplData : ReplacementData {
        /** Tick of last access */
        Tick lastTouchTick;

        /** Default constructor */
        CFRReplData() : lastTouchTick(0) {}
    };

public:
    /** Typedef for parameter struct */
    typedef CFRRPParams Params;

    /** Constructor */
    CFRRP(const Params* p);

    /** Destructor */
    ~CFRRP() {}

    /** Invalidate: no-op for CFR */
    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
        const override;

    /** Touch: update access time */
    void touch(const std::shared_ptr<ReplacementData>& replacement_data) const
        override;

    /** Reset: update last access time on insertion */
    void reset(const std::shared_ptr<ReplacementData>& replacement_data) const
        override;

    /** Select victim: prefer clean and LRU */
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const
        override;

    /** Instantiate a new replacement data entry */
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_CFR_RP_HH__
