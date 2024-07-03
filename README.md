Here are some RDTSC results from experiments with the compare-and-swap (CAS) queue:

Producers | 1 | 3 | 5 | 9 |
--- | --- | --- | --- |--- |--- |
Mean push time (cycles) | 392 | 261 | 323 | 280 |
--- | --- | --- | --- |--- |--- |
Median push time (cycles) | 390 | 280 | 327 | 270 |
--- | --- | --- | --- |--- |--- |
99th percentime push time (cycles) | 557 | 498 | 569 | 503 |
--- | --- | --- | --- |--- |--- |
99.99th percentime push time (cycles) | 738 | 27232 | 35358 | 38237 |

No concurrent writers RDTSC measured times:
![image](https://github.com/mastamysta/Shared_memory_experiment/assets/47383446/809b9b29-cdeb-4d74-8d9f-6a0cf3ad6f88)

3 concurrent writers RDTSC measured times:
![image](https://github.com/mastamysta/Shared_memory_experiment/assets/47383446/c97681be-d7ee-4698-b431-3d180f1ab92e)

5 concurrent writers RDTSC measured times:
![image](https://github.com/mastamysta/Shared_memory_experiment/assets/47383446/0e571aa0-33a0-4bd8-a212-8225f5a42b02)

9 concurrent writers RDTSC measured times:
![image](https://github.com/mastamysta/Shared_memory_experiment/assets/47383446/24d7d6a5-d79a-42e0-8c96-188cfb752819)

