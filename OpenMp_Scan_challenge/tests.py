import subprocess
import numpy

vec_powers = [4, 8, 12, 13, 14, 15]
numberOfThreads = [4, 8]
scheduling_types = ['static', 'dynamic', 'guided', 'auto']

tests_toRun = 15

subprocess.run('gcc -fopenmp scan.c -o parallel_scan -lm', shell=True)
subprocess.run('gcc scan.c -o serial_scan -lm', shell=True)

# 
results = {}

vecsize = 2**10
print('Let\'s begin finding the best scheduling type on a medium size (1024) vector with number of threads = 4')


for sched in scheduling_types:
    command = 'OMP_NUM_THREADS=4' + \
        ' OMP_SCHEDULE='+ sched + \
        ' ./parallel_scan ' + str(vecsize)
    avg = 0
    for step in range(tests_toRun):
        elapsed_time = subprocess.check_output(
            command, shell=True).decode('utf-8').split('\n')[1].split(' ')[4]
        avg += float(elapsed_time)
    avg = round((avg/tests_toRun), 5)

    results[sched] = avg

best_sched = min(results, key=results.get)
print('the best scheduling type on average is: ' +  best_sched)
print(results)


#Let's now compare the serial algo with the parallel one using the best scheduling type for the for loops
for p in vec_powers:
    vecsize = 2**p
    print('\n-------VECSIZE = ' + str(vecsize)+' ----------')

    command = './serial_scan ' + str(vecsize)
    elapsed_time = subprocess.check_output(
        command, shell=True).decode('utf-8').split('\n')[1].split(' ')[4]

    print('Serial algo -> ' + elapsed_time +' millisecs;')

    for nt in numberOfThreads:
        command = 'OMP_NUM_THREADS=' + \
            str(nt) + ' OMP_SCHEDULE='+best_sched + \
            ' ./parallel_scan ' + str(vecsize)
        avg = 0
        for step in range(tests_toRun):
            elapsed_time = subprocess.check_output(
                command, shell=True).decode('utf-8').split('\n')[1].split(' ')[4]
            avg += float(elapsed_time)
        avg = round((avg/tests_toRun), 5)

        print('Parallel algo n_threads = ' + str(nt) +
              ' -> ' + str(avg))
