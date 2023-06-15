import re
import time
import sys
from matplotlib import pyplot as plt

def match_timer(r, input_str):
    regexp = re.compile("^" + r + "$")
    start_time = time.perf_counter()
    result = regexp.match(input_str)
    execute_time = time.perf_counter() - start_time
    return execute_time

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
    for string in strings:
        if string != '':
            xs.append(len(string))
            res_file.write(str(match_timer(r, string)) + '\n')

with open(f'../test/example_{example_number}/python_results.txt', 'r') as res_file1, \
        open(f'../test/example_{example_number}/diploma_results.txt', 'r') as res_file2, \
        open(f'../test/example_{example_number}/diploma_reverse_results.txt', 'r') as res_file3:
    ys1 = res_file1.read().split()
    ys2 = res_file2.read().split()
    ys3 = res_file3.read().split()

    ys3 = [float(y) for y in ys3]
    ys2 = [float(y) for y in ys2]
    ys1 = [float(y) for y in ys1]

    plt.figure(dpi=300)

    if len(ys3) > 0:
        fig, axs = plt.subplots(1, 3, figsize=(40,20))
    else:
        fig, axs = plt.subplots(1, 2, figsize=(40,20))
    fig.suptitle(mfa_r + ", " + r, fontsize=25)
    axs = axs.flatten()


    axs[0].plot(xs, ys2)
    axs[0].set(xlabel='Length, chars', ylabel='Time, seconds')
    axs[0].set_title("diploma")
    if len(ys3) > 0:
        axs[1].plot(xs, ys3)
        axs[1].set(xlabel='Length, chars', ylabel='Time, seconds')
        axs[1].set_title("diploma reverse")

        axs[2].plot(xs, ys1)
        axs[2].set(xlabel='Length, chars', ylabel='Time, seconds')
        axs[2].set_title("python")
    else:
        axs[1].plot(xs, ys1)
        axs[1].set(xlabel='Length, chars', ylabel='Time, seconds')
        axs[1].set_title("python")

    plt.show()