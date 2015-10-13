# sequence-alignment
An implementation of the Smith Waterman algorithm in C++, with pthreads.

## Pointer Hell
- It was a pain to get the pointer logic down with the struct. As you can see my pthread method is very difficult to read. [Code Example](https://github.com/bvanderhaar/sequence-alignment/blob/master/main.cpp#L57)
- Now I'm getting a segmentation fault when running my code over the vector<vector<int>>.  I think I need a mutex to control writes to this object

## The right optimization
- At what level do you split up the matrices?
- I figured I could start threads on equal parts of the diagonal line.  Some processes would be wasted, but probably still end up with a speed up in the end; with minimal extra memory usage.  The "start spots" would be equally placed along the diagonal.
![diagram](https://raw.githubusercontent.com/bvanderhaar/sequence-alignment/master/docs/matrix-processing.png)
- I also thought optimizing for the "squares" in the matrix - i.e. the longer string would be broken up into smaller pieces.  But my challenge was patching the matrices back together seemed too complex
- For starters, I tried three static threads, feeding them look-ahead numbers in the same for loop as the static

## Speedup Examples
Using a flat, single-threaded algorithm, my results were:
- Mid 2012 15" MBP
- 2.3GHz Core i7
- 8GB RAM
- 11 minutes (673 Seconds)

I'm hypothesising that a parallel version will at least see a 2x speedup based on the parallel ability of the problem and the overhead of keeping track of threads.
