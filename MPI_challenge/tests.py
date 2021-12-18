import subprocess
import numpy 

veclen= [729, 997, 1024]
numberOfProcesses = [2, 3, 5]

results = []
testsToRun= 10
avg=0

subprocess.run('mpicxx parallelMergeSort.cpp helpers.cpp -o mpi_mergesort', shell=True)

for np in numberOfProcesses:
  for vl in veclen:
    command = 'mpiexec -np ' + str(np) + ' --hostfile my-hostfile ./mpi_mergesort ' + str(vl)
    avg = 0
    for i in range(testsToRun):
      output = subprocess.check_output(command, shell=True).decode('utf-8').split(' ')
      try:
        tm = float(output[5])
        avg +=tm
      except:
        i -= 1 
    results.append([np, vl, round((avg/testsToRun), 3)])

results = numpy.array(results)

for el in results:
  dspl = 'Using ' + str(int(el[0]))+' processes with an array of ' +  str(int(el[1])) +' it took on average '+ str(el[2])+' millisecs \n'
  print(dspl)