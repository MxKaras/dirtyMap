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
`/usr/local/include` (or wherever you'd like) and:

```c++
#include <dirtyMap/Hashmap.hpp>
...
drt::Hashmap<int, int> m;
drt::Hashmap<int, int, 1000> k; // specify number of objects per pool
```

If you want to compile the tests do a CMake out of source build,
although past the first one I'd recommend compiling the benchmarks
yourself:

> g++ -std=c++11 -O2 -I ../../ \<file\>.cc -o name
