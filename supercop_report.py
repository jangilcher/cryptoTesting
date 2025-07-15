import os
import subprocess
import collections
from report import XLSXReport, SQLiteReport


CRASHFILE="/fuzzing/tech/paper_fuzzing/supercop/crypto_hash/supercop.fuzzcrash.log"


def main(report_fn):
    xlsreport = XLSXReport()
    sqlreport = SQLiteReport()
    sqlreport.startsection("hash collision")

    visited = []
    failed_to_visit = []

    with open(CRASHFILE) as crashfile_fd:
        # for each implementation that crashed
        for line in crashfile_fd.readlines():
            implementation, _ = tuple(line.split(" "))
            if implementation in visited:
                continue
            visited.append(implementation)
            first_crash = True

            # enter the directory
            print(f"looking at {implementation}")
            crash_dir = os.path.join(implementation, 'crashes')

            # for each crash file
            try:
                listdir = os.listdir(crash_dir)
            except Exception as e:
                failed_to_visit.append(implementation)
                print(f"Could not read directory {implementation}")
                print(e)
                continue
            for crash_file in os.listdir(crash_dir):
                crash_fn = os.fsdecode(crash_file)
                if crash_fn == "README.txt":
                    continue
                print(f"\t{crash_fn}")

                # run ParseInput on the crash file
                proc = subprocess.run([f"{implementation}/ParseInput.out", os.path.join(crash_dir, crash_fn)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                out = proc.stdout.decode('ascii')
                err = proc.stderr.decode('ascii')

                # 3. profit
                obj = collections.OrderedDict()
                obj["name"] = implementation.strip()
                if "==ERROR" in err:
                    for line in err.split('\n'):
                        if "SUMMARY" in line:
                            obj["error"] = line
                            break
                else:
                    for line in out.split('\n'):
                        if line == "" or line == "Hello from GenInput":
                            continue
                        key, val = tuple(line.split(":"))
                        obj[key] = val.strip()

                if first_crash:
                    xlsreport.startsection(list(obj.keys()), bold=True)
                    first_crash = False

                xlsreport.addentry(list(obj.values()))
                sqlreport.addentry(obj)

    xlsreport.savetodisk(report_fn)
    sqlreport.savetodisk(report_fn)

    if len(failed_to_visit) > 0:
        print()
        print("Failed to visit the following directories")
        for dir in failed_to_visit:
            print(dir)

if __name__ == "__main__":
    main(f"reports/crash_report_supercop_hash")
