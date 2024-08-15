# Performance Benchmarks for CAS-Based (Compare-and-Swap) MPSC Queue

| Producers | 1 | 3 | 5 | 7 |
| --- | --- | --- | --- | --- |
| Mean push time (cycles) | 191 | 680 | 976 | 645 |
| Median push time (cycles) | 180 | 521 | 300 | 560 |
| 99th percentime push time (cycles) | 420 | 3560 | 1460 | 2620 |
| 99.99th percentime push time (cycles) | 1060 | 14280 | 7940 | 16300 |

The results for the 3-producer system look a bit odd, but the rest seem fairly intuitive. As contention on the list increases, threads spin more waiting to acquire an index in the queue and so latency increases.

## No concurrent writers RDTSC measured times
![image](https://github.com/mastamysta/MPSC-Queue/assets/47383446/87019dbc-8d90-47a9-bc43-b638e98141b0)


## 3 concurrent writers RDTSC measured times
![image](https://github.com/mastamysta/MPSC-Queue/assets/47383446/907f8eb2-2f1a-4e68-a178-eb359abcf209)


## 5 concurrent writers RDTSC measured times
![image](https://github.com/mastamysta/MPSC-Queue/assets/47383446/0cc81e74-4318-444f-bb4f-b8943406383a)


## 7 concurrent writers RDTSC measured times
![image](https://github.com/mastamysta/MPSC-Queue/assets/47383446/a651a173-4057-4ded-9803-2c59bc5d2381)

## 9 concurrent writers RDTSC measured times:

The formance with 9 concurrent writers was so poor that I wasn't able to get a good result. The program either ran for hours or the warmups didn't overlap properly and the program terminated early.

# In comparison with the FA-Based (Fetch-and-Add) MPSC Queue

| Producers | 1 | 3 | 5 | 7 | 9 |
| --- | --- | --- | --- | --- | --- |
| Mean push time (cycles) | 191 | 252 | 607 | 720 | X |
| Median push time (cycles) | 200 | 200 | 340 | 320 | 220 |
| 99th percentime push time (cycles) | 680 | 498 | 3320 | 3240 | 2060 |
| 99.99th percentime push time (cycles) | 760 | 1520 | 8280 | 3700 | 9840 |

## No concurrent writers RDTSC measured times
![image](https://github.com/mastamysta/MPSC-Queue/assets/47383446/db15eba5-9994-4d46-bc40-f243f7be3c52)


## 3 concurrent writers RDTSC measured times
![image](https://github.com/mastamysta/MPSC-Queue/assets/47383446/f1b27ecc-e675-49f3-9e1c-ed10bc0c7ff8)


## 5 concurrent writers RDTSC measured times
![image](https://github.com/mastamysta/MPSC-Queue/assets/47383446/23a14368-b99e-4077-9d47-d5ec8f77b550)


## 7 concurrent writers RDTSC measured times
![image](https://github.com/mastamysta/MPSC-Queue/assets/47383446/99d49757-9cc9-4ddb-8308-2a21aa62cad5)

## 9 concurrent writers RDTSC measured times:
![image](https://github.com/mastamysta/MPSC-Queue/assets/47383446/7a0bdf89-34a3-4811-a33d-00f3a0feed58)

Notablby, the latency of the non-spinning queue scales *much* better with number of producers. We're even able to get some results for 9 producers with ~200 cycles of latency. Not too bad!

# Development and Debug

Here are some RDTSC results from experiments with the compare-and-swap (CAS) queue before I'd done any debug. These results are clearly a bit odd, for starters the uncontended list performed worse than the list with 9 contentious writers. I did a bit of investigation (shown below) and found that the cause was a combo of:
 - The results data was buffered in memory for the full duration of the program and then dumped at the end. The pipeline was stalling waiting for changes to commit back to this results buffer.
 - Not allowing enough warmup cycles for the single-threaded application to give reliable results. Allowing for 10-100 million warmup cycles improved this dramatically.

| Producers | 1 | 3 | 5 | 9 |
| --- | --- | --- | --- | --- |
| Mean push time (cycles) | 392 | 261 | 323 | 280 |
| Median push time (cycles) | 390 | 280 | 327 | 270 |
| 99th percentime push time (cycles) | 557 | 498 | 569 | 503 |
| 99.99th percentime push time (cycles) | 738 | 27232 | 35358 | 38237 |

No concurrent writers RDTSC measured times:
![image](https://github.com/mastamysta/Shared_memory_experiment/assets/47383446/809b9b29-cdeb-4d74-8d9f-6a0cf3ad6f88)

3 concurrent writers RDTSC measured times:
![image](https://github.com/mastamysta/Shared_memory_experiment/assets/47383446/c97681be-d7ee-4698-b431-3d180f1ab92e)

5 concurrent writers RDTSC measured times:
![image](https://github.com/mastamysta/Shared_memory_experiment/assets/47383446/0e571aa0-33a0-4bd8-a212-8225f5a42b02)

9 concurrent writers RDTSC measured times:
![image](https://github.com/mastamysta/Shared_memory_experiment/assets/47383446/24d7d6a5-d79a-42e0-8c96-188cfb752819)

Initially, the timing data was stored in memory and dumped to disk at program termination. I did this rather than using some separate logging thread because that would require an MPSC-queue and I didn't want to have to worry about second order effects due to logging with the same code that was being measured.

perf data on the deferred logging approach:

 Performance counter stats for './creator 5':

          1,813.48 msec task-clock                #    3.995 CPUs utilized          
               296      context-switches          #  163.222 /sec                   
                 7      cpu-migrations            #    3.860 /sec                   
             5,391      page-faults               #    2.973 K/sec                  
     5,489,758,361      cycles                    #    3.027 GHz                      (62.83%)
        24,304,631      stalled-cycles-frontend   #    0.44% frontend cycles idle     (63.52%)
     4,919,669,301      stalled-cycles-backend    #   89.62% backend cycles idle      (63.22%)
     1,621,028,803      instructions              #    0.30  insn per cycle         
                                                  #    3.03  stalled cycles per insn  (62.70%)
       243,012,678      branches                  #  134.004 M/sec                    (62.66%)
           743,881      branch-misses             #    0.31% of all branches          (62.60%)
       885,281,809      L1-dcache-loads           #  488.168 M/sec                    (62.46%)
        15,952,739      L1-dcache-load-misses     #    1.80% of all L1-dcache accesses  (62.11%)
   <not supported>      LLC-loads                                                   
   <not supported>      LLC-load-misses                                             

       0.453991307 seconds time elapsed

       1.786107000 seconds user
       0.027984000 seconds sys

Due to the high number of cycles spent stalling in the backed (presumably on memory, since there isn't any other IO going on here), I decided to try and eliminate the main memory requirement of the `creator` process by dumping all measured results at runtime. I'm lazy so I synchronised that with a mutex, which might confound our results somewhat. Below are the perf results under the new regime:

 Performance counter stats for './creator 5':

            616.39 msec task-clock                #    2.476 CPUs utilized          
            21,132      context-switches          #   34.284 K/sec                  
               116      cpu-migrations            #  188.194 /sec                   
               308      page-faults               #  499.687 /sec                   
     1,592,409,590      cycles                    #    2.583 GHz                      (62.83%)
        89,633,838      stalled-cycles-frontend   #    5.63% frontend cycles idle     (59.15%)
       697,052,023      stalled-cycles-backend    #   43.77% backend cycles idle      (59.49%)
     2,250,983,878      instructions              #    1.41  insn per cycle         
                                                  #    0.31  stalled cycles per insn  (61.75%)
       375,295,519      branches                  #  608.864 M/sec                    (64.37%)
         2,110,269      branch-misses             #    0.56% of all branches          (66.79%)
     1,150,982,238      L1-dcache-loads           #    1.867 G/sec                    (65.86%)
        10,033,224      L1-dcache-load-misses     #    0.87% of all L1-dcache accesses  (64.93%)
   <not supported>      LLC-loads                                                   
   <not supported>      LLC-load-misses                                             

       0.248970440 seconds time elapsed

       0.443600000 seconds user
       0.173510000 seconds sys

There's a pretty large jump in performance there. The obvious change is that the number of cycles spent stalling in the backend *halved* and the runtime reduced by ~40%. I was a bit surprised by this, since we presumably now spend a decent amount of time blocking on the dump file mutex.

# Impacts of alignas() to reduce false sharing on FA-based queue

Aligning each data and valid flag to a cache line should reduce false sharing and improve latencies. All experiments here are with 10 producers contending over the queue:

Here are the stats for the implementation which is cache-line naive:
```
      1,111,659.71 msec task-clock                       #    7.778 CPUs utilized             
           788,002      context-switches                 #  708.852 /sec                      
            18,233      cpu-migrations                   #   16.402 /sec                      
               320      page-faults                      #    0.288 /sec                      
 4,047,473,558,026      cycles                           #    3.641 GHz                         (71.42%)
     7,429,226,236      stalled-cycles-frontend          #    0.18% frontend cycles idle        (71.42%)
   314,524,789,636      instructions                     #    0.08  insn per cycle            
                                                  #    0.02  stalled cycles per insn     (71.43%)
    45,421,602,269      branches                         #   40.859 M/sec                       (71.43%)
       118,925,484      branch-misses                    #    0.26% of all branches             (71.43%)
   186,022,353,334      L1-dcache-loads                  #  167.337 M/sec                       (71.44%)
     6,397,230,364      L1-dcache-load-misses            #    3.44% of all L1-dcache accesses   (71.43%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

     142.929940019 seconds time elapsed

    1103.003503000 seconds user
       7.913438000 seconds sys
```
![image](https://github.com/user-attachments/assets/89a31690-4647-467f-93b1-23e13fdca492)

count     5.000000e+06
mean      2.750125e+02
std       3.639226e+06
min      -5.746573e+09
50%       1.200000e+02
99%       1.940000e+03
99.99%    9.580005e+04
max       5.761637e+09

... and here are the stats for the same implementation but segmenting data and valid flags by cache line.
```
        620,954.54 msec task-clock                       #    7.722 CPUs utilized             
           799,491      context-switches                 #    1.288 K/sec                     
            18,398      cpu-migrations                   #   29.629 /sec                      
               357      page-faults                      #    0.575 /sec                      
 2,377,107,163,786      cycles                           #    3.828 GHz                         (71.43%)
     5,920,377,786      stalled-cycles-frontend          #    0.25% frontend cycles idle        (71.45%)
   198,627,812,097      instructions                     #    0.08  insn per cycle            
                                                  #    0.03  stalled cycles per insn     (71.43%)
    29,391,809,979      branches                         #   47.333 M/sec                       (71.42%)
        99,778,647      branch-misses                    #    0.34% of all branches             (71.42%)
   115,880,473,484      L1-dcache-loads                  #  186.617 M/sec                       (71.42%)
     3,975,936,415      L1-dcache-load-misses            #    3.43% of all L1-dcache accesses   (71.42%)
   <not supported>      LLC-loads                                                             
   <not supported>      LLC-load-misses                                                       

      80.411673381 seconds time elapsed

     611.762058000 seconds user
       8.521569000 seconds sys
```
![image](https://github.com/user-attachments/assets/28d55966-c110-44e8-ab94-02dcec12c453)

count     5.000000e+06
mean      5.295631e+02
std       1.045284e+04
min       2.000000e+01
50%       2.000000e+02
99%       2.060000e+03
99.99%    1.542200e+05
max       8.071400e+06
