"""
Instructions to run this code.
==============================
"""
import json
import collections
import tqdm
import multiprocessing
import subprocess
import psutil
import time


LOGFILE=None


TESTPATHS=(
    "tech/paper_fuzzing/liboqs/KEM/Decaps/c",
    "tech/paper_fuzzing/liboqs/KEM/Decaps/sk",
    "tech/paper_fuzzing/liboqs/KEM/Encaps/badrng",
    "tech/paper_fuzzing/liboqs/KEM/Encaps/pk-0",
    "tech/paper_fuzzing/liboqs/KEM/Encaps/pk",
    "tech/paper_fuzzing/liboqs/KEM/Keygen/badrng",
    "tech/paper_fuzzing/liboqs/SIGN/Keygen/badrng",
    "tech/paper_fuzzing/liboqs/SIGN/Sign/badrng",
    "tech/paper_fuzzing/liboqs/SIGN/Sign/m",
    "tech/paper_fuzzing/liboqs/SIGN/Sign/sk",
    "tech/paper_fuzzing/liboqs/SIGN/Verify/m",
    "tech/paper_fuzzing/liboqs/SIGN/Verify/sig",
    "tech/paper_fuzzing/liboqs/SIGN/Verify/pk",
)


BLACKLIST=(
    # the three entries below slow significantly the process
    # full results do however include these experiments
    ("McEliece", "Encaps/pk"),      # huge pk, makes the test take really long
    ("BIKE", "Encaps/pk"),          # ditto
    ("Frodo", "Encaps/pk"),         # ditto
    ("sntrup761", "Keygen/badrng"),  # hangs
)


def blacklisted(alg, testpath):
    for n, t in BLACKLIST:
        if      n.lower() in alg.lower() \
            and t.lower() in testpath.lower():
            if LOGFILE:
                print(f"Skipping {n}, {t}", file=LOGFILE)
                LOGFILE.flush()
            return True
    return False


def get_algs(testpath, liboqs):
    shellcmd = f'bash -c "cd {testpath}; DIRNAME={liboqs} make clean all > /dev/null 2>&1; python3 run_all.py --n_algs_only; exit $?"'
    proc = subprocess.run(shellcmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    algs_d = collections.OrderedDict(json.loads(proc.stdout.decode('ascii').strip()))
    return algs_d


def experiment(ctr_testpath_alg_mutator_liboqs_algsd):
    ctr, testpath, alg, mutator, liboqs, algs_d = ctr_testpath_alg_mutator_liboqs_algsd

    if blacklisted(algs_d[alg], testpath):
        return { 'ctr': ctr, 'testpath': testpath, 'alg': alg }

    shellcmd = f'bash -c "cd {testpath}; make clone; bash clone.sh {alg}; cd {alg}; DIRNAME={liboqs} make clean all > /dev/null 2>&1; python3 run_all.py --mutator {mutator} --base_path aggr_{liboqs}_{mutator} --run_specific_alg_only {alg} --run_inside_clone"'
    try:
        subprocess.run(shellcmd,
                        shell=True,
                        stdout = subprocess.PIPE,
                        stderr = subprocess.STDOUT,
                        check=True,
                        universal_newlines=True)
        return { 'ctr': ctr, 'testpath': testpath, 'alg': alg }
    except:
        if LOGFILE:
            print(shellcmd, file=LOGFILE)
            LOGFILE.flush()
        else:
            print(shellcmd)
        exit(1)


nproc = int(0.8 * psutil.cpu_count(logical=False)) - 1
print(f"Using pool of size {nproc}")

def main(mutator, liboqs):

    bars = []
    algs = []
    for ctr in range(len(TESTPATHS)):
        testpath = TESTPATHS[ctr]
        algs_d = get_algs(testpath, liboqs)
        algs.append(algs_d)
        bars.append(tqdm.tqdm(total=len(list(algs_d.keys())), position=ctr, desc='/'.join(testpath.split('/')[-3:])))

    try:
        with multiprocessing.Pool(nproc) as pool:
            for rv in pool.imap_unordered(
                        experiment,
                        list(
                            (_ctr, TESTPATHS[_ctr], _alg, mutator, liboqs, algs[_ctr])
                            for _ctr in range(len(TESTPATHS))
                            for _alg in algs[_ctr].keys()
                        )
            ):
                # rv['ctr']
                # rv['testpath']
                # rv['alg']
                bars[rv['ctr']].update(1)
            pool.close()
            pool.join()
    except KeyboardInterrupt:
        subprocess.run(f"echo -ne '%s'" % ('\eD' * len(TESTPATHS)), shell=True)
        print("\n\nAborting.\n")
        pass

    for bar in bars:
        bar.close()
    subprocess.run(f"echo -ne '%s'" % ('\eD' * len(TESTPATHS)), shell=True)


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--liboqs', type=str, default="cur_liboqs")
    parser.add_argument('--mutator', type=str, default="python")
    parser.add_argument('--testpath', type=str, default=None)
    parser.add_argument('--logfile', type=str, default=None)

    args = parser.parse_args()
    mutator = args.mutator
    liboqs = args.liboqs
    testpath = args.testpath
    logfile = args.logfile
    if testpath:
        TESTPATHS = [ testpath ]
    if logfile:
        LOGFILE = open(logfile, 'w')

    # print (args)
    if "old" in liboqs:
        TESTPATHS = [_ for _ in TESTPATHS if "Verify" not in _]

    dt = time.time()
    main(mutator, liboqs)
    dt = time.time() - dt

    print("Wall time:", dt)
    print(f"Used pool of size {nproc}")

    if LOGFILE:
        LOGFILE.close()
