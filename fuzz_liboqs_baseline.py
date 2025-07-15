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
    "tech/paper_fuzzing/vanilla/liboqs/KEM/Decaps/c-sk-eq",
    "tech/paper_fuzzing/vanilla/liboqs/KEM/Encaps/pk-eq",
    "tech/paper_fuzzing/vanilla/liboqs/SIGN/Sign",
    "tech/paper_fuzzing/vanilla/liboqs/SIGN/Verify/m-pk-sig",
)


BLACKLIST=(
    # the three entries below slow significantly the process
    # full results do however include these experiments
    # ("McEliece", "Encaps/pk-eq"),      # huge pk, makes the test take really long
    # ("BIKE", "Encaps/pk-eq"),          # ditto
    # ("Frodo", "Encaps/pk-eq"),         # ditto
)

WHITELIST = (
    # ("SIDH", "Encaps/pk-eq"),
)

def whitelisted(alg, testpath):
    for n, t in WHITELIST:
        if n.lower() in alg.lower() and t.lower() in testpath.lower():
            if LOGFILE:
                print(f"Not Skipping {alg.lower()}, {testpath.lower()}", file=LOGFILE)
                LOGFILE.flush()
            return True
    else:
        print(f"Skipping {alg.lower()}, {testpath.lower()}", file=LOGFILE)
        LOGFILE.flush()
    return False

def blacklisted(alg, testpath):
    for n, t in BLACKLIST:
        if      n.lower() in alg.lower() \
            and t.lower() in testpath.lower():
            if LOGFILE:
                print(f"Skipping {n}, {t}", file=LOGFILE)
                LOGFILE.flush()
            return True
    else:
        print(f"Not Skipping {alg.lower()}, {testpath.lower()}", file=LOGFILE)
        LOGFILE.flush()
    return False

FILTER_KEM = [1]
FILTER_SIG = [1]

def get_algs(testpath, liboqs):
    shellcmd = f'bash -c "cd {testpath}; DIRNAME={liboqs} make clean all > /dev/null 2>&1; python3 run_all.py --n_algs_only; exit $?"'
    
    proc = subprocess.run(shellcmd, shell=True, stdout=subprocess.PIPE, stderr= subprocess.PIPE) #subprocess.DEVNULL)
    # print(proc.stderr.decode())
    algs_d = collections.OrderedDict(json.loads(proc.stdout.decode('ascii').strip()))
    return algs_d


def experiment(ctr_testpath_alg_mutator_liboqs_algsd):
    ctr, testpath, alg, mutator, liboqs, algs_d = ctr_testpath_alg_mutator_liboqs_algsd

    if blacklisted(algs_d[alg], testpath):
        return { 'ctr': ctr, 'testpath': testpath, 'alg': alg }

    # if not whitelisted(algs_d[alg], testpath):
    #     return { 'ctr': ctr, 'testpath': testpath, 'alg': alg }
    # if "KEM" in testpath and alg not in FILTER_KEM:
    #     return { 'ctr': ctr, 'testpath': testpath, 'alg': alg }
    # if "SIGN" in testpath and alg not in FILTER_SIG:
    #     return { 'ctr': ctr, 'testpath': testpath, 'alg': alg }
    
    shellcmd = f'bash -c "cd {testpath}; make clone; bash clone.sh {alg}; cd {alg}; DIRNAME={liboqs} DESIRED_ALG_TO_FUZZ={alg} make clean all > /dev/null 2>&1; python3 run_all.py --base_path aggr_{liboqs}_{mutator} --run_specific_alg_only {alg} --run_inside_clone"'
    try:
        subprocess.run(shellcmd,
                        shell=True,
                        stdout = subprocess.PIPE,
                        stderr = subprocess.STDOUT,
                        check=True,
                        universal_newlines=True)
        return { 'ctr': ctr, 'testpath': testpath, 'alg': alg }
    except Exception as e:
        if LOGFILE:
            print(shellcmd, file=LOGFILE)
            print(e, file=LOGFILE)
            LOGFILE.flush()
        else:
            print(shellcmd)
            print(e)
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
