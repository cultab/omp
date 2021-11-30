"""Benchmark."""

import subprocess

from contextlib import redirect_stdout
from tqdm import tqdm


def main():
    """."""
    # benchmark with different sizes
    runs = [
        {"t": 4, "N": 4},
        {"t": 4, "N": 8},
        {"t": 4, "N": 16},
        {"t": 4, "N": 32},
        {"t": 4, "N": 64},
    ]
    repeat = 500
    with open('sizes.csv', 'w') as f:
        with redirect_stdout(f):
            bench(runs, repeat)

    # benchmark with different ammount of threads
    runs = [
        {"t": 1, "N": 1024},
        {"t": 2, "N": 1024},
        {"t": 4, "N": 1024},
        {"t": 8, "N": 1024},
        {"t": 16, "N": 1024},
        {"t": 32, "N": 1024},
    ]
    repeat = 10
    with open('threads.csv', 'w') as f:
        with redirect_stdout(f):
            bench(runs, repeat)


def bench(runs, repeat):
    """Benchmark @runs, repeating each run @repeat times."""
    print("threads,size,time")
    bar = tqdm(total=len(runs) * repeat)
    for run in runs:
        cmd = f'./create --sddm {run["N"]} | ./diag {run["t"]}'
        # print(cmd)
        sddm_time = 0.0
        diag_max_time = 0.0
        diag_max_value = 0.0
        create_time = 0.0
        b_min_reduce_time = 0.0
        b_min_critical_time = 0.0
        b_min_tree_time = 0.0

        for _ in range(repeat):
            res = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE)
            # result = subprocess.Popen(cmd, shell=True, stdin=None, stdout=subprocess.PIPE).stdout.decode('utf-8')
            result = res.stdout.decode('utf-8')
            lines = result.split("\n")

            sddm_time += float(lines[2].split()[0])

            diag_max_time += float(lines[3].split()[0])
            diag_max_value += float(lines[3].split()[6].split("=")[1])

            create_time += float(lines[4].split()[0])

            b_min_reduce_time += float(lines[run["N"] + 5].split()[0])
            b_min_critical_time += float(lines[run["N"] + 6].split()[0])
            b_min_tree_time += float(lines[run["N"] + 7].split()[0])
            bar.update()

        #     print(f'{sddm_time=:>19}')
        #     print(f'{diag_max_time=:>15}')
        #     print(f'{create_time=:>17}')
        #     print(f'{b_min_reduce_time=:>11}')
        #     print(f'{b_min_critical_time=:>9}')
        #     print(f'{b_min_tree_time=:>13}')

        sddm_time /= repeat
        diag_max_time /= repeat
        diag_max_value /= repeat
        create_time /= repeat
        b_min_reduce_time /= repeat
        b_min_critical_time /= repeat
        b_min_tree_time /= repeat
        total_time = sddm_time + diag_max_time + create_time + b_min_reduce_time + b_min_critical_time + b_min_tree_time
        total_time_b = sddm_time + diag_max_time + create_time + b_min_reduce_time
        print(f'{run["t"]},{run["N"]},{total_time_b}')
        # print(f'{sddm_time=}')


if __name__ == "__main__":
    main()
