import os, shutil, argparse, sys, subprocess, time
from pathlib import Path

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description = 'Duplicate Build Folder and SD Card')
  parser.add_argument('--sdCardFile', help = 'Path to SD Card file')
  parser.add_argument('--binaryOutputPath', help = 'Path to Binary Output')
  args = parser.parse_args(sys.argv[1:])

  binaryLocation = os.path.abspath(args.binaryOutputPath)

  if os.path.exists(binaryLocation + '/../x64 (2)'):
    shutil.rmtree(binaryLocation + '/../x64 (2)')

  shutil.copytree(binaryLocation, binaryLocation + '/../x64 (2)')

  if len(args.sdCardFile.strip()) != 0:
    sdCardPath = os.path.abspath(args.sdCardFile)
    sdCardFileName = Path(sdCardPath).stem
    secondSDCardPath = os.path.dirname(sdCardPath) + "/" + sdCardFileName + "2.raw"
    if os.path.exists(secondSDCardPath):
      os.remove(secondSDCardPath)
    shutil.copyfile(sdCardPath, secondSDCardPath)

    data = None
    dolphinConfigLocation = os.path.abspath(binaryLocation + '/../x64 (2)/User/Config/Dolphin.ini')
    if not os.path.exists(dolphinConfigLocation):
      subprocess.Popen([binaryLocation + '/../x64 (2)/dolphin.exe'])
      time.sleep(3)
      subprocess.call("TASKKILL /F /IM dolphin.exe", shell=True)
    with open(dolphinConfigLocation, 'r') as file:
      data = file.readlines()

    sdCardFilePathChanged = False
    for i in range(0, len(data)):
      if 'WiiSDCardPath' in data[i]:
        sdCardFilePathChanged = True
        data[i] = 'WiiSDCardPath = ' + secondSDCardPath + '\n'

    if sdCardFilePathChanged:
      with open(dolphinConfigLocation, 'w') as file:
        file.writelines(data)
