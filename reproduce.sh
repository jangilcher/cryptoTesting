#!/bin/bash

LIBRARY=$1
BASELINEFLAG=${2:-""}

if [[ "$LIBRARY" == "" ]]
then
    echo "To reproduce the experiments, call \`bash reproduce.sh <library> <baseline>\`"
    echo "where <library> is set as per the README file,"
    echo "and <baseline> is either"
    echo "  baseline           for the baseline experiments from Section 5.1.1"
    echo "  anything else      for the functional testing introduced in this paper"
    exit 1
fi

if [[ "$LIBRARY" == "supercop" ]]
then
    make get_supercop # download supercop-20240107.tar.xz
    if [[ "$BASELINEFLAG" == "baseline" ]]
    then
        # supercop baseline tests
        make supercop_baseline # to install supercop and run experiments
        python3 supercop_report_baseline.py # to collect results
    else
        # supercop functional tests
        make supercop # to install supercop and run experiments
        python3 supercop_report.py # to collect results
    fi
else
    # not supercop => liboqs
    LIBOQS=$LIBRARY
    make ${LIBOQS} # to install dependences
    if [[ "$BASELINEFLAG" == "baseline" ]]
    then
        # liboqs baseline tests
        python3 fuzz_liboqs_baseline.py --liboqs ${LIBOQS} --logfile ${LIBOQS}.log # to run experiments
        python3 report_baseline.py --liboqs ${LIBOQS} # to collect results
    else
        # liboqs functional tests
        python3 fuzz_liboqs.py --liboqs ${LIBOQS} --logfile ${LIBOQS}.log # to run experiments
        python3 report.py --liboqs ${LIBOQS} # to collect results
    fi
fi
