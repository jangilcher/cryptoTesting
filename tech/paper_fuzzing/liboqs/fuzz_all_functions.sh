# DIRNAME?=cur_liboqs
# DIRNAME?=mid_liboqs
# DIRNAME?=old_liboqs

PATHS_FOR_THREE=KEM/Decaps/c:KEM/Decaps/sk:KEM/Encaps/pk:KEM/Encaps/badrng:KEM/Keygen/badrng:SIGN/Keygen/badrng
PATHS_FOR_TWO=SIGN/Sign/badrng:SIGN/Sign/m:SIGN/Sign/sk:SIGN/Verify/m:SIGN/Verify/sig

CUR_CPU=0

(
    IFS=:
    for p in $PATHS_FOR_THREE; do
        (
            cd $p
            DIRNAME=liboqs taskset -c $CUR_CPU make clean all
            DIRNAME=liboqs taskset -c $CUR_CPU python3 run_all.py current
            touch finished_current
            DIRNAME=mid_liboqs taskset -c $CUR_CPU make clean all
            DIRNAME=mid_liboqs taskset -c $CUR_CPU python3 run_all.py loqs_0.4.0
            touch finished_mid
            # DIRNAME=old_liboqs taskset -c $CUR_CPU make clean all
            # DIRNAME=old_liboqs taskset -c $CUR_CPU python3 run_all.py loqs_2018_11
            # touch finished_old
        ) &
        # (cd $p; echo $CUR_CPU)
        CUR_CPU=$((CUR_CPU+1))
    done

    for p in $PATHS_FOR_TWO; do
        (
            cd $p
            DIRNAME=liboqs taskset -c $CUR_CPU make clean all
            DIRNAME=liboqs taskset -c $CUR_CPU python3 run_all.py current
            touch finished_current
            DIRNAME=mid_liboqs taskset -c $CUR_CPU make clean all
            DIRNAME=mid_liboqs taskset -c $CUR_CPU python3 run_all.py loqs_0.4.0
            touch finished_mid
        ) &
        # (cd $p; echo $CUR_CPU)
        CUR_CPU=$((CUR_CPU+1))
    done
)
