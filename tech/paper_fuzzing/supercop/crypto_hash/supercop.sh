#!/bin/bash

ROOTDIR=../../../../
CC=clang
AFLFUZZ=$ROOTDIR/aflpp/afl-fuzz
AFLCC=$ROOTDIR/aflpp/afl-clang-fast
if [ ! -f $AFLFUZZ ]; then
    echo "ERROR: afl-fuzz missing"
    exit 1
fi
if [ ! -f $AFLCC ]; then
    echo "ERROR: afl-clang-fast missing"
    exit 1
fi

min=1;
max=64;
while getopts u:l: flag
do
    case "${flag}" in
        u) max=${OPTARG};;
        l) min=${OPTARG};;
    esac
done

re='^[0-9]+$'
if ! [[ $min =~ $re ]] ; then
    echo "'$min' is not a non-negative number setting min=1"
    min=1
fi

if ! [[ $max =~ $re ]] ; then
    echo "'$max' is not a non-negative number setting max=1"
    max=1
fi

#set SUPERDIR if not already set
if [ -z ${SUPERDIR+x} ];
then
    SUPERDIR=/fuzzing/supercop-20220213
fi
echo "$SUPERDIR"
PRIMITIVE=crypto_hash
INTTYPES="$SUPERDIR/supercop-data/inttypes"
INCLUDE="-I$INTTYPES -I$SUPERDIR/include -I./"

LIBS="-lkeccak -lcrypto -lssl -lgmp -lcryptopp"

CFLAGS="-march=native -O3 -DSUPERCOP -ggdb -lm" #-fsanitize=address -mxop (AMD only)
AFLCFLAGS="-march=native -O3 -DSUPERCOP -lm"

BASICINPUTCRASH=""
FUZZCRASH=""
GENINPUTCRASH=""
NOCRASH=""
COMPILEERROR=""
APPROX_MEMCMP=""

rm -f supercop.log supercop.approx_memcmp.log supercop.basicinputcrash.log supercop.geninputcrash.log supercop.fuzzcrash.log supercop.compileerror.log supercop.nocrash.log;

