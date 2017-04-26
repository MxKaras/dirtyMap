# dirtyMap

dirtyMap is a memory-efficient hash map implementation
using separate chaining. Buckets eliminate the need for nullptr-terminated
lists through the use of dirty bits (hence the name).

This started as a university project, that I am bringing
into the world and will be working on in my spare time.
It's not production ready yet, but feel free to take a look.

I've only tested the code on Ubuntu, with GCC 5.4.
No guarantees are made concerning it's compatibility in other
situations.

Installation is easy - drop the into /usr/local/include

TODO: update this readme