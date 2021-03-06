/*	these are included from "my_malloc.h"
#include <iostream>
#include <random>
#include <cstdio>
#include <ctime>
#include <unistd.h>
#include <chrono>
*/
#include "my_malloc.h"

using namespace std;

//Custom Memory Allocator for Problem 2
memoryManagement mem;

// Random value generator; uses normal distribution for random values
class Generator {
	default_random_engine generator;
	normal_distribution<double> distribution;
	double min;
	double max;
public:
	Generator(double mean, double stddev, double min, double max) :
		distribution(mean, stddev), min(min), max(max)
	{}

	double operator ()() {
		while (true) {
			double number = this->distribution(generator);
			if (number >= this->min && number <= this->max)
				return number;
		}
	}
};

struct process {
	int processId;
	int cpuCycles;
	int memFootprint;
};

// Start pid from 
int init_pid = 0;
// Number of processes required
int numProcessesRequired;
// Where all processes will be stored
process* pArray;

// Random value generator
Generator randCycle(6000.0, 2000.0, 1000.0, 11000.0);
Generator randMemFootprint(100.0, 50.0, 1.0, 200.0);

int getProcessID() {
	return init_pid++;
}

void printAllProcesses(process* p) {

	for (int i = 0; i < numProcessesRequired; i++)
		cout << "PID: " << p[i].processId << "\t\t CPU cyles: " << p[i].cpuCycles << "\t\t Memory footprint:" << p[i].memFootprint << endl;

}

// Set values for each individual process
process createProcess() {

	process p;

	p.processId = getProcessID();
	p.cpuCycles = randCycle();
	p.memFootprint = randMemFootprint();

	return p;

}

void createAllProcesses() {

	// Loops until all processes are initialized with mem and cycle values
	for (int i = 0; i < numProcessesRequired; i++)
		pArray[i] = createProcess();

}
void sleepNanoSeconds(int sleepTime)
{
	auto sleepStart = chrono::high_resolution_clock::now();
	int sleepedNanoSeconds = 0;
	do
	{
		sleepedNanoSeconds = chrono::duration_cast<chrono::nanoseconds>(chrono::high_resolution_clock::now() - sleepStart).count();
	}
	while(sleepedNanoSeconds < sleepTime);
}

