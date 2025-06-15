./build/X86/gem5.opt configs/example/se.py -c ../benchmark/quicksort \
    --cpu-type=TimingSimpleCPU --caches --l2cache --l3cache \
    --l3_assoc=2 --l3_size=64kB \
    --l1i_size=32kB --l1d_size=32kB \
    --l2_size=128kB \
    --mem-type=NVMainMemory \
    --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config > log_2way.txt


./build/X86/gem5.opt configs/example/se.py -c ../benchmark/quicksort \
    --cpu-type=TimingSimpleCPU --caches --l2cache --l3cache \
    --l3_assoc=1024 --l3_size=64kB \
    --l1i_size=32kB --l1d_size=32kB \
    --l2_size=128kB \
    --mem-type=NVMainMemory \
    --nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config > log_fullway.txt

    
