#include <iostream>
#include <random>
#include <cstdio>
#include <ctime>
#include <unistd.h>

using namespace std;

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

int simulateProcesses() {

	bool flagRunSimulation = true; 	// the variable is false when the simulation has finished
	int currentCycle = 0; 			// the cycle that is now simulated
	int cyclesUntilProcFinish = 0; 	// the number of cycles remaining until the current executing process finishes computing
	int availableMemory = 10240; 	// the amount of available memory remaining in KB; at first, we have 10Mb = 10 * 1024 KB = 10240KB
	int indexOfCurrentlyExecutingProcess = 0; 
	void *allocatedMemory = NULL;
	clock_t processExecutionStart, processExecutionEnd; // used to measure how much time a process spends.

	clock_t start = clock();
	cyclesUntilProcFinish = pArray[0].cpuCycles;
	if (pArray[0].memFootprint <= availableMemory) { // check to see that we have enough memory,
		
		allocatedMemory = malloc(pArray[0].memFootprint * 1024); // beacuse memFootPrint represents the amount of memory in KB and 1KB = 1024B
		if (allocatedMemory == NULL) {
			cout << "Error allocating memory for the process with pid " << pArray[0].processId << endl;
			return -2; // return error code -2
		}
		availableMemory -= pArray[0].memFootprint;
		indexOfCurrentlyExecutingProcess = 0;
		processExecutionStart = clock(); // the first process has started executing
	}
	else {
		cout << "Error allocating memory for the first process with pid " << pArray[0].processId << endl;
		return -1; // return error code -1(not enough memory for the first process)
	}

	while (flagRunSimulation == true) {
		usleep(1000); // sleep 1 milisecond per cycle
		if (currentCycle % 50 == 0 && ((currentCycle / 50) < numProcessesRequired)) {
			int numberOfArrivingProcess = currentCycle / 50; // the number of the process that has arrived
			cout << "PID " << pArray[numberOfArrivingProcess].processId << " arrived at cycle " << currentCycle << ". It has a memory footprint of " <<
				pArray[numberOfArrivingProcess].memFootprint << " and it requires " << pArray[numberOfArrivingProcess].cpuCycles << " cycles to execute" << endl;
			cout << "Currently executing PID " << pArray[indexOfCurrentlyExecutingProcess].processId << endl;
		}
		++currentCycle;
		--cyclesUntilProcFinish;
		if (cyclesUntilProcFinish == 0) {
			free(allocatedMemory);
			allocatedMemory = NULL;
			availableMemory += pArray[indexOfCurrentlyExecutingProcess].memFootprint; // retrieve memory
			processExecutionEnd = clock();
			processExecutionEnd += pArray[indexOfCurrentlyExecutingProcess].cpuCycles * (CLOCKS_PER_SEC / 1000.0);// for the sleeping
			double processExecTime = (double)(processExecutionEnd - processExecutionStart) / CLOCKS_PER_SEC;

			cout << ">> PID " << pArray[indexOfCurrentlyExecutingProcess].processId << " finished executing in cycle " << currentCycle << ". Execution time: " << processExecTime << " seconds.\n\n";

			if (indexOfCurrentlyExecutingProcess == (numProcessesRequired - 1)) {
				clock_t end = clock();
				end += currentCycle* (CLOCKS_PER_SEC / 1000.0);// for the sleeping
				double systemTime = (double)(end - start) / CLOCKS_PER_SEC;

				cout << "Total system time: " << systemTime << " seconds" << endl;
				cout << "Number of processor cycles required to compute all the processes: " << currentCycle << endl;
				flagRunSimulation = false;
			}
			else { // the currently executing project has finished, and the next process from the waiting queue must start
				indexOfCurrentlyExecutingProcess++;
				cyclesUntilProcFinish = pArray[indexOfCurrentlyExecutingProcess].cpuCycles;
				if (pArray[indexOfCurrentlyExecutingProcess].memFootprint <= availableMemory) { // check to see that we have enough memory,
					allocatedMemory = malloc(pArray[indexOfCurrentlyExecutingProcess].memFootprint * 1024); // beacuse memFootPrint represents the amount of memory in KB and 1KB = 1024B
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
				cout << "Starting execution of PID " << pArray[indexOfCurrentlyExecutingProcess].processId << " in cycle " << currentCycle + 1 << endl;
				processExecutionStart = clock();
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