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
