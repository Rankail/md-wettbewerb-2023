import sys
import os
import subprocess
import re
from multiprocessing import Pool, Value
import numpy as np
import matplotlib.pyplot as plt
import time

PROCESS_COUNT = 6

def computePoints(id: int, file: str, start: float, end: float, count: int):
    results = []

    totalSize = end-start
    part = totalSize/PROCESS_COUNT
    step = totalSize/count

    weight = start + part * id
    partEnd = start + part * (id + 1)
    while weight < partEnd:
        p = subprocess.Popen(["./../inputs/Solver_Release.exe", f"./../inputs/{file}.txt", str(weight)], stdout=subprocess.PIPE)
        output = str(p.communicate()[0]).split("\\r\\n")[-3]
        if output == None: break
        m = re.match("Max: (\d*\.\d*) = (\d*\.\d*) \* (\d*\.\d*) .*", output)
    
        results.append([weight, *m.groups()])
        results = list(list(float(n) for n in t) for t in results)

        with cnt.get_lock():
            cnt.value += 1
            print(cnt.value, weight, m.group(1))

        weight += step

    return results

def init_globals(counter):
    global cnt
    cnt = counter

if __name__ == "__main__":
    if len(sys.argv) != 5:
        name = input("File: ")
        count = int(input("Count: "))
        start = float(input("Start: "))
        end = float(input("End: "))
    else:
        name = sys.argv[1]
        count = int(sys.argv[2])
        start = float(sys.argv[3])
        end = float(sys.argv[4])

    inputPath = f"../inputs/{name}.txt"
    if not os.path.isfile(inputPath):
        print("Failed to find input-file")
        exit(1)
    if count < 1:
        print("Count must at least be 1")
        exit(1)

    if start > end:
        print("Start must be less than end")
        exit(1)
    elif not (0 <= start <= 2 and 0 <= end <= 2):
        print("Start and End cant be less than zero or greater than 2")
        exit(1)

    startTime = time.perf_counter_ns()
    
    cnt = Value("i", 0)

    with Pool(initializer=init_globals, initargs=(cnt,)) as p:
        r = p.starmap(computePoints, [(i, name, start, end, count) for i in range(PROCESS_COUNT)])

    endTime = time.perf_counter_ns()

    print(f"Finished after {(endTime - startTime)/1E9}s")

    r = [i for s in r for i in s]
    x = [o[0] for o in r]
    b = [o[1] for o in r]
    a = [o[2] for o in r]
    d = [o[3] for o in r]

    maxB = max(b)
    maxX = x[b.index(maxB)]

    print(maxX, maxB)

    plt.plot(x, b, ".", color="orange", label="B", linewidth=1.)
    # plt.plot(x, d, "-g", label="D", linewidth=1.)
    # plt.plot(x, a, "-b", label="A", linewidth=1.)
    plt.plot(maxX, maxB, ".r", label="Max")
    plt.show()

