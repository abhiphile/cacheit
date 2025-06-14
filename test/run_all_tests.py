import subprocess
import sys
from testlib import color_text

test_files = [
    "test_string.py",
    "test_linkedlist.py",
    "test_set.py",
    "test_hash.py",
    "test_zset.py",
    "test_bitmap.py",
]

all_passed = True
for test in test_files:
    print(color_text(f"Running {test}...", "blue"))
    result = subprocess.run([sys.executable, test], cwd="./test")
    if result.returncode != 0:
        print(color_text(f"FAILED: {test}", "red"))
        all_passed = False
    else:
        print(color_text(f"PASSED: {test}", "green"))

if all_passed:
    print(color_text("\nAll tests passed!", "cyan"))
else:
    print(color_text("\nSome tests failed.", "red"))
