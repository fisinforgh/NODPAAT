# NasaCodes

To compile:

```cpp
  g++ -O3 -march=native -std=c++17 optimized_ozone_processor.cpp -o optimized_ozone_processor
```

Usage examples:

Parallel processing with 4 threads

```cpp
./optimized_ozone_processor pgrid /path/to/data/ -90 90 10 7 4
```

Sequential processing (original behavior)

```cpp
./optimized_ozone_processor grid /path/to/data/ -90 90 10 6
```

Single location

```cpp
./optimized_ozone_processor location BOG /path/to/data/ 4.36 -74.04 6
```

To compile chi2LinearRelStudyO3vsSn:

```cpp
g++ -g -o chi2LRSO3vsSnRunApp chi2LinearRelStudyO3vsSn.cxx `root-config --cflags --libs`
```

To execute chi2LRSO3vsSnRunApp run the .sh file

```bash
 ./bulkChi2LRSO3vsSnRunApp.sh 7 10 1.1
```

```cpp
g++ -o grid_analysis analysis_runner.cpp -std=c++11 -pthread
