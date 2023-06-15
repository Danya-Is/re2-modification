import re
import time
import sys
import signal
from matplotlib import pyplot as plt

class TimeoutException(Exception):
    pass

def timeout_handler(signum, frame):
    raise TimeoutException("Function timed out")

def match_timer(r, input_str):
    regexp = re.compile("^" + r + "$")
    signal.signal(signal.SIGALRM, timeout_handler)
    signal.alarm(1)

    try:
        start_time = time.perf_counter()
        result = regexp.match(input_str)
        return time.perf_counter() - start_time
    except TimeoutException:
        return 1
    finally:
        signal.alarm(0)

example_number = sys.argv[1]
xs = []
mfa_r = ""
r = ""

with open(f'../test/example_{example_number}/regexp.txt', 'r') as file, \
        open(f'../test/example_{example_number}/input_strings.txt', 'r') as input_file, \
        open(f'../test/example_{example_number}/python_results.txt', 'w') as res_file:
    text = file.read()
    (mfa_r, r) = text.split('\n')
    print(r)
    strings = input_file.read().split('\n')
    execute_time = 0
    for string in strings:
        if string != '':
            xs.append(len(string))
            if execute_time < 1:
                execute_time = match_timer(r, string)
            if execute_time < 1:
                res_file.write(str(execute_time) + '\n')


with open(f'../test/example_{example_number}/python_results.txt', 'r') as res_file1, \
        open(f'../test/example_{example_number}/diploma_results.txt', 'r') as res_file2, \
        open(f'../test/example_{example_number}/diploma_reverse_results.txt', 'r') as res_file3:
    ys1 = res_file1.read().split()
    ys2 = res_file2.read().split()
    ys3 = res_file3.read().split()

    ys3 = [float(y) for y in ys3]
    ys2 = [float(y) for y in ys2]
    ys1 = [float(y) for y in ys1]

    plt.figure(dpi=400)

    if len(ys3) > 0:
        fig, axs = plt.subplots(1, 3, figsize=(30,10))
    else:
        fig, axs = plt.subplots(1, 2, figsize=(30,10))
    fig.suptitle(mfa_r + ", " + r, fontsize=20)
    axs = axs.flatten()


    axs[0].plot(xs[:len(ys2)], ys2)
    axs[0].set(xlabel='Length, chars', ylabel='Time, seconds')
    axs[0].set_title("diploma")
    if len(ys3) > 0:
        axs[1].plot(xs[:len(ys3)], ys3)
        axs[1].set(xlabel='Length, chars', ylabel='Time, seconds')
        axs[1].set_title("diploma reverse")

        axs[2].plot(xs[:len(ys1)], ys1)
        axs[2].set(xlabel='Length, chars', ylabel='Time, seconds')
        axs[2].set_title("python")
    else:
        axs[1].plot(xs[:len(ys1)], ys1)
        axs[1].set(xlabel='Length, chars', ylabel='Time, seconds')
        axs[1].set_title("python")

    plt.savefig(f'../test/example_{example_number}/result.png')