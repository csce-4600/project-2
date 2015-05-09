//my_malloc.h - defined 'my_malloc()' and 'my_free()' for Project #2, Question #2

/*------------------------------------------------------+
|                                                       |
|   Implementation of Custom Memory Management System   |
|                                                       |
+------------------------------------------------------*/

//included for main program file
#include <iostream>
#include <random>
#include <cstdio>
#include <ctime>
#include <unistd.h>
#include <chrono>
//included for this addition
#include <cerrno>

using namespace std;

enum bule {TROO, FALLCE, NOLE};	//special bool - allows true/false/null (or troo/fallce/nole respectively)

							/*-----------------------------------------------------------------------------------+
							|  memPartArray (defined below) is a static metadata array holding values for each   |
							|  kilobyte sized unit of memory in the 10MB allocated space. Each subsequent cell   |
							|  refers to each subsequent kilobyte in the allocated space. For example, (pseudo)  |
							|  >void * memBank = realAllocatedMemorySpace;                                       |
							|  >memPartArray[0].reference -> realAllocatedMemorySpace;                           |
							|  >memPartArray[5].reference -> (realAllocatedMemorySpace + 5);                     |
							|  This is the more human-readable component. Configuration as follows.              |
							+-----------------------------------------------------------------------------------*/

struct memPart 		//Memory Partition - will represent metadata of all kilobytes in a static array
{
	bool isChunkStart;	//Array will only utilize cells that represent the start of a hole or memory block, thus minimizing time
	bule isFree;		//Differentiates between a hole and an allocated chunk - utilizes "boolean third option" to represent non-value for all cells other than start cells
	int prevChunk;		//int value pointing to previous start cell
	int chunkSize;		//int value can be used to determine next start cell's location on array
	void Re_memPart()		//all non-ChunkStart partitions initialize "isChunkStart" to false, and all other variables to non-values
	{
		isChunkStart = false;
		isFree = NOLE;
		prevChunk = -1;		//default value - if = -1, is first cell
		chunkSize = -1;		//default value - if = -1, is not start cell
	}
	memPart()			//implementing the constructor this way allows for "reconstruction" - ie. resetting to default values
	{
		Re_memPart();		//to reset values to default, run "Re_memPart()".
	}
};

							/*-----------------------------------------------------------------------------------+
							|  chunkArray (defined below) is a static metadata array holding values for each     |
							|  each location in memory that is the start of a hole. All cells are initialized    |
							|  as non-value placeholders, and each new reference is added in the next inactive   |
							|  "placeholder" cell. Also, after each reference removal, the last active cell's    |
							|  information is relocated to the removed reference's cell. This ensures the first  |
							|  "placeholder" cell reached will be the first cell after all references.           |
							+-----------------------------------------------------------------------------------*/

struct freeChunks 		//second metadata array - manages ONLY details of holes, not allocated chunks
{							//in this array, details on all holes are inserted, unordered, at front of array for easy searching
	int chunkSize;		//same purpose as in "memPart"
	int chunkStart;		//int index value for corresponding cell in "memPart" array
	void Re_freeChunks()
	{
		chunkSize = -1;		//default value - if = -1, is not start cell (ie. current cell is non-value placeholder)
		chunkStart = -1;	//default value - if = -1, is not start cell (ie. current cell is non-value placeholder)
	}
	freeChunks()		//same constructor scheme as "memPart"
	{
		Re_freeChunks();	//to reset values to default, run "Re_memPart()".
	}
};

