~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PROJECT 3 - Hamayel Qureshi & Mago Sheehy ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This project was very straightforward. No major errors or difficulties were encountered. 

init():

We found that casting size_t size to an int would pass the tests but produce an incorrect output when compared to 
output_ref.txt. To solve this, we cast size_t size to a double which produces the correct output, and we check for
an incorrect/invalid/bad argument by seeing if (double)size > MAX_ARENA_SIZE.

destroy():

A very straightforward function to write. We simply called munmap() at the _arena_start address and destroyed up 
till the size of the arena (defined in _arena_size).

walloc():

Again, straightforward to implement. The only thing we worried about originally was the amount of casting taking place.
Unsure if there is a more 'elegant' way of taking care of this. We had a small issue here with line 72 of the test file,
but realized we had to return the address of the allocated block instead of the header.

free():

We originally though wfree() might have to be a recursive function and call itself in the coalescing implementation. However,
it turns out that wfree() is only expected to combine contiguous blocks of memory (either fwd or bwd). We have a variable 'frees'
to match the output in output_ref.txt which tells us which coalescing test case is being run.