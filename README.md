

## 2025 計算機組織 Final Project 
### 評分標準
- [x] (Q1) GEM5 + NVMAIN BUILD-UP (40%)

- [x] (Q2) Enable L3 last level cache in GEM5 + NVMAIN (15%)
（看到 log 裡面有 L3 cache 的資訊）

- [x] (Q3) Config last level cache to  2-way and full-way associative cache and test performance (15%)
必須跑 benchmark quicksort 在 2-way 跟 full way（直接在 L3 cache implement，可以用 miss rate 判斷是否成功）
 
- [x] (Q4) Modify last level cache policy based on frequency based replacement policy (15%)
 
- [x] (Q5) Test the performance of write back and write through policy based on 4-way associative cache with isscc_pcm(15%) 
必須跑 benchmark multiply 在 write through 跟 write back ( gem5 default 使用 write back，可以用 write request 的數量判斷 write through 是否成功 )
 
- [ ] Bonus (10%) Design last level cache policy to reduce the energy consumption of pcm_based main memory
 Baseline:LRU
 
- - -
### 繳交方式
* (Q3) 直接在l3 cache implement，並繳交兩份 log 檔案，一份是 2-way， 一份是full-way，需在 log 檔裡看到l3 cache 的資訊
* (Q4) 繳交兩份log 檔案，一份是原始policy，一份是frequency based policy
* (Q5) 繳交兩份log 檔案，一份write through，一份是 write back
* (bonus) 繳交兩份 log 檔案，一份是baseline (hello)，一份是修改過後的方法
* log 檔案包含m5out 中的 stat.txt 檔案，以及 nvmain 輸出的 log

- - -

### 詳細步驟
#### (Q1) GEM5 + NVMAIN BUILD-UP (40%)

1. 就跟著給的教學做
:::info
可以多給一點 thread，不然我只用 2 個 build 很久，大概一個多小時。但只有這次 build 會最久，後面都蠻快的
:::
2. 然後混合編譯
<pre style="white-space: pre-wrap;">scons EXTRAS=../NVmain build/X86/gem5.opt   </pre>
    
3. 可以跑一下 hello 確認
<pre style="white-space: pre-wrap;">./build/X86/gem5.opt configs/example/se.py -c tests/test-progs/hello/bin/x86/linux/hello --cpu-type=TimingSimpleCPU --caches --l2cache --mem-type=NVMainMemory --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config</pre>
    
會有一行 Hello world!
   - - - 
    
#### (Q2) Enable L3 last level cache in GEM5 + NVMAIN (15%) 

:::info
需要修改的檔案在gem5資料夾中的
(1) Options.py
(2) Caches.py
(3) Xbar.py
(4) BaseCPU.py
(5) CacheConfig.py
前四個檔案只是增加 L3 cache 的 parameter ，照著 L2 cache 的設定去做模仿就可以。CacheConfig.py 需要讓 L3 cache 連接整個 Gem5 系統，這邊要注意 L2 跟 L3 這兩個 cache 的關係，要讓系統在已使用 L2 cache 的情況下才能使用 L3 cache，所以要注意修改的時候有沒有滿足這個條件
\- from Final_Project_說明.pdf
:::

1. 修改 gem5/configs/common/Caches.py
        
    * 在 class L2Cache 下新增一個 class L3Cache
    ```py
    class L3Cache(Cache):
        assoc = 8
        tag_latency = 20
        data_latency = 20
        response_latency = 20
        mshrs = 20
        tgts_per_mshr = 12
        write_buffers = 8
    ```
        
2. 修改 /gem5/configs/common/CacheConfig.py
        
    * 改第一部分，加上l3的資訊
        ```py
            dcache_class, icache_class, l2_cache_class, l3_cache_class, walk_cache_class = \
            O3_ARM_v7a_DCache, O3_ARM_v7a_ICache, O3_ARM_v7aL2, \
            O3_ARM_v7aL3, O3_ARM_v7aWalkCache
        else:
            dcache_class, icache_class, l2_cache_class, l3_cache_class, walk_cache_class = \
            L1_DCache, L1_ICache, L2Cache, L3Cache, None
        ```
        
        
    * 改第二部分，加上 options.L3cache 
        ```py
        if options.l2cache and options.l3cache:
            # Provide a clock for the L2 and the L1-to-L2 bus here as they
            # are not connected using addTwoLevelCacheHierarchy. Use the
            # same clock as the CPUs.
            system.l2 = l2_cache_class(clk_domain=system.cpu_clk_domain,
                                       size=options.l2_size,
                                       assoc=options.l2_assoc)
            system.l3 = l3_cache_class(clk_domain=system.cpu_clk_domain,
                                       size=options.l3_size,
                                       assoc=options.l3_assoc)

            system.tol2bus = L2XBar(clk_domain = system.cpu_clk_domain)
            system.tol3bus = L3XBar(clk_domain = system.cpu_clk_domain)

            system.l2.cpu_side = system.tol2bus.master
            system.l2.mem_side = system.tol3bus.slave

            system.l3.cpu_side = system.tol3bus.master
            system.l3.mem_side = system.membus.slave

        elif options.l2cache:
            system.l2 = l2_cache_class(clk_domain=system.cpu_clk_domain,
                                       size=options.l2_size,
                                       assoc=options.l2_assoc)
            system.tol2bus = L2XBar(clk_domain = system.cpu_clk_domain)
            system.l2.cpu_side = system.tol2bus.master
            system.l2.mem_side = system.membus.slave
        ```        
        
        