class memoryManagement {		//Memory Management System
	private:
		int totChunks;		//maintains a count of holes in memory - used to limit traversal of 'chunkArray'
		void * memBank;		//pointer to start of actual allocated memory
		memPart * memPartArray;		//see above explanation of "memPart" struct
		freeChunks * chunkArray;	//see above explanation of "freeChunks" struct
		freeChunks workingChunk;	//extra temp "chunk" object for use in traversing "chunkArray" and comparing values
		int findFree(int blocks)	//function to locate and return location data for the hole of best fit for the passed memory footprint (in kB)
		{
			workingChunk.Re_freeChunks();	//resets workingChunk to default values
			for(int i = 0; i < totChunks; i++)
			{
				if(chunkArray[i].chunkSize >= blocks)	//if hole is large enough
				{
					if(workingChunk.chunkSize == -1 || workingChunk.chunkSize > chunkArray[i].chunkSize)
					{		//if hole is first found that's large enough, OR is smaller than currently referenced hole
						workingChunk.chunkSize = chunkArray[i].chunkSize;	//save current hole's size
						workingChunk.chunkStart = i;	//replace saved reference with current reference value
					}
				}
			}
			return workingChunk.chunkStart;	//return location data of hole of best fit
		}
		int freeArrays(int s, int p, int c, int n)	//s: situation, p: previous, c: i (or current), n: next
		{		//functions to handle all possible memory chunk freeing scenarios
						/*------------------------------------------------------------------------------+
						|  Possible scenarios include the following:                                    |
						|      1) previous chunk = allocated, next chunk = allocated                    |
						|      2) previous chunk = free, next chunk = allocated                         |
						|      3) previous chunk = allocated, next chunk = free                         |
						|      4) previous chunk = free, next chunk = free                              |
						|                                                                               |
						|  Unique scenarios exist if current chunk has no previous and/or next chunks.  |
						|    However, appropriate actions match those taken in the above situations.    |
						+------------------------------------------------------------------------------*/
			if(s == 1)
			{
				//free [i] and add 1 to 'totChunks'
					//memPartArray - re: Step 2
				memPartArray[c].isFree = TROO;		//mark start cell as free
					//chunkArray - re: Step 3
				chunkArray[totChunks].chunkStart = c;	//add reference to start location to 'chunkArray'
				chunkArray[totChunks].chunkSize = memPartArray[c].chunkSize;	//add chunk's size to same
				totChunks++;	//increment count of holes
				return 0;
			}
			else if(s == 2)
			{
				//expand [previous] to include [i] and leave 'totChunks' the same
					//memPartArray - re: Step 2
				memPartArray[p].chunkSize += memPartArray[c].chunkSize;		//increase prev. chunk's size to include current chunk's size
				memPartArray[n].prevChunk = memPartArray[c].prevChunk;		//point next chunk to prev. chunk for its 'previous chunk' reference
				memPartArray[c].Re_memPart();		//reset current cell's values to non-value default - cell is no longer a start reference
					//chunkArray - re: Step 3
				int i = 0;
				for(i = 0; i < totChunks; i++)	//find [previous] memory reference in unordered 'chunkArray'
				{		//if not found, 'i' will equal 'totChunks'. if found, size value will adjust and 'i' will equal 'totChunks + 1'
					if(chunkArray[i].chunkStart == p)
					{
						chunkArray[i].chunkSize = memPartArray[p].chunkSize;
						i = totChunks;
					}
				}
				if(i == totChunks + 1) return 0;
				else
				{
					//return error
					cout << "Error - 'freeArrays()' chunkArray Error" << endl << "\tError Code: freeArrays(-2)" << endl;
					return -2;
				}
			}
			else if(s == 3)
			{
				//expand [next] backwards to include [i] and leave 'totChunks' the same
					//memPartArray - re: Step 2
				memPartArray[c].chunkSize += memPartArray[n].chunkSize;		//increase current chunk's size to include next chunk's size
				int x = memPartArray[n].chunkSize;		//locate next active cell following [n]
				memPartArray[n + x].prevChunk = c;		//set said cell's 'prevChunk' to current chunk
				memPartArray[n].Re_memPart();			//reset 'next' cell's values to non-value default - cell is no longer a start reference
					//chunkArray - re: Step 3
				int i = 0;
				for(i = 0; i < totChunks; i++)	//find 'next' memory reference in unordered 'chunkArray'
				{		//if not found, 'i' will equal 'totChunks'. if found, start reference and size will adjust and 'i' will equal 'totChunks + 1'
					if(chunkArray[i].chunkStart == n)
					{
						chunkArray[i].chunkStart = c;
						chunkArray[i].chunkSize = memPartArray[c].chunkSize;
						i = totChunks;
					}
				}
				if(i == totChunks + 1) return 0;
				else
				{
					//return error
					cout << "Error - 'freeArrays()' chunkArray Error" << endl << "\tError Code: freeArrays(-3)" << endl;
					return -3;
				}
			}
			else if(s == 4)
			{
				//expand [previous] to include both [i] and [next] and subtract 1 from 'totChunks'
					//memPartArray - re: Step 2
				memPartArray[p].chunkSize += (memPartArray[c].chunkSize + memPartArray[n].chunkSize);	//increase previous chunk's size to include that of current and next cells
				int x = memPartArray[n].chunkSize;		//locate next active cell following [n]
				memPartArray[n + x].prevChunk = p;		//set said cell's 'prevChunk' to [p] chunk
				memPartArray[c].Re_memPart();		//reset [c] cell's values to non-value default - cell is no longer a start reference
				memPartArray[n].Re_memPart();		//reset [n] cell's values to non-value default - cell is no longer a start reference
					//chunkArray - re: Step 3
				int i = 0;
				bool foundP = false;		//to check for previous ref adjustment
				for(i = 0; i < totChunks; i++)	//find relevant memory reference in unordered 'chunkArray'
				{
					if(chunkArray[i].chunkStart == p)
					{		//if [p] is found, adjust its size to include that of [c] and [n]
						chunkArray[i].chunkSize = memPartArray[p].chunkSize;
						foundP = true;
					}
					if(chunkArray[i].chunkStart == n)
					{		//if [n] is found, adjust 'totChunks', then replace reference cell's values w/ that of final ref. cell
						totChunks--;
						chunkArray[i].chunkStart = chunkArray[totChunks].chunkStart;
						chunkArray[i].chunkSize = chunkArray[totChunks].chunkSize;
						chunkArray[totChunks].Re_freeChunks();		//reset final active reference cell in array to default values
						i = totChunks;
					}
				}
				if(i == totChunks + 1 && foundP == true) return 0;	//verifies both relevant cells were adjusted
				else
				{
					//return error
					cout << "Error - 'freeArrays()' chunkArray Error" << endl << "\tError Code: freeArrays(-4)" << endl;
					return -4;
				}
			}
			else
			{
				//return error
				cout << "Error - 'freeArrays()' Unknown Situation Error" << endl << "\tError Code: freeArrays(-1)" << endl;
				return -1;
			}
		}
	public:
		memoryManagement()	//constructor
		{
			memBank = calloc(10240, 1024);			//allocate full memory space to be managed
			memPartArray = (memPart*) calloc(10240, sizeof(memPart));
			memPartArray[0].isChunkStart = true;	//initialize first reference cell w/ actual values
			memPartArray[0].isFree = TROO;
			memPartArray[0].chunkSize = 10240;
			chunkArray = (freeChunks*) calloc(10240, sizeof(freeChunks));
			chunkArray[0].chunkSize = 10240;		//initialize first reference cell w/ actual values
			chunkArray[0].chunkStart = 0;
			totChunks = 1;
		}
		~memoryManagement()	//destructor
		{		//release all allocated memory space
			free(memBank);
			free(memPartArray);
			free(chunkArray);
			cout << endl << "\t\t\tMemory Destruction Successful" << endl << endl;
		}
		void * my_malloc(size_t size)
		{
			int blocks = (int) (size / 1024);		//memory size translated to kB
			int chunkArrayOffset = findFree(blocks);	//find best hole for requested size
			if(chunkArrayOffset == -1)
			{
				//set error
				cout << "Error - Insufficient Memory" << endl;
				errno = -3;
				return NULL;
			}
			int myChunkStart = chunkArray[chunkArrayOffset].chunkStart;		//set reference to cell location of hole start
			if(!memPartArray[myChunkStart].isChunkStart)
			{
				//set error
				cout << "Error - isChunkStart Error" << endl;
				errno = -4;
				return NULL;
			}
			if(chunkArray[chunkArrayOffset].chunkSize > blocks)
			{
				//resize hole and allocate claimed piece
				chunkArray[chunkArrayOffset].chunkStart += blocks;	//move chunkStart to new location
				chunkArray[chunkArrayOffset].chunkSize -= blocks;	//set decreased size
				memPartArray[myChunkStart + blocks].isChunkStart = true;	//mark new cell as a start cell
				memPartArray[myChunkStart + blocks].isFree = TROO;			//mark cell as free
				memPartArray[myChunkStart + blocks].chunkSize = memPartArray[myChunkStart].chunkSize - blocks;	//set decreased size
				memPartArray[myChunkStart + blocks].prevChunk = myChunkStart;	//set new start cell's previous cell reference appropriately
			}
			else
			{
				totChunks--;		//if claimed chunk exact size, total count decreases
				if(totChunks != chunkArrayOffset)	//if end value wasn't value just claimed,
				{										//move end value to claimed values location
					chunkArray[chunkArrayOffset].chunkStart = chunkArray[totChunks].chunkStart;
					chunkArray[chunkArrayOffset].chunkSize = chunkArray[totChunks].chunkSize;
				}
				chunkArray[totChunks].Re_freeChunks();	//delete end value
			}
			memPartArray[myChunkStart].isFree = FALLCE;			//set start cell as nonfree
			memPartArray[myChunkStart].chunkSize = blocks;		//set cell's size value to allocated size
			void * retVal;
			retVal = (memBank + myChunkStart);
			return (void*)retVal;	//return pointer to allocated memory start
		}
		void my_free(void * ptr)
		{
			int i = 0;
			while(i < 10240)
			{		//find pointed value in 'chunkArray'
				if(ptr == memBank + i)
				{
					break;
				}
				else
				{
					i += memPartArray[i].chunkSize;
				}
			}
			if(i >= 10240)	//error if not found
			{
				cout << "Error - Cannot Find Memory Chunk to Free" << endl << "\tError Code: my_free(-2)" << endl;
				errno = -2;
				return;
			}
			int previous = memPartArray[i].prevChunk;	//set value for previous start cell
			int next = i + memPartArray[i].chunkSize;	//set value for next start cell
			int ret = 0;	//to be used for internally returned error codes
			if(previous != -1 && next < 10240)		//if target chunk has a chunk on either side
			{
				if(memPartArray[previous].isFree == FALLCE && memPartArray[next].isFree == FALLCE)
				{		//if surrounding chunks are nonfree
					//free [i] and add 1 to 'totChunks'
					ret = freeArrays(1,previous,i,next);
				}
				else if(memPartArray[previous].isFree == TROO && memPartArray[next].isFree == FALLCE)
				{		//if only previous chunk is free
					//expand [previous] to include [i] and leave 'totChunks' the same
					ret = freeArrays(2,previous,i,next);
				}
				else if(memPartArray[previous].isFree == FALLCE && memPartArray[next].isFree == TROO)
				{		//if only next chunk is free
					//expand [next] backwards to include [i] and leave 'totChunks' the same
					ret = freeArrays(3,previous,i,next);
				}
				else if(memPartArray[previous].isFree == TROO && memPartArray[next].isFree == TROO)
				{		//if surrounding chunks are free
					//expand [previous] to include both [i] and [next] and subtract 1 from 'totChunks'
					ret = freeArrays(4,previous,i,next);
				}
				else
				{
					//return error
					cout << "Error - 'my_free()' Three Chunk Logic Error" << endl << "\tError Code: my_free(-3)" << endl;
					errno = 3;
					return;
				}
			}
			else if(previous == -1 && next < 10240)		//special case : if target chunk is at the beginning of memBank
			{
				if(memPartArray[next].isFree == FALLCE)
				{		//if next chunk is nonfree
					//free [i] and add 1 to 'totChunks'
					ret = freeArrays(1,previous,i,next);
				}
				else if(memPartArray[next].isFree == TROO)
				{		//if next chunk is free
					//expand [next] backwards to include [i] and leave 'totChunks' the same
					ret = freeArrays(3,previous,i,next);
				}
				else
				{
					//return error
					cout << "Error - 'my_free()' Front Chunk Logic Error" << endl << "\tError Code: my_free(-4)" << endl;
					errno = 4;
					return;
				}
			}
			else if(previous != -1 && next == 10240)	//special case : if target chunk is at the end of memBank
			{
				if(memPartArray[previous].isFree == FALLCE)
				{		//if previous chunk is nonfree
					//free [i] and add 1 to 'totChunks'
					ret = freeArrays(1,previous,i,next);
				}
				else if(memPartArray[previous].isFree == TROO)
				{		//if previous chunk is free
					//expand [previous] to include [i] and leave 'totChunks' the same
					ret = freeArrays(2,previous,i,next);
				}
				else
				{
					//return error
					cout << "Error - 'my_free()' Back Chunk Logic Error" << endl << "\tError Code: my_free(-5)" << endl;
					errno = 5;
					return;
				}
			}
			else if(previous == -1 && next == 10240)	//special case : if target chunk uses entire memBank
			{
				//free [i] and add 1 to 'totChunks'
				ret = freeArrays(1,previous,i,next);
			}
			else
			{
				//return error
				cout << "Error - 'my_free()' No Matching Situation Error" << endl << "\tError Code: my_free(-6)" << endl;
				errno = 6;
				return;
			}
			if(ret != 0) 
			{		//translate internally returned error codes
				ret -= 10;
				errno = ret;
			}
			return;
		}
};
