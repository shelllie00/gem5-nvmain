#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_FB_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_FB_RP_HH__

#include "mem/cache/replacement_policies/base.hh"

struct FBRPParams;

class FBRP : public BaseReplacementPolicy {
protected:
    /** FB-specific implementation of replacement data. */
    struct FBReplData : ReplacementData {
        /** Number of references to this entry since it was reset. */
        unsigned refCount;
        /** Tick on which the entry was last touched. */
        Tick lastTouchTick;

        /**
         * Default constructor. Invalidate data.
         */
        FBReplData() : refCount(0), lastTouchTick(0) {}
    };

public:
    /** Convenience typedef. */
    typedef FBRPParams Params;

    /**
     * Construct and initiliaze this replacement policy.
     */
    FBRP(const Params* p);

    /**
     * Destructor.
     */
    ~FBRP() {}

    /**
     * Invalidate replacement data to set it as the next probable victim.
     * Clear the number of references.
     *
     * @param replacement_data Replacement data to be invalidated.
     */
    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
        const override;

    /**
     * Touch an entry to update its replacement data.
     * Increase number of references.
     *
     * @param replacement_data Replacement data to be touched.
     */
    void touch(const std::shared_ptr<ReplacementData>& replacement_data) const
        override;

    /**
     * Reset replacement data. Used when an entry is inserted.
     * Reset number of references.
     *
     * @param replacement_data Replacement data to be reset.
     */
    void reset(const std::shared_ptr<ReplacementData>& replacement_data) const
        override;

    /**
     * Find replacement victim using reference frequency.
     *
     * @param cands Replacement candidates, selected by indexing policy.
     * @return Replacement entry to be replaced.
     */
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const
        override;

    /**
     * Instantiate a replacement data entry.
     *
     * @return A shared pointer to the new replacement data.
     */
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_FB_RP_HH__
