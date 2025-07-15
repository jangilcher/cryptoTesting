import os
import re
import subprocess
from fuzz_liboqs_baseline import TESTPATHS
import collections
from report import XLSXReport, SQLiteReport
import signal


def main(mutator, liboqs, report_fn="crash_report"):
    xlsreport = XLSXReport()
    sqlreport = SQLiteReport()
    # texreport = TexReport()

    for testpath in TESTPATHS:
        no_entry_in_path = True
        aggr_dir = os.path.join(testpath, f'aggr_{liboqs}_{mutator}')
        if not os.path.isdir(aggr_dir):
            print(f"{aggr_dir} doesn't exist, skipping")
            continue
        for alg_aggr in sorted(os.listdir(aggr_dir)):
            if alg_aggr == "problematic.txt":
                continue
            # print(alg_aggr)
            alg = int(re.findall(r'\d+', alg_aggr)[0])
            alg_dir = os.path.join(aggr_dir, alg_aggr)
            if not os.path.isdir(alg_dir):
                continue
            with open(os.path.join(alg_dir, 'alg.txt')) as f:
                alg_name = f.readline()
            if alg_name == "DEFAULT":
                continue
            print(f"Including {alg_dir} {alg_name} in report")
            hangs_dir = os.path.join(alg_dir, 'fuzzoutputs', 'default', 'hangs')
            for file in os.listdir(hangs_dir):
                filename = os.fsdecode(file)
                if filename == "README.txt":
                    continue
                # print(f"\t{filename}")

                if no_entry_in_path:
                    # xlsreport.addentry(["path", os.path.join(*testpath.split("/")[-3:])], bold=True)
                    xlsreport.startsection(["path", os.path.join(*testpath.split("/")[-3:])], bold=True)
                    sqlreport.startsection(os.path.join(*testpath.split("/")[-3:]))
                    # texreport.startsection(os.path.join(*testpath.split("/")[-3:]))

                # xlsreport.addentry([alg_name], bold=True)
                # xlsreport.addentry(["hang"] * 8, italic=True)
                xlsreport.addentry([alg_name] + (["hang"] * 8))
                # sqlreport.addentry([alg_name] + (["hang"] * 8))
                sqlreport.addentry(collections.OrderedDict({"name": alg_name, "error": "hang"}))
                # texreport.addentry(alg_name, hang=True)
                no_entry_in_path = False

            crash_dir = os.path.join(alg_dir, 'fuzzoutputs', 'default', 'crashes')
            for file in os.listdir(crash_dir):
                filename = os.fsdecode(file)
                if filename == "README.txt":
                    continue
                # print(f"\t{filename}")
                # run parser on crash file:

                # 1. make sure parser executable exists (make dirs bin/ParseInput.out)
                parse_env = os.environ.copy()
                parse_env["DIRNAME"] = liboqs
                alg_id = os.path.basename(alg_dir)
                parse_env['DESIRED_ALG_TO_FUZZ'] = str(alg_id)
                make = subprocess.run(["make", "clean", "dirs", "bin/ParseInput.out", "bin/fuzz_harness_old.afl.out"], stdout=subprocess.PIPE, cwd=os.path.join(testpath, str(alg)), env=parse_env)
                # outs, errs = make.communicate(None)
                # print(make.stdout.decode('ascii'))
                # print("compilation complete")

                # 2. run parser bin and collect output
                parser_exe = os.path.join(testpath, str(alg), 'bin', 'ParseInput.out')
                proc = subprocess.run([parser_exe, os.path.join(crash_dir, filename)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                out = proc.stdout.decode('ascii')

                harness_exe = os.path.join(testpath, str(alg), 'bin', 'fuzz_harness_old.afl.out')
                proc = subprocess.run([harness_exe, os.path.join(crash_dir, filename)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                returncode = proc.returncode

                err = proc.stderr.decode('ascii')

                # 3. profit
                obj = collections.OrderedDict()
                obj["name"] = alg_name
                if returncode < 0:
                    obj["error"] = signal.Signals(-returncode).name
                else:
                    obj["error"] = f"returncode {returncode}"
                if "ERROR" in err:
                    for line in err.split('\n'):
                        if "ERROR:" in line:
                            obj["name"] = alg_name
                            obj["error"] = line
                            # don't break, "SUMMARY" in line is preferable
                        if "SUMMARY" in line:
                            obj["name"] = alg_name
                            obj["error"] = line
                            break
                else:
                    for line in out.split('\n'):
                        if line == "":
                            continue
                        key, val = tuple(line.split(":"))
                        obj[key] = val.strip()

                if no_entry_in_path:
                    # xlsreport.addentry(["path", os.path.join(*testpath.split("/")[-3:])], bold=True)
                    xlsreport.startsection(["path", os.path.join(*testpath.split("/")[-3:])], bold=True)
                    sqlreport.startsection(os.path.join(*testpath.split("/")[-3:]))
                    # texreport.startsection(os.path.join(*testpath.split("/")[-3:]))
                    xlsreport.addentry(list(obj.keys()), bold=True)

                xlsreport.addentry(list(obj.values()))
                sqlreport.addentry(obj)
                # texreport.addentry(alg_name, rowdata=obj)
                no_entry_in_path = False

    xlsreport.savetodisk(report_fn)
    sqlreport.savetodisk(report_fn)
    # texreport.savetodisk(report_fn)

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--liboqs', type=str, default="cur_liboqs")
    parser.add_argument('--mutator', type=str, default="python")

    args = parser.parse_args()
    mutator = args.mutator
    liboqs = args.liboqs

    # print (args)

    main(mutator, liboqs, report_fn=f"reports/crash_report_{liboqs}_vanilla")