int simulateProcesses() {

	bool flagRunSimulation = true; 	// the variable is false when the simulation has finished
	int currentCycle = 0; 			// the cycle that is now simulated
	int cyclesUntilProcFinish = 0; 	// the number of cycles remaining until the current executing process finishes computing
	int availableMemory = 10240; 	// the amount of available memory remaining in KB; at first, we have 10Mb = 10 * 1024 KB = 10240KB
	int indexOfCurrentlyExecutingProcess = 0; 
	void *allocatedMemory = NULL;
	auto memAllocStart = chrono::high_resolution_clock::now();
	auto totalTimeStart = chrono::high_resolution_clock::now();

	cyclesUntilProcFinish = pArray[0].cpuCycles;
	if (pArray[0].memFootprint <= availableMemory) { // check to see that we have enough memory,
		// Using high resolution clock as Joseph mentioned
		allocatedMemory = mem.my_malloc(pArray[0].memFootprint * 1024); // beacuse memFootPrint represents the amount of memory in KB and 1KB = 1024B
		if (allocatedMemory == NULL) {
			cout << "Error allocating memory for the process with pid " << pArray[0].processId << endl;
			return -2; // return error code -2
		}
		availableMemory -= pArray[0].memFootprint;
		indexOfCurrentlyExecutingProcess = 0;
	}
	else {
		cout << "Error allocating memory for the first process with pid " << pArray[0].processId << endl;
		return -1; // return error code -1(not enough memory for the first process)
	}

	while (flagRunSimulation == true) {

		if (currentCycle % 50 == 0 && ((currentCycle / 50) < numProcessesRequired)) {
			int numberOfArrivingProcess = currentCycle / 50; // the number of the process that has arrived
			cout << "PID " << pArray[numberOfArrivingProcess].processId << " arrived at cycle " << currentCycle << ". It has a memory footprint of " <<
				pArray[numberOfArrivingProcess].memFootprint << " and it requires " << pArray[numberOfArrivingProcess].cpuCycles << " cycles to execute" << endl;
			cout << "Currently executing PID " << pArray[indexOfCurrentlyExecutingProcess].processId << endl;
		}

		++currentCycle;
		--cyclesUntilProcFinish;

		if (cyclesUntilProcFinish == 0) {

			mem.my_free(allocatedMemory);
			auto memAllocEnd = chrono::high_resolution_clock::now();
			// 1000000 = milisecond
			int sleepTime = 1000000 * pArray[indexOfCurrentlyExecutingProcess].cpuCycles;
			//usleep(sleepTime);
			sleepNanoSeconds(sleepTime);

			allocatedMemory = NULL;
			availableMemory += pArray[indexOfCurrentlyExecutingProcess].memFootprint; // retrieve memory

			cout << ">> PID " << pArray[indexOfCurrentlyExecutingProcess].processId << " finished executing in cycle " << currentCycle << ". Execution time for malloc and free: " << chrono::duration_cast<chrono::nanoseconds>(memAllocEnd - memAllocStart).count()  << " nanoseconds\n\n";

			if (indexOfCurrentlyExecutingProcess == (numProcessesRequired - 1)) {
				auto totalTimeEnd = chrono::high_resolution_clock::now();
				cout << "Total program execution time: " << (chrono::duration_cast<chrono::nanoseconds>(totalTimeEnd - totalTimeStart).count()) << " nanoseconds" << endl;
				cout << "Number of processor cycles required to compute all the processes: " << currentCycle << endl;
				flagRunSimulation = false;
			}

			else { // the currently executing project has finished, and the next process from the waiting queue must start
				indexOfCurrentlyExecutingProcess++;
				cyclesUntilProcFinish = pArray[indexOfCurrentlyExecutingProcess].cpuCycles;
				if (pArray[indexOfCurrentlyExecutingProcess].memFootprint <= availableMemory) { // check to see that we have enough memory,
					memAllocStart = chrono::high_resolution_clock::now(); // timing needs to be restarted for new process
					allocatedMemory = mem.my_malloc(pArray[indexOfCurrentlyExecutingProcess].memFootprint * 1024); // beacuse memFootPrint represents the amount of memory in KB and 1KB = 1024B
					if (allocatedMemory == NULL) {
						cout << "Error allocating memory for the process with pid " << pArray[indexOfCurrentlyExecutingProcess].processId << endl;
						return -2; // return error code -2
					}
					availableMemory -= pArray[indexOfCurrentlyExecutingProcess].memFootprint;
				}
				else {
					cout << "Error allocating memory for the process with pid " << pArray[indexOfCurrentlyExecutingProcess].processId << endl;
					return -1; // return error code -1(not enough memory for the first process)
				}
				cout << "Starting execution of PID " << pArray[indexOfCurrentlyExecutingProcess].processId << " in cycle " << currentCycle + 1 << " with a memory footprint of " << pArray[indexOfCurrentlyExecutingProcess].memFootprint << endl;
			}
		}

	}

	return 0; // succesful function execution

}

int main(int argc, char **argv) {

	// Error handling 
	if (argc < 2) {
		cout << "Error!" << "\nUsage: " << argv[0] << " [REQUIRED: number of processes] [OPTIONAL: start pid from]\n" << endl;
		return -1;
	}

	if (argv[2]) init_pid = atoi(argv[2]);

	numProcessesRequired = atoi(argv[1]);

	// Create process array with number of process required
	pArray = new process[numProcessesRequired];

	// Initialize processes with random mem and cycle requirements
	createAllProcesses();
	
	// Simulate the memory allocation
	simulateProcesses();

	// Cleanup
	delete pArray;

	return 0;

}
