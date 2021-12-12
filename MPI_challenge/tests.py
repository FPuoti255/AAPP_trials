import subprocess

veclen= [729, 997, 1024]
numberOfProcesses = [2, 3, 5]

results = []
testsToRun= 5
avg=0

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
    results.append([np, vl, (avg/testsToRun)])

print(results)