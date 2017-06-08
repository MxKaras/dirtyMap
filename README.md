# dirtyMap

dirtyMap is a memory-efficient hash map implementation
using separate chaining. Buckets eliminate the need for nullptr-terminated
lists through the use of dirty bits (hence the name).
Furthermore, a custom-built memory allocator is used to provide fast and
cache friendly access to stored objects.

This started as a university project that I am bringing
into the world and will be working on in my spare time.
There are still a few kinks to iron out before it's production ready,
but feel free to take a look.

I've only tested the code on Ubuntu, with GCC 5.4.
No guarantees are made concerning it's compatibility in other
situations.

Installation is easy - drop the dirtyMap directory into
`/usr/local/include` and:

```c++
#include <dirtyMap/Hashmap.hpp>
...
drt::Hashmap<int, int> m;
```

If you want to compile the tests do a CMake out of source build,
although past the first one I'd recommend compiling the benchmarks
yourself:

> g++ -std=c++11 -O2 -I ../../ \<file\>.cc -o name

There are two important points to make. Firstly, issues with
concurrency were ignored (although concurrent reads should be
fine). Secondly, if you need to regularly delete elements then I
would warn you against using dirtyMap. It was not designed with
that in mind.

### Background

The motivating use case for this project is in AI Planners, which
store state information in hash maps/sets, and store them for the
duration of the run. The limiting factor here
is the amount of memory available, which limits the size and
complexity of the problem that can be tackled with planners.

I have been told that using Google's [sparsehash](https://github.com/sparsehash/sparsehash)
was tried, but the overhead involved in maintaining such a
small memory footprint impacted the planner's running time too much.

Thus, we have dirtyMap, which uses a fairly simple strategy
to conserve memory (8 bytes per bucket) combined with a custom
allocator to provide fast allocation and good cache performance.
