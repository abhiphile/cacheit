import subprocess
import os
from datetime import datetime
from benchmark_utils import color_text

BENCHMARK_DIR = os.path.dirname(os.path.abspath(__file__))
REPORT_PATH = os.path.join(BENCHMARK_DIR, "benchmark_report.md")

BENCHMARK_SCRIPTS = [
    'string/benchmark_string.py',
    'linkedlist/benchmark_linkedlist.py',
    'set/benchmark_set.py',
    'hash/benchmark_hash.py',
    'zset/benchmark_zset.py',
    'bitmap/benchmark_bitmap.py',
]

def run_benchmarks():
    report_lines = [f"# Benchmark Report\n", f"_Generated: {datetime.now()}_\n\n"]
    for script in BENCHMARK_SCRIPTS:
        script_path = os.path.join(BENCHMARK_DIR, script)
        section = f"## {script.split('/')[0].capitalize()} Benchmark\n"
        if os.path.exists(script_path):
            print(color_text(f"\n------ Running {script} ------\n", "yellow"))
            result = subprocess.run(['python3', script_path], capture_output=True, text=True)
            print(result.stdout)
            if result.stderr:
                print(result.stderr)
            report_lines.append(section)
            report_lines.append('```text\n' + result.stdout.strip() + '\n```\n')
        else:
            print(f"Script not found: {script_path}")
            report_lines.append(section)
            report_lines.append("Script not found.\n")
    with open(REPORT_PATH, "w") as f:
        f.writelines(report_lines)
    print(color_text(f"\nBenchmark report written to {REPORT_PATH}", "green"))

if __name__ == "__main__":
    run_benchmarks()