3. 修改 gem5/src/mem/XBar.py
    * 在 class L2XBar 下面直接複製一個改成 class L3XBar
        ```py
        class L3XBar(CoherentXBar):
            # 256-bit crossbar by default
            width = 32

            # Assume that most of this is covered by the cache latencies, with
            # no more than a single pipeline stage for any packet.
            frontend_latency = 1
            forward_latency = 0
            response_latency = 1
            snoop_response_latency = 1
        ```
        
      
        
4. 修改 /gem5/src/cpu/BaseCPU.py
    * from XBar import L2XBar 下面新增一行 import L3XBar
    ```py
    from XBar import L3XBar
    ```
    * def addTwoLevelCacheHierarchy下面新增一段 def addThreeLevelCacheHierarchy
    ```py
    def addThreeLevelCacheHierarchy(self, ic, dc, l3c, iwc=None, dwc=None):
        self.addPrivateSplitL1Caches(ic, dc, iwc, dwc)
        self.toL3Bus = L3XBar()
        self.connectCachedPorts(self.toL3Bus)
        self.l3cache = l3c
        self.toL2Bus.master = self.l3cache.cpu_side
        self._cached_ports = ['l3cache.mem_side']
    ```
        
        
5. 修改 /gem5/configs/common/Options.py
        
    * 新增 parser.add_option(”—l3cache”, action = “store_true”)
    ```py
    parser.add_option("--l3cache", action="store_true")
    ```
        
       
        
6. 以上都改完後，在 /gem5 再編譯一次
        
    ```
    scons EXTRAS=../NVmain build/X86/gem5.opt
    ```

7. 然後以 Gem5 測試執行檔時要記得加上 --l3cache 以啟用 L3
        
    ```
    ./build/X86/gem5.opt configs/example/se.py -c tests/test-progs/hello/bin/x86/linux/hello --cpu-type=TimingSimpleCPU --caches --l2cache --l3cache --mem-type=NVMainMemory --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config
    ```
        
8. 查看 /m5out/stats.txt 裡面是否有 l3 的資訊
    ```
    system.l3.ReadSharedReq_hits::total                 7                       # number of ReadSharedReq hits
    system.l3.demand_hits::cpu.data                    11                       # number of demand (read+write) hits
    system.l3.demand_hits::total                       11                       # number of demand (read+write) hits
    system.l3.overall_hits::cpu.data                   11                       # number of overall hits
    system.l3.overall_hits::total                      11                       # number of overall hits
    ```
- - -         
    
#### (Q3) Config last level cache to  2-way and full-way associative cache and test performance (15%)
      
1. 先到 /benchmark 裡編譯 quicksort.c       
    ```
    x86_64-linux-gnu-gcc -static -o quicksort quicksort.c
    ```
        
2. 跑 2-way quicksort
    ```
    ./build/X86/gem5.opt configs/example/se.py -c ../benchmark/quicksort \
        --cpu-type=TimingSimpleCPU --caches --l2cache --l3cache \
        --l3_assoc=2 --l3_size=64kB \
        --l1i_size=32kB --l1d_size=32kB \
        --l2_size=128kB \
        --mem-type=NVMainMemory \
        --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config > log_2way.txt
    ```
        
    * 複製一份 /m5out 到新資料夾 /m5out_2way
        
    ```
    cp -r m5out m5out_2way
    ```
        
3. 跑 full-way quicksort
    * 和上面一樣，只要把改成 l3_assoc=1024
    ```
    ./build/X86/gem5.opt configs/example/se.py -c ../benchmark/quicksort \
        --cpu-type=TimingSimpleCPU --caches --l2cache --l3cache \
        --l3_assoc=1024 --l3_size=64kB \
        --l1i_size=32kB --l1d_size=32kB \
        --l2_size=128kB \
        --mem-type=NVMainMemory \
        --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config > log_fullway.txt
    ```    
    * 一樣複製 /m5out 到 /m5out_fullway
        
        ```
        cp -r m5out m5out_fullway
        ```
        
4. 結果看 m5out/stats.txt
        
    * m5out/config.ini 可以找 l3 看 assoc 是不是正確的
    * m5out/stats.txt 找 system.l3.overall_miss_rate::total 看 miss rate
    * 應該是 2-way 的 miss rate 會比 full-way 小
    * 如果兩個數值都一樣，可以把 l3_size 縮小一點再跑看看
    
- - -         
    
#### (Q4) Modify last level cache policy based on frequency based replacement policy (15%)
需要自己手刻 Frequency based replacement policy
:::info
Frequency LRU: 每個 block 有自己的 F 值，每 access 一次就 F++，沒位置取代 F 值最小的，若有相同 F 值，取代最老的 block
:::

