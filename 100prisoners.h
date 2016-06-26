#ifndef UNION
#define UNION
#include "union-find/union-find.h"
#endif

/*
 * Simulates the 100 prisoners problem "n" times using the
 * best strategy and prints the statistics.
 *
 * int n is the number of simulations to simulate the 100 prisoners problem
 *
 * char* caller is the name of the function calling simulateAndStats.
 * This is used incase of debugging, to print statistics of all threads
 * or processes
 *
 * The return value is the number of times the simulations succeeded,
 * or the number of times all prisoners found their tag number.
 */
int simulateAndStats(int n, char* caller);

/*
 * Simulates the 100 prisoners problem once using the
 * union find data structure and returns success or failure.
 * success = 1 and failure = 0
 */
enum found_t {
    NOT_FOUND = 0,
    FOUND = 1,
};
enum found_t runSimulation(set_union* s);

/*
 * Simulates the 100 prisoners problem once using a
 * naive approach and returns success or failure.
 * success in this function only occurs if all prisoners find their tag
 */
enum found_t runNaiveSimulation(void);

/*
 * Simulates each prisoner to look for his tag number
 *
 * int prisonerNum is the number of the prisoner looking for his tag number.
 * This prisoner is looking for the number prisonerNum.
 *
 * int boxes[] is the room of uniformly distributed boxes.
 * prisoner #prisonerNum is looking through 50 boxes in boxes[]
 *
 * if prisoner #prisonerNum finds his tag, lookForTag returns 1
 * if the prisoner does not find his tag within 50 trails,
 * lookForTag returns 0
 */
int lookForTag(int prisonerNum, int boxes[]);

/*
 * Prints the statistics of a simulation that ran "n" times.
 * The statistics include the estimated parameter, variance of the parameter,
 * and a 95% confidence interval.
 *
 * int sum is the number of successes that the simulation returned
 *
 * int n is the number of simulations performed
 *
 * char* caller is the name of the thread / process that called printStats
 */
void printStats(int sum, int n, char* caller);

/*
 * Performs a single simulation of the 100 prisoners problem
 * using the union find data structure.
 * set_union* s is a pointer to the set of paths created
 *              from the randomization of the set of boxes.
 *              If a set is larger than 50, that means that
 *              at least 1 prisoner would need to inspect more
 *              than 50 boxes.
 * int size is the number of boxes.
 */
enum found_t single_simulation(set_union* s, int size);

/*
 * Randomizes / shuffles the array using the Fisher-Yates (Knuth) shuffle
 * algorithm.
 * http://en.wikipedia.org/wiki/Fisher–Yates_shuffle
 *
 * Source of algorithm implementation:
 * D. E. Knuth, "Random Numbers", in The Art of Computer Programming, Volume 2:
 * Seminumerical Algorithms, 3rd ed. Boston, Massachusetts: Addison-Wesley Professional,
 * 1997, ch. 3, sec. 4.2, pp. 145
 *
 * int* array is the array to randomize / shuffle
 *
 * int size is the size of the array
 */
void randomizeArray(int* array, int size);

/*
 * Seeds the random() function.
 * Using random() instead of rand() for better randomness.
 */
void seed(void);

/*
 * Simulates 100 prisoners problem "n" times using numProcesses processes.
 * very similar to simulateAndStatsWithThreads, except instead of spawning
 * new threads, new processes are spawned.
 *
 * int n is the total number of simulations to be performed
 *
 * int numProcesses is the number of processes to create and simulate the
 * 100 prisoners problem (n / numProcesses) times.
 *
 * eg. if n == 100, and numProcesses == 4, then each process performes 100/4 = 25 simulations
 */
void simulateAndStatsWithProcesses(int n, int numProcesses);

/*
 * The threads or processes take a parameter to call the
 * splitSimulation function.
 */
struct simParam {
    char* taskName; // name of caller, name could be either Thread or Process
    int taskNum;    // the number or id of each thread or process, eg. Thread 1 / Process 3
    int* successes; // shared array to store number of successes in their respective location.
                    // their respective location is index of their number, their threadOrProcessNum
    int numSimulations; // number of simulations for this thread or process to simulate.
};

/*
 * Specialized simulation function dedicated for threads or processes.
 */
void* splitSimulation(struct simParam* p);

void printUsage(void);