fuzz_implementation() {
        impl=$1
        test_impl=0
        if [ -d $impl ]; then
            # we have an implementation
            test_impl=1
            
            rm -f $impl/aflclang.error.log
            rm -f $impl/clang.error.log
            rm -rf $impl/crashes
            rm -rf $impl/fuzzinputs
            rm -rf $impl/fuzzoutputs

            rm -rf $impl/BASICINPUTCRASH.log;
            rm -rf $impl/FUZZCRASH.log;
            rm -rf $impl/GENINPUTCRASH.log;
            rm -rf $impl/NOCRASH.log;
            rm -rf $impl/COMPILEERROR.log;
            rm -rf $impl/APPROX_MEMCMP.log;

            touch $impl/BASICINPUTCRASH.log
            touch $impl/FUZZCRASH.log
            touch $impl/GENINPUTCRASH.log
            touch $impl/NOCRASH.log
            touch $impl/COMPILEERROR.log
            touch $impl/APPROX_MEMCMP.log

            if [ -f $impl/architectures ]; then
            # if not then these are not complete, e.g. aarch64 is arm, too
                # there is an architectures file, better make sure it's x86
                test_impl=0
                if grep -q -E 'x86|amd64' "$impl/architectures"; then
                    # implementation is x86
                    test_impl=1
                fi
                # if grep -q arm "$impl/architectures"; then
                #     # implementation is arm specific
                #     test_impl=0
                # fi
                # if grep -q ppc "$impl/architectures"; then
                #     # implementation is powerpc specific
                #     test_impl=0
                # fi
            fi
        fi
        if [ $test_impl -eq 0 ]; then
            # either $impl is not a directory, or is known to be an arm implementation
            return
        fi

        # try compiling, creating inputs and running fuzzing
        c_files=$(find $impl -maxdepth 1 -iname "*.c" -print0 | tr -d -c "\000" | wc -c)
        if [ $c_files -eq 0 ]; then
            # nothing to compile?
            return
        fi
        err_in_impl=0
        retry_with_avx=0
        cp crypto_hash.h $impl/crypto_hash.h
        for c_file in $impl/*.c; do
            # need to try compiling with different increasing flags, eg to catch avx etc
            $CC $CFLAGS -c $c_file -o $c_file.o $INCLUDE $LIBS &> $impl/clang.error.log
            ret=$?
            # echo "$ret"
            if [ $ret -ne 0 ]; then
                # cleanup
                retry_with_avx=1
                err_in_impl=1
                break;
            fi
            $AFLCC $AFLCFLAGS -c $c_file -o $c_file.afl.o $INCLUDE $LIBS &> $impl/aflclang.error.log
            ret=$?
            # echo "$ret"
            if [ $ret -ne 0 ]; then
                # cleanup
                err_in_impl=1
                break;
            fi
        done;
        err_in_impl=0
        if [ $retry_with_avx -ne 0 ]; then
            echo "retrying $impl with avx2 support" | tee -a $impl/clang.error.log $impl/aflclang.error.log
            for c_file in $impl/*.c; do
                # need to try compiling with different increasing flags, eg to catch avx etc
                $CC $CFLAGS -mavx2 -c $c_file -o $c_file.o $INCLUDE $LIBS &>> $impl/clang.error.log
                ret=$?
                # echo "$ret"
                if [ $ret -ne 0 ]; then
                    # cleanup
                    err_in_impl=1
                    break;
                fi
                $AFLCC $AFLCFLAGS -mavx2 -c $c_file -o $c_file.afl.o $INCLUDE $LIBS &>> $impl/aflclang.error.log
                ret=$?
                # echo "$ret"
                if [ $ret -ne 0 ]; then
                    # cleanup
                    err_in_impl=1
                    break;
                fi
            done;
        fi
        if [ $err_in_impl -ne 0 ]; then
            # an error during compilation happened
            # TODO: "fatal error: 'crypto_hash.h' file not found" should be fixable somehow
            rm -rf "$impl/*.o" # for some reason this does not always remove all
            echo "$impl" >> $impl/COMPILEERROR.log
            return
        fi
        # making sure there is no main function in any of the objects
        for o_file in $impl/*.o; do
            objcopy --remove-section .llvm_addrsig $o_file # https://github.com/travitch/whole-program-llvm/issues/75#issuecomment-430433423
            objcopy --redefine-sym main=oldmain $o_file
        done;

        # here we should be able to link our test code with the implemetation in $impl
        # should already have compiled GenInput etc as shared objects to save time and space
        $CC $CFLAGS -o $impl/GenInput.out bin/GenInput.a $(ls $impl/*.S $impl/*.s 2> /dev/null) $(ls $impl/*.o | sed -e 's/[^ ]*.afl.o//g') $INCLUDE $LIBS &>> $impl/clang.error.log
        ret=$?
        # echo "$ret"
        if [ $ret -ne 0 ]; then
            # cleanup
            err_in_impl=1
        fi
        $CC $CFLAGS -o $impl/ParseInput.out bin/ParseInput.a $(ls $impl/*.S $impl/*.s 2> /dev/null) $(ls $impl/*.o | sed -e 's/[^ ]*.afl.o//g') $INCLUDE $LIBS &>> $impl/clang.error.log
        ret=$?
        # echo "$ret"
        if [ $ret -ne 0 ]; then
            # cleanup
            err_in_impl=1
        fi
        # TODO: parametrize
        # echo "$AFLCC $AFLCFLAGS -o Match.afl.out $(ls $impl/*.afl.o) bin/Match.afl.a"
        $AFLCC $AFLCFLAGS -o $impl/Match.afl.out $(ls $impl/*.afl.o) $(ls $impl/*.S $impl/*.s 2> /dev/null) bin/Match.afl.a $INCLUDE $LIBS &>> $impl/aflclang.error.log
        ret=$?
        # echo "$ret"
        if [ $ret -ne 0 ]; then
            # cleanup
            err_in_impl=1
        fi
        if [ $err_in_impl -ne 0 ]; then
            # an error during compilation happened
            # TODO: "fatal error: 'crypto_hash.h' file not found" should be fixable somehow
            rm -rf "$impl/*.o" # for some reason this does not always remove all
            echo "$impl" >> $impl/COMPILEERROR.log
            return
        fi
        echo "trying $impl"
        mkdir -p $impl/fuzzinputs $impl/fuzzoutputs $impl/crashes
        rm -f "$impl/supercop.geninput.log"
        export AFL_CUSTOM_MUTATOR_LIBRARY="$(pwd)/bin/Maul.so"
        export AFL_CUSTOM_MUTATOR_ONLY=1
        export AFL_SKIP_CPUFREQ=1
        export AFL_DISABLE_TRIM=1
        export AFL_EXIT_WHEN_DONE=1
        export AFL_EXIT_ON_TIME=1
        export AFL_DEBUG_CHILD=0
        export AFL_NO_UI=0
        outbytes=
        if [ -f $impl/api.h ]
        then
            outbytes=$(grep CRYPTO_BYTES $impl/api.h | sed 's/#define CRYPTO_BYTES //');
        fi
        for bytes in `seq $min $max`;
        do
            if [ ! -f $impl/GenInput.out ]
            then
                echo "$impl/GenInput.out doesn't exist"
            fi
            # $impl/GenInput.out $impl/fuzzinputs/test.bin $bytes $outbytes &>> $impl/supercop.geninput.log
            if ! ( false || $impl/GenInput.out $impl/fuzzinputs/test.bin $bytes $outbytes &>> $impl/supercop.geninput.log ) >/dev/null 2>&1; then
                # something went wrong, including possibly SIGABRT from GenInput.out
                echo "BAD $impl"
                (
                flock -x 200
                echo "$impl crashed on GenInput with $bytes byte" >> supercop.log;
                ) 200>/var/lock/supercop.log.lock
                echo "$impl $bytes" >> $impl/GENINPUTCRASH.log
                continue
            fi
            errorline=$($AFLFUZZ -i $impl/fuzzinputs/ -o $impl/fuzzoutputs/ -D -V 60 -- $impl/Match.afl.out @@);
            # echo "$errorline";
            if [ $(echo $errorline | grep 'We need at least one valid input seed that does not crash' | wc -w) -gt 0 ]
            then
                (
                flock -x 200
                echo "$impl crashed on basic test case with $bytes byte" >> supercop.log;
                ) 200>/var/lock/supercop.log.lock
                echo "$impl $bytes" >> $impl/BASICINPUTCRASH.log
                echo "$errorline" >> $impl/supercop.basicinputcrash.log
                continue
            fi

            if [ $(ls $impl/fuzzoutputs/default/crashes/ | wc -w) -eq 0 ]
            then
                (
                flock -x 200
                echo "$impl did not crash with $bytes byte" >> supercop.log;
                ) 200>/var/lock/supercop.log.lock
                echo "$impl $bytes" >> $impl/NOCRASH.log
                continue
            else
                if [ $(echo $errorline | grep 'not enough randomness in output' | wc -w) -gt 0 ]
                then
                    (
                    flock -x 200
                    echo "$impl results are too close $bytes byte" >> supercop.log;
                    ) 200>/var/lock/supercop.log.lock
                    echo "$impl $bytes" >> $impl/APPROX_MEMCMP.log
                    # echo "$errorline" >> $impl/supercop.approx_memcmp.log
                else
                    (
                    flock -x 200
                    echo "$impl crashed with $bytes byte" >> supercop.log;
                    ) 200>/var/lock/supercop.log.lock
                    echo "$impl $bytes" >> $impl/FUZZCRASH.log
                fi
            fi
            cp $impl/fuzzoutputs/default/crashes/* $impl/crashes;
        done;
        exit 0 # DEBUG: to stop the loop
}

# Run fuzzing in parallel using subprocesses (use all but one cores so the system does not hang)
N=$(grep -c ^processor /proc/cpuinfo)
let N-=1
# N=2

(
for hash in $SUPERDIR/$PRIMITIVE/* ; do
    # if [[ "$hash" != *"acehash"* ]]; then
    #     continue
    # fi
    for impl in $hash/*; do
        if [ -d $impl ]; then
            ((i=i%N)); ((i++==0)) && wait
            fuzz_implementation $impl &
        fi
    done;
done;

# wait for all proccesses to return
for job in `jobs -p`
do
    # echo $job
    wait $job || let "FAIL+=1"
done
)

sleep 2 # helps with some race condition-y thing happening at the end

# collect info from each subprocess that was run
for hash in $SUPERDIR/$PRIMITIVE/* ; do
    # if [[ "$hash" != *"acehash"* ]]; then
    #     continue
    # fi
    for impl in $hash/*; do
        if [ -d $impl ]; then
            touch $impl/BASICINPUTCRASH.log;
            BASICINPUTCRASH="$BASICINPUTCRASH$(cat $impl/BASICINPUTCRASH.log; echo x)"; BASICINPUTCRASH=${BASICINPUTCRASH%x}  # stupid workaround for $() force-trimming trailing whitespaces
            rm -rf $impl/BASICINPUTCRASH.log
            touch $impl/FUZZCRASH.log
            FUZZCRASH="$FUZZCRASH$(cat $impl/FUZZCRASH.log; echo x)"; FUZZCRASH=${FUZZCRASH%x}
            rm -rf $impl/FUZZCRASH.log
            touch $impl/GENINPUTCRASH.log;
            GENINPUTCRASH="$GENINPUTCRASH$(cat $impl/GENINPUTCRASH.log; echo x)"; GENINPUTCRASH=${GENINPUTCRASH%x}
            rm -rf $impl/GENINPUTCRASH.log
            touch $impl/NOCRASH.log
            NOCRASH=$NOCRASH"$(cat $impl/NOCRASH.log; echo x)"; NOCRASH=${NOCRASH%x}
            rm -rf $impl/NOCRASH.log
            touch $impl/COMPILEERROR.log
            COMPILEERROR="$COMPILEERROR$(cat $impl/COMPILEERROR.log; echo x)"; COMPILEERROR=${COMPILEERROR%x}
            rm -rf $impl/COMPILEERROR.log
            touch $impl/APPROX_MEMCMP.log
            APPROX_MEMCMP="$APPROX_MEMCMP$(cat $impl/APPROX_MEMCMP.log; echo x)"; APPROX_MEMCMP=${APPROX_MEMCMP%x}
            rm -rf $impl/APPROX_MEMCMP.log
        fi
    done;
done;

echo -n "$FUZZCRASH" > supercop.fuzzcrash.log;
echo -n "$GENINPUTCRASH" > supercop.geninputcrash.log;
echo -n "$BASICINPUTCRASH" > supercop.basicinputcrash.log;
echo -n "$NOCRASH" > supercop.nocrash.log;
echo -n "$COMPILEERROR" > supercop.compileerror.log;
echo -n "$APPROX_MEMCMP" > supercop.approx_memcmp.log;

numfuzzcrashes=$(cat supercop.fuzzcrash.log | wc -l)
numgeninputcrashes=$(cat supercop.geninputcrash.log | wc -l)
numbasicinputcrashes=$(cat supercop.basicinputcrash.log | wc -l)
numnocrashes=$(cat supercop.nocrash.log | wc -l)
numcompileerror=$(cat supercop.compileerror.log | wc -l)
numapprox_memcmp=$(cat supercop.approx_memcmp.log | wc -l)

echo "Crashes during fuzzing: $numfuzzcrashes"
echo "Crashes during geninput: $numgeninputcrashes"
echo "Crashes on basic test input: $numbasicinputcrashes"
echo "Fuzzing runs without crashes: $numnocrashes"
echo "Compilation Errors: $numcompileerror"
echo "Not enough randomness in results: $numapprox_memcmp"
