import subprocess

subprocess.run('gcc -fopenmp scan.c -o parallel_scan -lm', shell=True)
subprocess.run('gcc scan.c -o serial_scan -lm', shell=True)


print('Let\'s begin finding the best scheduling type on a medium size (1024) vector with number of threads = 4')

scheduling_types = ['static', 'dynamic', 'guided']
vecsize = 2**12
tests_toRun = 5

results = {} # python dictionary containing the average execution time for each schedule type

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


# Let's now compare the serial algo with the parallel one using the best scheduling type for the for loops

vec_powers = [4, 8, 12, 13, 14, 15]
numberOfThreads = [2, 4, 6, 8]

tests_toRun = 10

for p in vec_powers:
    vecsize = 2**p
    print('\n-------VECSIZE = ' + str(vecsize)+' ----------')

    avg = 0
    command = './serial_scan ' + str(vecsize)
    for step in range(tests_toRun):
        elapsed_time = subprocess.check_output(
            command, shell=True).decode('utf-8').split('\n')[1].split(' ')[4]
        avg += float(elapsed_time)
    avg = round((avg/tests_toRun), 5)

    print('Serial algo -> ' + str(avg) +' millisecs;')


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
