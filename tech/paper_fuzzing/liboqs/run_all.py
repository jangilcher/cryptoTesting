import json
import subprocess
import os
import shutil
import tqdm
import sys
import pathlib


def cleanup_fuzzing_outputs(base_path):
    # clean pre-existing aggregated outputs if they exist
    try:
        shutil.rmtree(base_path)
    except FileNotFoundError:
        pass

    # create directory for aggregated outputs
    pathlib.Path(base_path).mkdir(parents=True)


def count_algs():
    """ Counts avilable algorithms in liboqs """
    kems, sigs = 0, 0
    kems_d = {}
    sigs_d = {}

    res = subprocess.run(['bin/list.out'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout = res.stdout
    for line in stdout.decode().split("\n"):
        if "OQS_KEM_algs_length" in line:
            kems = int(line.split(":")[1])
            continue
        if "OQS_SIG_algs_length" in line:
            sigs = int(line.split(":")[1])
            continue
        if "KEM " in line:
            _num, _name = tuple(line.split(":"))
            kems_d[int(_num.split(" ")[1])] = _name.strip()
            continue
        if "SIG " in line:
            _num, _name = tuple(line.split(":"))
            sigs_d[int(_num.split(" ")[1])] = _name.strip()
            continue

    return kems, sigs, kems_d, sigs_d


def run_algo(alg, alg_name, mutator_tool, base_path, run_inside_clone=False, verbose=False):
    """ runs fuzzer with provided mutator tool for the given algorithm """

    if mutator_tool not in ["afl", "python"]:
        raise ValueError("Only `python` and `afl` are the only supported mutating tools.")

    if verbose:
        print('Fuzzing algorithm', alg)

    if run_inside_clone:
        base_path = os.path.normpath(os.path.join('..', base_path))

    # build and fuzz
    my_env = os.environ.copy()
    my_env['DESIRED_ALG_TO_FUZZ'] = str(alg)
    res = subprocess.run(['make', f'run_{mutator_tool}'], env=my_env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout = res.stdout
    if verbose:
        print(stdout)

    pathlib.Path(base_path).mkdir(parents=True, exist_ok=True)
    problematic_file_path = os.path.join(base_path, 'problematic.txt')

    # Creates a new file to track algorithms where fuzzing was not possible or where it crashed
    if b'is too big' in stdout or b'PROGRAM ABORT' in stdout:
        with open(problematic_file_path, 'a') as f:
            f.write(f"{alg} {alg_name}\n")

    # prepare a directory to store crashes from _this_ algorithm
    dir_name = str(alg)
    full_path = os.path.join(base_path, dir_name)
    if run_inside_clone:
        cleanup_fuzzing_outputs(full_path)
    else:
        os.mkdir(full_path)

    # make name of algorithm available
    if alg_name:
        with open(os.path.join(full_path, "alg.txt"), "w") as f:
            f.write(alg_name)

    # backup full stdout from fuzzing
    with open(os.path.join(full_path, 'stdout.txt'), 'wb') as f:
        f.write(stdout)

    # count crashes nad if any, add this algorithm to the list of problematic ones
    crash_files = os.listdir("fuzzoutputs/default/crashes/")
    if len(crash_files) > 0:
        with open(problematic_file_path, 'a') as f:
            f.write(f"{alg} {alg_name}\n")
    hang_files = os.listdir("fuzzoutputs/default/hangs/")
    if len(hang_files) > 0:
        with open(problematic_file_path, 'a') as f:
            f.write(f"{alg} {alg_name}\n")

    # copy the outputs of fuzzing for later inspection
    try:
        shutil.move('fuzzinputs/', os.path.join(full_path, 'fuzzinputs'))
        shutil.move('fuzzoutputs/', os.path.join(full_path, 'fuzzoutputs'))
    except:
        with open(problematic_file_path, 'a') as f:
            f.write(f"{alg} {alg_name}\n")


def main():

    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--mutator", help="python or afl", default="python")
    parser.add_argument("--base_path", help="path to aggregated outputs", default="aggregatedfuzzingoutputs")
    parser.add_argument("--bar_pos", help="position for tqdm bar", type=int, default=0)
    parser.add_argument("--dbg_max_alg", help="for debug purpose, only test the first few algorithms", type= int, default=9999)
    parser.add_argument("--n_algs_only", action="store_true")
    parser.add_argument("--run_specific_alg_only", type=int, default=-1)
    parser.add_argument("--run_inside_clone", action="store_true")
    args = parser.parse_args()

    base_path = args.base_path
    mutator_tool = args.mutator
    bar_pos = args.bar_pos
    dbg_max_alg = args.dbg_max_alg
    n_algs_only = args.n_algs_only
    run_specific_alg_only = args.run_specific_alg_only
    run_inside_clone = args.run_inside_clone
    cwd = os.getcwd()

    # check if we are running from inside the generic source directory by mistake,
    # or properly from a specific test directory
    srcfile_path = os.path.dirname(os.path.realpath(__file__))
    if srcfile_path == cwd:
        print("You should not be running this script directly.")
        print("Run this from a specific test directory instead.")
        exit(1)

    # estimate how many algorithms can be fuzzed
    kems, sigs, kems_d, sigs_d = count_algs()
    if "KEM" in cwd: # UGLY
        algs = kems
        algs_d = kems_d
    else:
        algs = sigs
        algs_d = sigs_d

    if n_algs_only:
        # print(algs)
        print(json.dumps(algs_d))
        exit(algs)

    if run_specific_alg_only == -1:
        algs = min(algs, dbg_max_alg)

        # cleanup directory from previous builds
        cleanup_fuzzing_outputs(base_path)

        # for alg in tqdm.tqdm(range(2), desc=f"{mutator_tool} {cwd}: ", position=int(bar_pos)):
        for alg in tqdm.tqdm(range(algs), desc=f"{mutator_tool} {cwd}: ", position=int(bar_pos)):
            sys.stdout.flush()
            run_algo(alg, algs_d[alg], mutator_tool, base_path)
    else:
        alg = run_specific_alg_only
        assert(alg >= 0)
        assert(alg < algs)
        run_algo(alg, algs_d[alg], mutator_tool, base_path, run_inside_clone=run_inside_clone)

    subprocess.run(['make', 'clean'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    sys.stdout.flush()


if __name__ == "__main__":
    main()
