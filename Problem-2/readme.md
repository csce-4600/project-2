[Note: I'll update or delete this file once my code is prettified.]

Here's a basic overview of my implementation:

I packaged (most) everything inside a "memoryManagement" class, except for a few structs and some odds and ends.
My approach was "best fit first". I had to restructure a few times (hence the plague of comment-code), mainly
because I wanted to keep track of 'metadata' using a dynamic container from STL, and had pointed out to me
later on that said containers utilize 'malloc()', this negating our goal of NOT utilizing 'malloc()'.

So our memory space is made up of 3 main parts:
  1) 'void * memBank' : this is the 10MB chunk of memory that is allocated up front, to be divided among the 'processes'
  2) 'memPart * memPartArray' : this array (of size 10240 - 1 cell per kB of memory) keeps track of the state of each kB
        block of memory in 'memBank'.
  3) 'freeChunks * chunkArray' : This array (of max size 10240 - 1 cell per kB of memory - but mostly unused) maintains a list 
        of all free memory partition locations (eg. where the front of each free mem slice is), all pushed to the front
        of the array. Each cell (that's actually being used) stores in it a) an int offset for 'memPartArray' to find the
        front of the slice, and a slice size. This list (unordered, btw) allows a process to analyze all free slices and
        quickly find the one of best fit, e.g. slice memory > process memory && slice memory < all_other_slice memory.
  
That's the gist of everything. Right now, if a free slice w/ sufficient memory is unavailable, the program returns an error
(which isn't a problem for this program's current implementation, but could be for a parallel implementation), but I plan to
utilize a semaphore or pseudo-semaphore to handle this relatively soon. I plan to essentially have the process wait until
another process executed 'my_free()', then scan for a sufficient slice again, lather, rinse, repeat. I'll get to that later on.

All that being said, yes, my code may be a little clunky, but it works. I may clean up in-code later. We'll see.
