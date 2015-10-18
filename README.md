# sequence-alignment
An implementation of the Smith Waterman algorithm in C++, with C++11 threads.

## Abstract
A single-threaded and multi-threaded version of the program was tested and compared.  Results indicate limited speedup on modern hardware using threads.

## High Level Design
- The single threaded version goes row-by-row through the matrix and calculates the score.
- Insert image
- The multi-threaded version dispatches threads on each row to process until the last row.  If one thread gets ahead of the other it waits until the "top" thread fulfills the bottom threads dependency
- Insert Image

## Implementation
- C++14 standard library with Clang++/LLVM was chosen due to availability of Mac OS.  C++11 threads were used in the threaded version due to the more simple implementation.  
- The program accepted arguments for number of threads.  If the argument for threads is equal to one, none of the C++ threading API was called.  If the number of threads was 2 or more, the known sequence (smaller) was split up between the available threads, going in order from top to bottom.
- Level 2 optimizations were enabled for compilation.
- I figured I could start threads on equal parts of the diagonal line.  Some processes would be wasted, but probably still end up with a speed up in the end; with minimal extra memory usage.  The "start spots" would be equally placed along the diagonal.
![diagram](https://raw.githubusercontent.com/bvanderhaar/sequence-alignment/master/docs/matrix-processing.png)

## Testing conditions
A single Macbook Pro, Early 2015 with a 2.7GHz Core i5 processor with 8GB of DDR3 RAM was used for testing.  Each test was run for 4 iterations to ensure consistent test results.  The program was run with threads between 1-10, 12 and 16 threads and compared.  The clock API of C++ was used to record only the processing time of both the threaded and non-threaded code execution.  The time for initialization and file reading was ignored in all cases.  

## Discussion
In every case, the symmetrical / single-threaded version of the application ran faster than the parallelized version on the MacBook Pro.  
