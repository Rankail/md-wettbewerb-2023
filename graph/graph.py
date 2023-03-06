import sys
import os
import subprocess
import re
from multiprocessing import Pool, Value
import numpy as np
import matplotlib.pyplot as plt
import time

PROCESS_COUNT = 6

def computePoints(id: int, file: str, start: float, end: float, count: int, seed_start, seed_end):
    results = []

    totalSize = end-start
    part = totalSize/PROCESS_COUNT
    step = totalSize/count

    weight = start + part * id
    partEnd = start + part * (id + 1)
    while weight < partEnd:
        try:
            for seed in range(seed_start, seed_end+1):
                p = subprocess.Popen(["./../inputs/Solver_Release.exe", f"./../inputs/{file}.txt", f"--out=./out/{file}-{weight}.txt", str(weight), str(seed)], stdout=subprocess.PIPE)
                output = str(p.communicate()[0]).split("\\r\\n")
                if output == None: break
                m = re.match("Max: (\d*\.\d*) = (\d*\.\d*) \* (\d*\.\d*) .*", output[-5])
            
                results.append([weight, seed, *m.groups()])
                results = list(list(float(n) for n in t) for t in results)

                with cnt.get_lock():
                    cnt.value += 1
                    print(cnt.value, weight, seed, m.group(1))

        except AttributeError:
            print(f"Error:\n{file}-{seed}-{weight}.txt\n{output}")
        weight += step

    return results

def init_globals(counter):
    global cnt
    cnt = counter

if __name__ == "__main__":
    if len(sys.argv) != 7:
        name = input("File: ")
        count = int(input("Count: "))
        seed_start = int(input("Seed-start: "))
        seed_end = int(input("Seed-end: "))
        start = float(input("Start: "))
        end = float(input("End: "))
    else:
        name = sys.argv[1]
        count = int(sys.argv[2])
        seed_start = int(sys.argv[3])
        seed_end = int(sys.argv[4])
        start = float(sys.argv[5])
        end = float(sys.argv[6])

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
    elif seed_start > seed_end or seed_start < 0 or seed_end > 4294967295:
        print("Seed-start must not be greater than seed-end and both must be between 0 and 4294967295")
        exit(1)

    startTime = time.perf_counter_ns()
    
    cnt = Value("i", 0)

    with Pool(initializer=init_globals, initargs=(cnt,)) as p:
        r = p.starmap(computePoints, [(i, name, start, end, count, seed_start, seed_end) for i in range(PROCESS_COUNT)])

    endTime = time.perf_counter_ns()

    print(f"Finished after {(endTime - startTime)/1E9}s")

    r = [i for s in r for i in s]
    weights = [o[0] for o in r]
    seeds = [o[1] for o in r]
    scores = [o[2] for o in r]

    maxB = max(scores)
    maxW = weights[scores.index(maxB)]
    maxS = seeds[scores.index(maxB)]

    print(maxS, maxW, maxB)

    plt.plot(weights, scores, ".", color="orange", label="B", linewidth=1.)
    plt.plot(maxW, maxB, ".r", label="Max")
    plt.show()

