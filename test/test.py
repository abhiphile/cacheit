#!/usr/bin/python3

import asyncio
import sys
import time

async def run(repetitions):
  cmd = './client'
  to_wait = []
  # stress test, simulate multiple concurrent connections
  for i in range(repetitions):
    to_wait.append(await asyncio.create_subprocess_shell(
      cmd,
      stdout= asyncio.subprocess.DEVNULL,
      stderr= asyncio.subprocess.DEVNULL
    ))
    print(f"{i} executed")

  for i in range(repetitions):
    await to_wait[i].communicate()
    print(f'[{cmd!r} exited with {to_wait[i].returncode}]')

if __name__ == '__main__':
  if len(sys.argv) < 2:
    repetitions = 10
  else:
    repetitions = int(sys.argv[1])
  asyncio.run(run(repetitions))