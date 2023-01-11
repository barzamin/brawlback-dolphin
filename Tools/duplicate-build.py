import os, shutil, argparse, sys, subprocess, time
from pathlib import Path

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description = 'Duplicate Build Folder and SD Card')
  parser.add_argument('--binaryOutputPath', help = 'Path to Binary Output')
  args = parser.parse_args(sys.argv[1:])

  binaryLocation = os.path.abspath(args.binaryOutputPath)

  if os.path.exists(binaryLocation + '/../x64 (2)'):
    shutil.rmtree(binaryLocation + '/../x64 (2)')

  shutil.copytree(binaryLocation, binaryLocation + '/../x64 (2)')