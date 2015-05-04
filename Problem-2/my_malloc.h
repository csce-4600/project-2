//my_malloc.h - defined 'my_malloc()' and 'my_free()' for Project #2, Question #2

/*-----------------------------------------------------------------------------------+
|                                                                                    |
|	This version will simplify the memPartition struct greatly.                      |
|                                                                                    |
+-----------------------------------------------------------------------------------*/

/*
struct memPartition
{
	bool isChunkStart;
	bool isChunkEnd;
	bool isFree;
	void * memStart;
	void * memEnd;
	memPartition * chunkStart;
	memPartition * chunkEnd;
	memPartition * nextChunk;
	memPartition * prevChunk;
	int chunkSize;
	memPartition()
	{
		//
		isChunkStart = false;
		isChunkEnd = false;
		isFree = true;
		memStart = NULL;
		memEnd = NULL;
		chunkStart = NULL;
		chunkEnd = NULL;
		nextChunk = NULL;
		prevChunk = NULL;
		chunkSize = 0;
	}
};
*/

//	struct usedMemPartition
//	{
//		void * start;
//		void * end;
//		int usedSpace;
//	};

#include <iostream>
#include <random>
#include <cstdio>
#include <ctime>
#include <unistd.h>
#include <cerrno>

using namespace std;

enum bule {TROO, FALLCE, NOLE};	//special bool - allows true/false/null (or troo/fallce/nole respectively)

struct memPart
{
	bool isChunkStart;
	bule isFree;
	int prevChunk;
	int chunkSize;
	void Re_memPart()		//all non-ChunkStart partitions initialize "isChunkStart" to false, and all other variables to non-values
	{
		isChunkStart = false;
		isFree = NOLE;
		prevChunk = -1;
		chunkSize = -1;
	}
	memPart()
	{
		Re_memPart();
	}
};

/*
struct partBySize
{
	//structure (for array) to enable best fit
	int numberOfChunks;
	int firstChunk;
	partBySize()
	{
		numberOfChunks = 0;
		firstChunk = -1;	//non-value for no chunk
	}
};
*/

struct freeChunks
{
	int chunkSize;
	int chunkStart;
	void Re_freeChunks()
	{
		chunkSize = -1;
		chunkStart = -1;
	}
	freeChunks()
	{
		Re_freeChunks();
	}
};

