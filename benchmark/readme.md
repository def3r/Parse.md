## Benchmark
> Benchmark, as run on 12th Gen Intel Core i5-12500H(4.5 GHz max) runnin Arch Linux(kernel 6.16.4), compiled with g++(15.2.1) after compiler optimizations

Throughput: 21 MB/s

| Benchmark | Time | CPU | Iterations |
|:----------|-----:|----:|-----------:|
| ParseMd Empty String  |    0.038 us |        0.038 us  |  18497943 |
| ParseMd Simple String |     4.17 us |       4.17 us    |  168443 |
| ParseMd 100KB         |     4572 us |       4562 us    |     155 |
| ParseMd 1MB           |    49430 us |      49317 us    |      14 |
