import os
import time

os.chdir("Mountingpoint")

for i in range(10):
    if os.path.exists('F' + str(i) + '.txt'):
        print('F' + str(i) + '.txt', 'PASSED')
    else:
        print('F' + str(i) + '.txt', 'FAILED')

for i in range(5):
    newDirectory = 'D' + str(i)

    if os.path.exists(newDirectory):
        print('D' + str(i), 'PASSED')
    else:
        print('D' + str(i), 'FAILED')

    os.chdir(newDirectory)
    if os.path.exists('FD' + str(i) + '.txt'):
        print('FD' + str(i) + '.txt', 'PASSED')
    else:
        print('FD' + str(i) + '.txt', 'FAILED')