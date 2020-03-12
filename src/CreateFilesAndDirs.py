import os
import time

os.chdir("Mountingpoint")
#
for i in range(10):
    with open('F' + str(i) + '.txt', 'w') as FW:
        time.sleep(1)
        FW.write( "This is File:" + str(i))
        time.sleep(1)

for i in range(5):
    newDirectory = 'D' + str(i)
    os.mkdir(newDirectory)
    time.sleep(1)
    os.chdir(newDirectory)
    time.sleep(1)
    with open('FD' + str(i) + '.txt', 'w') as FW:
        FW.write( "This is File in D:" + str(i))
        time.sleep(1)