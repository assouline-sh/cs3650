# Threaded Merge Sort Experiments


## Host 1: Shannon's Mac

- CPU: Apple M1
- Cores: 8 
- Cache size (if known):
- RAM: 16 GB
- Storage (if known): 1TB 
- OS: Version 11.6 Big Sur

### Input data
*Briefly describe how large your data set is and how you created it. Also include how long `msort` took to sort it.*
Our input data is a hundred million numbers. We created this by shuffling a hundred million numbers using the shuf command and directing this output to a file named hundred-million.txt. The exact command we used was:
shuf -i 1-100000000 > hundred-million.txt

Then we ran msort with this dataset with the following command:
./msort 100000000 < hundred-million.txt > /dev/null

The resulting time it took msort to sort was:
Sorting completed in 17.047069 seconds. 

### Approximate Number of Processes Running Before Running Each Experiment
422

### Experiments

#### 2 Threads
Command used to run experiment:
MSORT_THREADS=2 ./tmsort 100000000 < hundred-million.txt > /dev/null

Sorting portion timings:
1. 25.537134 seconds
2. 25.543118 seconds
3. 25.527074 seconds
4. 25.480379 seconds


#### 4 Threads
Command used to run experiment:
MSORT_THREADS=4 ./tmsort 100000000 < hundred-million.txt > /dev/null

Sorting portion timings:
1. 12.636794 seconds
2. 12.692858 seconds
3. 12.673672 seconds
4. 12.668964 seconds


#### 8 Threads
Command used to run experiment:
MSORT_THREADS=8 ./tmsort 100000000 < hundred-million.txt > /dev/null

Sorting portion timings:
1. 7.051433 seconds
2. 7.140593 seconds
3. 7.067937 seconds
4. 7.104097 seconds


#### 10 Threads
Command used to run experiment:
MSORT_THREADS=10 ./tmsort 100000000 < hundred-million.txt > /dev/null`

Sorting portion timings:
1. 7.351072 seconds
2. 7.955926 seconds
3. 7.539172 seconds
4. 7.524035 seconds



## Host 2: XOA VM

- CPU: Intel(R) Xeon(R) CPU E5-2690 v3
- Cores: 12 (can only use 2 out of 12)
- Cache size (if known):
- RAM: 3.8 GB
- Storage (if known): 
- OS: Ubuntu 22.04.1 LTS (Jammy Jellyfish)

### Input data
*Briefly describe how large your data set is and how you created it. Also include how long `msort` took to sort it.*
Our input data is a hundred million numbers. We created this by shuffling a hundred million numbers using the shuf command and directing this output to a file named hundred-million.txt. The exact command we used was:
shuf -i 1-100000000 > hundred-million.txt

Then we ran msort with this dataset with the following command:
./msort 100000000 < hundred-million.txt > /dev/null

The resulting time it took msort to sort was:
Sorting completed in 22.807532 seconds. 

### Approximate Number of Processes Running Before Running Each Experiment
111

### Experiments

#### 2 Threads
Command used to run experiment: 
MSORT_THREADS=2 ./tmsort 100000000 < hundred-million.txt > /dev/null

Sorting portion timings:
1. 26.212108 seconds
2. 25.996220 seconds
3. 25.887593 seconds
4. 26.095431 seconds


#### 4 Threads
Command used to run experiment: 
MSORT_THREADS=4 ./tmsort 100000000 < hundred-million.txt > /dev/null

Sorting portion timings:
1. 13.785464 seconds
2. 13.882600 seconds
3. 13.790296 seconds
4. 14.143854 seconds


#### 8 Threads
Command used to run experiment: 
MSORT_THREADS=8 ./tmsort 100000000 < hundred-million.txt > /dev/null

Sorting portion timings:
1. 14.071813 seconds
2. 14.068618 seconds
3. 14.052956 seconds
4. 14.130163 seconds


#### 10 Threads
Command used to run experiment:
MSORT_THREADS=10 ./tmsort 100000000 < hundred-million.txt > /dev/null

Sorting portion timings:
1. 20.968235 seconds
2. 13.924966 seconds
3. 18.131517 seconds
4. 19.009832 seconds


#### 16 Threads
Command used to run experiment: 
MSORT_THREADS=16 ./tmsort 100000000 < hundred-million.txt > /dev/null

Sorting portion timings:
1. 14.071813 seconds
2. 14.578525 seconds
3. 14.461162 seconds
4. 13.809148 seconds


## Observations and Conclusions
*Reflect on the experiment results and the optimal number of threads for your concurrent merge sort implementation on different hosts or platforms. Try to explain why the performance stops improving or even starts deteriorating at certain thread counts.*
For the experiments on Host #1: M1 Mac, the optimal number of threads for concurrent merge sort seems to be 8 threads. When we ran the same experiment on 10 threads, there was no noticeable improvement in performance. Using threads improved the run time for merge sorting by about 10 seconds, compared to the run time without using threads.
For the experiments on Host #2: XOA VM, the optimal number of threads for concurrent merge sort seems to also be 8 threads. When we ran the same experiment on 10 or 16 threads, there was no noticeable improvement in performance. In fact, for 10 threads the results became more irregular and run times sometimes exceeded that of 8 threads. Using threads improved the run time for merge sorting by about 8 seconds, compared to the run time without using threads.
For both machines, at a certain point the performance stops improving, regardless of increasing the number of threads. There are many possible reasons for this, the most likely being that at a certain point, threading can sometimes cause too much overhead that it hinders performance. The work needed to create, join, and end threads may not be efficient if too many threads are running such that each thread has too little work to do to make creating a thread worth it. Also, threads all share the hardware resources of the machine they are running on. These resources are limited, and thus at a certain point, these limitations will limit performance even if more threads are used. 

