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
    # signal.setitimer(signal.SIGALRM, 0.5, 0.5)
    signal.alarm(1)

    try:
        start_time = time.perf_counter()
        regexp.match(input_str)
        return time.perf_counter() - start_time
    except TimeoutException:
        return 1
    finally:
        signal.alarm(0)

def pumpimg(n, pump):
    pump_count = len(pump) // 2 + 1
    del_count = len(pump) - pump_count

    pump_str = pump[0]
    tmp_len = len(pump[0])
    while len(pump_str) +  tmp_len < (n - del_count) // pump_count:
        pump_str += pump[0]

    if len(pump) == 1:
        return pump_str
    else:
        return (pump_str + pump[1]) * del_count + pump_str

example_number = sys.argv[1]
xs = []
mfa_r = ""
r = ""

with open(f'test/example_{example_number}/regexp.txt', 'r') as file, \
        open(f'test/example_{example_number}/pump.txt', 'r') as input_file, \
        open(f'test/example_{example_number}/python_results.txt', 'w') as res_file:
    text = file.read()
    (mfa_r, r) = text.split('\n')
    print(r)
    (pump, suffix, prefix) = input_file.read().split('\n')
    pump = pump.split(',')
    pump_step = 10
    pump_size = 5
    execute_time = 0
    count = 0
    len_limit = 10**8
    string = prefix + pumpimg(pump_size, pump) + suffix
    while execute_time < 0.5 and len(string) < len_limit:
        xs.append(len(string))
        execute_time = match_timer(r, string)
        res_file.write(str(execute_time) + '\n')
        count += 1

        if count % 10 == 0:
            pump_size *= 2

        pump_size += pump_step
        string = prefix + pumpimg(pump_size, pump) + suffix


with open(f'test/example_{example_number}/python_results.txt', 'r') as res_file1, \
        open(f'test/example_{example_number}/diploma_results.txt', 'r') as res_file2, \
        open(f'test/example_{example_number}/diploma_bnf_results.txt', 'r') as res_file3, \
        open(f'test/example_{example_number}/diploma_reverse_results.txt', 'r') as res_file4:
    ys1 = res_file1.read().split()
    lines2 = res_file2.read().split('\n')
    lines3 = res_file3.read().split('\n')
    lines4 = res_file4.read().split('\n')

    ys1 = [float(y) for y in ys1]

    points2 = list(zip(*([line.split(' ') for line in lines2][:-1])))
    points2[0] = [int(y) for y in points2[0]]
    points2[1] = [float(y) for y in points2[1]]
    points3 = list(zip(*([line.split(' ') for line in lines3][:-1])))
    points3[0] = [int(y) for y in points3[0]]
    points3[1] = [float(y) for y in points3[1]]
    points4 = list(zip(*([line.split(' ') for line in lines4][:-1])))
    points4[0] = [int(y) for y in points4[0]]
    points4[1] = [float(y) for y in points4[1]]

    fig, axs = plt.subplots(1, 4, figsize=(40,20))
    fig.suptitle(mfa_r + ", " + r, fontsize=20)
    # axs = axs.flatten()

    axs[0].plot(points2[0], points2[1])
    axs[0].set(xlabel='Length, chars', ylabel='Time, seconds')
    axs[0].set_title("diploma")

    axs[1].plot(points3[0], points3[1])
    axs[1].set(xlabel='Length, chars', ylabel='Time, seconds')
    axs[1].set_title("diploma bnf")

    axs[2].plot(points4[0], points4[1])
    axs[2].set(xlabel='Length, chars', ylabel='Time, seconds')
    axs[2].set_title("diploma reverse")

    axs[3].plot(xs[:len(ys1)], ys1)
    axs[3].set(xlabel='Length, chars', ylabel='Time, seconds')
    axs[3].set_title("python")

    plt.savefig(f'test/examples/image_{example_number}.png')
    plt.show()