1. 建立 frequency_rp.hh 和 frequency_rp.cc
    * 到 src/mem/cache/replacement_policies/ 建立 frequency_rp.hh 和 frequency_rp.cc
        
    * frequency_rp.hh 程式碼：
        ```cpp
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
        ```
        
    * frequency_rp.cc 程式碼：
        
        ```cpp
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
        ```
2. 加入 source('frequency_rp.cc')
        
    * 到 src/mem/cache/replacement_policies/SConscript 加入 Source('frequency_rp.cc')
    ```
    Source('fb_rp.cc')
    ```
        
3. 同個資料夾下修改 ReplacementPolicies.py
        
    * 新增 class FrequencyRP
    ```py
    class FBRP(BaseReplacementPolicy):
    type = 'FBRP'
    cxx_class = 'FBRP'
    cxx_header = "mem/cache/replacement_policies/fb_rp.hh"
    ```
        
4. 修改 /configs/common/cache.py
        
    * L3Cache 裡面加 replacement_policy = 
      ```py
      class L3Cache(Cache):
        assoc = 8
        tag_latency = 20
        data_latency = 20
        response_latency = 20
        mshrs = 20
        tgts_per_mshr = 12
        write_buffers = 8
        replacement_policy = Param.BaseReplacementPolicy(FBRP(),"Replacement policy")
      ```
5. 再編譯一次
        
    ```
    scons EXTRAS=../NVmain build/X86/gem5.opt
    ```
        
6. 跑quicksort 並 > log_fbrp.txt
    ```
    ./build/X86/gem5.opt configs/example/se.py -c ../benchmark/quicksort --cpu-type=TimingSimpleCPU --caches --l2cache --l3cache --l3_assoc=2 --l3_size=64kB --l1i_size=32kB --l1d_size=32kB --l2_size=128kB --mem-type=NVMainMemory --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config
    ```
    * 複製 m5out 到 m5out_fbrp
    ```jsx
    cp -r m5out m5out_fbrp
    ```
- - -

#### (Q5) Test the performance of write back and write through policy based on 4-way associative cache with isscc_pcm(15%)

:::info
在 /src/mem/cache 底下有 [cache.cc](http://cache.cc) 和 [base.cc](http://base.cc) 可以研究一下 write back 是怎麼做的
\- from Final_Project_說明.pdf
:::
        
1. 先編譯 multiply.c        
    ```
    x86_64-linux-gnu-gcc -static -o multiply multiply.c
    ```
        
2. 4way 跑 multiply、複製 m5out        
    ```
    ./build/X86/gem5.opt configs/example/se.py -c ../benchmark/multiply \ 
        --cpu-type=TimingSimpleCPU --caches --l2cache --l3cache \
        --l1i_size=32kB --l1d_size=32kB --l1d_assoc=4 --l1i_assoc=4 \
        --l2_size=128kB --l2_assoc=4 \
        --l3_size=64kB --l3_assoc=4 \
        --mem-type=NVMainMemory \
        --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config > log_writeback.txt 
    ```
        
    ```
    cp -r m5out m5out_writeback
    ```
        
    * 跑完有一行會寫 WriteRequest 的數量，或進 log_writeback.txt 裡面看
        
    ```
    i0.defaultMemory.totalWriteRequests 12
    ```
        
3. 改成 Write Through：改 /src/mem/cache/base.cc
    * 加上底下這串
        
        ```cpp
         else if (blk && (pkt->needsWritable() ? blk->isWritable() :
                       blk->isReadable())) {
            // OK to satisfy access
            incHitCount(pkt);
            satisfyRequest(pkt, blk);
            maintainClusivity(pkt->fromCache(), blk);


            // modify to write through
            if (blk->isWritable()){
                PacketPtr writeclean_pkt = writecleanBlk(blk, pkt->req->getDest(), pkt->id);
                writebacks.push_back(writeclean_pkt);
            }
            // end modify to write through

            return true;
        }
        ```       
        （把 write back 的 cache block 在 cache 寫入時直接加入要寫回 memory 的清單）
        
4. 編譯 again
        
    ```
    scons EXTRAS=../NVmain build/X86/gem5.opt
    ```
     
5. 改成 Write Through 後跑 multiply
        
    ```
    ./build/X86/gem5.opt configs/example/se.py -c ../benchmark/multiply \ 
        --cpu-type=TimingSimpleCPU --caches --l2cache --l3cache \
        --l1i_size=32kB --l1d_size=32kB --l1d_assoc=4 --l1i_assoc=4 \
        --l2_size=128kB --l2_assoc=4 \
        --l3_size=64kB --l3_assoc=4 \
        --mem-type=NVMainMemory \
        --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config > log_writethrough.txt 
    ```

    ```
    cp -r m5out m5out_writethrough
    ```
        
6. 比較一下 wb 和 wt（/result/q5 這裡的數據是跑 SIZE 100 所以比較小，result/q5_new 是跑 SIZE 300）
     ![image](https://github.com/user-attachments/assets/2966c43a-9b6b-477d-b940-5c05ffeb5acf)


   
---
#### Bonus (10%) Design last level cache policy to reduce the energy consumption of pcm_based main memory Baseline:LRU
* 還沒做