class memoryManagement {
	private:
		int totChunks;
		void * memBank;
		//bool * inUse;
		memPart * memPartArray;
		//partBySize * pbsArray;
		freeChunks * chunkArray;
		freeChunks workingChunk;
		int findFree(int blocks)
		{
			//finds free chunk of size 'blocks' and returns its index. returns -1 if no large enough free chunk found.
			workingChunk.Re_freeChunks();
			//cout << "chunkSize : " << workingChunk.chunkSize << " | chunkStart : " << workingChunk.chunkStart << endl;
			//cout << "0-out" << endl;
			for(int i = 0; i < totChunks; i++)
			{
				//
				//cout << "1-out" << endl;
				if(chunkArray[i].chunkSize >= blocks)
				{
					//cout << "2-out" << endl;
					if(workingChunk.chunkSize == -1 || workingChunk.chunkSize > chunkArray[i].chunkSize)
					{
						//
						//cout << "3-out" << endl;
						workingChunk.chunkSize = chunkArray[i].chunkSize;
						workingChunk.chunkStart = i;
						//cout << "@@@chunkSize : " << workingChunk.chunkSize << " | chunkStart : " << workingChunk.chunkStart << endl;
					}
				}
			}
			return workingChunk.chunkStart;
				/*
				for(int i = blocks-1; i < 10240; i++)
				{
					//
					pbsArray[i]
				}
				*//*
				int i = 0;
				while(i < 10240)
				{
					//
					if(!memPartArray[i].isChunkStart) return -2;
					if(memPartArray[i].isFree == TROO && memPartArray[i].chunkSize)
				}
				*/
		}
		int freeArrays(int s, int p, int c, int n)	//s: situation, p: previous, c: i (or current), n: next
		{
			//
			if(s == 1)
			{
				//free [i] and add 1 to 'totChunks'
					//memPartArray - re: Step 2
				memPartArray[c].isFree = TROO;
					//chunkArray - re: Step 3
				chunkArray[totChunks].chunkStart = c;
				chunkArray[totChunks].chunkSize = memPartArray[c].chunkSize;
				totChunks++;
				return 0;
			}
			else if(s == 2)
			{
				//expand [previous] to include [i] and leave 'totChunks' the same
					//memPartArray - re: Step 2
				memPartArray[p].chunkSize += memPartArray[c].chunkSize;
				memPartArray[n].prevChunk = memPartArray[c].prevChunk;
				memPartArray[c].Re_memPart();
					//chunkArray - re: Step 3
				int i = 0;
				for(i = 0; i < totChunks; i++)
				{
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
				memPartArray[c].chunkSize += memPartArray[n].chunkSize;
				memPartArray[n].Re_memPart();
					//chunkArray - re: Step 3
				int i = 0;
				for(i = 0; i < totChunks; i++)
				{
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
				memPartArray[p].chunkSize += (memPartArray[c].chunkSize + memPartArray[n].chunkSize);
				int x = memPartArray[n].chunkSize;
				memPartArray[n + x].prevChunk = p;
				memPartArray[c].Re_memPart();
				memPartArray[n].Re_memPart();
					//chunkArray - re: Step 3
				int i = 0;
				for(i = 0; i < totChunks; i++)
				{
					if(chunkArray[i].chunkStart == p)
					{
						chunkArray[i].chunkSize = memPartArray[p].chunkSize;
					}
					if(chunkArray[i].chunkStart == n)
					{
						totChunks--;
						chunkArray[i].chunkStart = chunkArray[totChunks].chunkStart;
						chunkArray[i].chunkSize = chunkArray[totChunks].chunkSize;
						chunkArray[totChunks].Re_freeChunks();
						i = totChunks;
					}
				}
				if(i == totChunks + 1) return 0;
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
			//list<freeMemPartition>::iterator freeIt;
			//usedMemPartition * usedArray;
			//list<usedMemPartition>::iterator usedIt;
	public:
		memoryManagement()
		{
			memBank = calloc(10240, 1024);
			//inUse = (bool*) calloc(10240, sizeof(bool));	//process mem req's are in terms of kB
			memPartArray = (memPart*) calloc(10240, sizeof(memPart));
			memPartArray[0].isChunkStart = true;
			memPartArray[0].isFree = TROO;
			memPartArray[0].chunkSize = 10240;
			chunkArray = (freeChunks*) calloc(10240, sizeof(freeChunks));
			chunkArray[0].chunkSize = 10240;
			chunkArray[0].chunkStart = 0;
			totChunks = 1;
			//pbsArray = calloc(10240, sizeof(partBySize));
			//pbsArray[10239].numberOfChunks = 1;				//NOTE: pbsArray MUST be accesses by "needed blocks - 1" due to "start on 0"
			//pbsArray[10239].firstChunk = 0;
				/*
				freeMemPartition fmp;
				fmp.start = &(memBank[0]);
				fmp.end = &(memBank[10239]);
				fmp.freeSpace = 10240;
				freeList.push_back(fmp);
				freeIt = freeList.begin();
				usedIt = usedList.begin();
				*/
		}
		void * my_malloc(size_t size)
		{
			int blocks = (int) (size / 1024);
			int chunkArrayOffset = findFree(blocks);
			if(chunkArrayOffset == -1)
			{
				cout << "Error - Insufficient Memory" << endl;		//possibly find better way to handle later
				errno = -3;
				return NULL;
			}
			int myChunkStart = chunkArray[chunkArrayOffset].chunkStart;
			if(!memPartArray[myChunkStart].isChunkStart)
			{
				cout << "Error - isChunkStart Error" << endl;
				errno = -4;
				return NULL;
			}
			if(chunkArray[chunkArrayOffset].chunkSize > blocks)
			{
				//resize chunk and claim needed piece
				chunkArray[chunkArrayOffset].chunkStart += blocks;	//move chunkStart to new location
				chunkArray[chunkArrayOffset].chunkSize -= blocks;
				memPartArray[myChunkStart + blocks].isChunkStart = true;
				memPartArray[myChunkStart + blocks].isFree = TROO;
				memPartArray[myChunkStart + blocks].chunkSize = memPartArray[myChunkStart].chunkSize - blocks;
				memPartArray[myChunkStart + blocks].prevChunk = myChunkStart;
			}
			else
			{
				totChunks--;		//if claimed chunk exact size, total count decreases
				//if(totChunks == chunkArrayOffset)
				//{
					//delete value
				//}
				//else
				if(totChunks != chunkArrayOffset)	//if end value wasn't value just claimed,
				{									//move end value to claimed values location
					chunkArray[chunkArrayOffset].chunkStart = chunkArray[totChunks].chunkStart;
					chunkArray[chunkArrayOffset].chunkSize = chunkArray[totChunks].chunkSize;
					//chunkArray[totChunks]
				}
				chunkArray[totChunks].Re_freeChunks();	//delete end value
			}
				//memPartArray[myChunkStart + blocks].isChunkStart = true;
				//memPartArray[myChunkStart + blocks].isFree = TROO;
				//memPartArray[myChunkStart + blocks].chunkSize = memPartArray[myChunkStart].chunkSize - blocks;
			memPartArray[myChunkStart].isFree = FALLCE;
			memPartArray[myChunkStart].chunkSize = blocks;
			void * retVal;// = (*reinterpret_cast<void* (*)()>(&(memBank[myChunkStart])));
			retVal = (memBank + myChunkStart);
			return (void*)retVal;
			//freeIt = freeList.begin();
			//while(freeIt->freeSpace < blocks && freeIt != freeList.end()) freeIt++;
				/*
				for(freeIt = freeList.begin(); freeIt != freeList.end(); freeIt++)
				{
					//
					if(freeIt->freeSpace >= blocks)
					{
						//
						void * ptr = freeIt->start;
						usedMemPartition ump;
						ump.start = freeIt->start;
						ump.end = &(freeIt->start[blocks]);		//THIS IS THE LINE I'M UNSURE ABOUT SYNTACTICALLY
						if(freeIt->freeSpace > blocks)
						{
							freeMemPartition fmp;
							fmp.start = ump.end[1];
							fmp.end = freeIt->end;
							fmp.freeSpace = freeIt->freeSpace - blocks;
						}
					}
				}
				*/
		}
		void my_free(void * ptr)
		{
			//
			int i = 0;
			while(i < 10240)
			{
				if(ptr == memBank + i)
				{
					//cout << "IT FREAKIN WORKED SEAN!!!!!" << endl;
					break;
				}
				else
				{
					//
					i += memPartArray[i].chunkSize;
				}
			}
			if(i >= 10240)
			{
				cout << "Error - Cannot Find Memory Chunk to Free" << endl << "\tError Code: my_free(-2)" << endl;
				errno = -2;
				return;
			}
			//Memory chunk found - must reset to 0
				//ref: step 1)
			//for(int j = i; j < (i + memPartArray[i].chunkSize); j++) memBank[j] = 0;
					//NOTE TO SELF: IMPLEMENT LATER!!! (currently irrelevant since process do nothing)
			//Check surrounding chunks for freedom to determine situation
				//ref: step 2)
			int previous = memPartArray[i].prevChunk;
			int next = i + memPartArray[i].chunkSize;
			int ret = 0;
			if(previous != -1 && next < 10240)		//if target chunk has a chunk on either side
			{
				if(memPartArray[previous].isFree == FALLCE && memPartArray[next].isFree == FALLCE)
				{
					//free [i] and add 1 to 'totChunks'
					ret = freeArrays(1,previous,i,next);
				}
				else if(memPartArray[previous].isFree == TROO && memPartArray[next].isFree == FALLCE)
				{
					//expand [previous] to include [i] and leave 'totChunks' the same
					ret = freeArrays(2,previous,i,next);
				}
				else if(memPartArray[previous].isFree == FALLCE && memPartArray[next].isFree == TROO)
				{
					//expand [next] backwards to include [i] and leave 'totChunks' the same
					ret = freeArrays(3,previous,i,next);
				}
				else if(memPartArray[previous].isFree == TROO && memPartArray[next].isFree == TROO)
				{
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
			else if(previous == -1 && next < 10240)		//if target chunk is at the beginning of memBank
			{
				if(memPartArray[next].isFree == FALLCE)
				{
					//free [i] and add 1 to 'totChunks'
					ret = freeArrays(1,previous,i,next);
				}
				else if(memPartArray[next].isFree == TROO)
				{
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
			else if(previous != -1 && next == 10240)	//if target chunk is at the end of memBank
			{
				if(memPartArray[previous].isFree == FALLCE)
				{
					//free [i] and add 1 to 'totChunks'
					ret = freeArrays(1,previous,i,next);
				}
				else if(memPartArray[previous].isFree == TROO)
				{
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
			else if(previous == -1 && next == 10240)	//if target chunk uses entire memBank
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
			if(ret != 0) ret -= 10;
			errno = ret;
			return;
		}
};
