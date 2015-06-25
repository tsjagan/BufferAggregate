# BufferAggregate
A simple implementation of buffer aggregates.

- Threads/multiple modules in a software stack pass along buffers amongst them all the time.
- They usually pass these buffers by making local copies or using concurrency control to protect
  accesses to these buffers.
- And copying itself is an expensive operation. Also, redundant copies of data, memory usage etc.
- BufferAggregate is an ADT that is a container for multiple buffers.
  Instances of this ADT can be passed by value among threads. So each thread will have its own
  local copy. The buffers inside are immutable in essence so no locking is required to access them.
  Writes are supported by using (read-copy-update).
- Please refer to the paper below for clearer understanding.
- This is a simple implementation of a Bufferaggregate data structure.

Motivation for this project is from a paper/project called
IOLite, by some people from Rice University. Below is the link for the paper.
https://www.usenix.org/legacy/publications/library/proceedings/osdi99/full_papers/pai/pai_html/node5.html

That said, this project does not implement the above paper. So the guarantees and performance may widely differ.

PS:
- Requires. c++11 compiler
