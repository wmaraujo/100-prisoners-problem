/*
 * Author:  Wesley Araujo
 * License: Creative Commons Attribution 4.0
 *          http://creativecommons.org/licenses/by/4.0/
 *
 * Simulation of the 100 prisoner problem using the best strategy to
 * estimate the probability that all prisoners succeed.
 *
 * Explanation of the problem can be found on wikipedia or
 * the youtube video links below:
 * http://en.wikipedia.org/wiki/100_prisoners_problem
 *
 * The youtube videos inspired me to do this simulation.
 * "An Impossible Bet"
 * https://www.youtube.com/watch?v=eivGlBKlK6M
 * "Solution to The Impossible Bet"
 * https://www.youtube.com/watch?v=C5-I0bAuEUE
 *
 * True value is about = 0.31182782
 * Obtained with WolframAlpha:
 * http://www.wolframalpha.com/input/?i=1+-+%28HarmonicNumber[100]+-+HarmonicNumber[50]%29
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/wait.h>

#include "100prisoners.h"

#ifdef PRNG
#if PRNG == 2

#include "dSFMT/dSFMT.h"
dsfmt_t dsfmt;

#endif
#endif

#define DEFAULT_NUM_PRISONERS 100
#define MAX_TRIALS 50
#define DEBUG 0


int main(int argc, char* argv[]) {
    if (argc == 3) {
        int inputNumSimulations = atoi(argv[1]);
        if (*argv[2] == 's') { // simulate sequentially
            int sum = simulateAndStats(inputNumSimulations, "Sequence (Single Thread / Process)");
            printStats(sum, inputNumSimulations, "Sequence (Single Thread / Process)");
        }
        else {
            printUsage();
        }
    }
    else if (argc == 4) {
        int inputNumSimulations = atoi(argv[1]);
        if (*argv[2] == 'p') { // simulate with processes
            int numProcesses = atoi(argv[3]);
            simulateAndStatsWithProcesses(inputNumSimulations, numProcesses);
        }
        else if (*argv[2] == 't') { // simulate with threads
            int numThreads = atoi(argv[3]);
            simulateAndStatsWithThreads(inputNumSimulations,  numThreads);
        }
        else {
            printUsage();
        }
    }
    else {
        printUsage();
    }
    return EXIT_SUCCESS;
}

void printUsage(void) {
    puts("Usage:\n"
         "\tsimuBestop numSimulations threadOrProcess numThreadOrProcess\n"
         "\teg. Simulate 1000 with 2 threads\n"
         "\tsimuBestop 1000 t 2\n"
         "\teg. Simulate 1234 with 4 processes\n"
         "\tsimuBestop 1234 p 3\n"
         "\teg. Simulate 1234 sequentially\n"
         "\tsimuBestop 1234 s");
}

int simulateAndStats(int n, char* caller) {
    int sum = 0;

    seed(); // seed to randomize boxes array in simulation
    for (int i=0; i<n; i++) {
        sum += runSimulation(); // simulation performed here
    }
#if DEBUG == 1
    printStats(sum, n, caller);
#endif
    return sum;
}

enum found_t runSimulation(void) {
    const int num = DEFAULT_NUM_PRISONERS;
    int prisoners[num];
    int boxes[num];

    for (int i=0; i<num; i++) {
        prisoners[i] = i;
        boxes[i] = i;
    }

    randomizeArray(boxes, num);

    for (int i=0; i<num; i++) {
        // if one prisoner does not find his tag, then return NOT_FOUND = 0, since
        // not all prisoners found their tag.
        if (lookForTag(prisoners[i], boxes) == NOT_FOUND) {
            return NOT_FOUND;
        }
    }
    // if all prisoners found their tag, then return FOUND = 1
    return FOUND;
}

int lookForTag(int prisonerNum, int boxes[]) {
    int currentNum = prisonerNum;

    // have the prisoner check each box
    for (int trials=0; trials<MAX_TRIALS; trials++) {
        if (prisonerNum == boxes[currentNum]) { // prisoner checks number inside box
            return FOUND;
        }
        else {
            currentNum = boxes[currentNum]; // use number in box to search for next box
        }
    }
    return NOT_FOUND; // exhausted all 50 boxes
}

void printStats(int sum, int n, char* caller) {
    double mean = sum / (n + 0.0);
    // standard variance formula = ( sigmaSum(x^2) * n*mean^2 ) / (n - 1)
    // since sigmaSum(x^2) = sum because each simulation is a Bernoulli random variable,
    // and mean = sum / n, then
    // variance = (sum * (n*sum^2)/n^2) / (n-1) = (sum * sum^2/n) / (n-1) = (sum*(1 - mean))/(n-1)
    double var = (sum*(1 - mean))/(n-1);
    printf("\nStatistics of %s:\n", caller);
    printf("Number of simulations: %d\n", n);
    printf("Parameter Estimate = %f\n", mean);
    printf("Variance is %f\n", var);
    printf("95%% CI: {%f, %f}\n",
           mean - 1.96*sqrt(var/n),
           mean + 1.96*sqrt(var/n));
}

void randomizeArray(int* array, int size) {
    int currentIndex = size - 1;
    int randomIndex;
    int toSwap;

    while (currentIndex > 0) {
        randomIndex = randomInt(currentIndex);

        toSwap = array[randomIndex];
        array[randomIndex] = array[currentIndex];
        array[currentIndex] = toSwap;

        currentIndex--;
    }
}

unsigned int randomInt(int currentIndex) {
#if PRNG == 0 // default c PRNG
    return random() % (currentIndex+1);
#elif PRNG == 1 // MRG32k3a PRNG
    int stuff = 1;
    return stuff++;
#elif PRNG == 2 // dSFMT (successor of mersenne twister)
    return dsfmt_genrand_close_open(&dsfmt) * (currentIndex+1);
#endif
}

void seed(void) {
    FILE* urandom = fopen("/dev/urandom", "r"); // letting program close file on program completion
    if (urandom == NULL) {
        perror("Couldn't open urandom file");
        exit(EXIT_FAILURE);
    }

    unsigned int seedVal;
    if (fread(&seedVal, sizeof(seedVal), 1, urandom) == 0) {
        perror("Couldn't read urandom file");
        exit(EXIT_FAILURE);
    }
    srandom(seedVal);
}

void simulateAndStatsWithThreads(int n, int numThreads) {
    pthread_t threads[numThreads];
    int successes[numThreads]; // array for each thread to store number of successes in simulation
    struct simParam listOfParam[numThreads];

    // create each thread, setup the thread arguments and call the simulation function
    for (int i=0; i<numThreads; i++) {
        listOfParam[i].taskName =       "Thread";
        listOfParam[i].successes =      successes;
        listOfParam[i].taskNum =        i;
        listOfParam[i].numSimulations = n / numThreads;
        pthread_create(&threads[i], NULL,
                       (void* (*)(void*)) &splitSimulation,
                       &listOfParam[i]);
    }
    for (int i=0; i<numThreads; i++) {
        pthread_join(threads[i], NULL); // wait for all threads to finish
    }
    int sum = 0;
    for (int i=0; i<numThreads; i++) {
        sum += successes[i];
    }

    int numSimulation = (n / numThreads) * numThreads; // integer division
    printStats(sum, numSimulation, "All threads");
}

void simulateAndStatsWithProcesses(int n, int numProcesses) {
    int pid, sum = 0;
    // create array that all processes can communicate with
    int* successes = mmap(NULL, sizeof(int)*numProcesses,
                          PROT_WRITE|PROT_READ, MAP_ANON|MAP_SHARED, -1, 0);
    struct simParam listOfParam[numProcesses];

    // let parent fork() multiple times and wait for children to simulate.
    for (int i=0; i<numProcesses; i++) {
        pid = fork();
        if (pid == 0) { // children
            listOfParam[i].taskName =       "Process";
            listOfParam[i].successes =      successes;
            listOfParam[i].taskNum =        i;
            listOfParam[i].numSimulations = n / numProcesses;
            splitSimulation(&listOfParam[i]);
            exit(EXIT_SUCCESS); // children finished simulating
        }
        else if (pid < 0) { // failed
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
    }
    while (wait(NULL) > 0); // let parent wait for all children processes to exit

    for (int i=0; i<numProcesses; i++) {
        sum += successes[i];
    }
    int numSimulation = (n / numProcesses)*numProcesses; // integer division
    printStats(sum, numSimulation, "All processes");
}

void* splitSimulation(struct simParam* p) {
    // specify whether this function is being called by thread or process,
    // specify their taskNum, and number of simulations they will perform
    printf("%s %d, number of simulations to perform: %d\n",
           p->taskName, p->taskNum + 1, p->numSimulations);

    char nameAndNum[20]; // string variable to contain taskNume and taskNum
    int idealBufSize = snprintf(nameAndNum, sizeof(nameAndNum),
                                "%s %d", p->taskName, p->taskNum + 1);
    int sum;
    // if array of 20 char is not enough
    if (idealBufSize > sizeof(nameAndNum)) {
        char secondaryBuf[idealBufSize];
        snprintf(secondaryBuf, sizeof(secondaryBuf),
                 "%s %d", p->taskName, p->taskNum + 1);
        sum = simulateAndStats(p->numSimulations, secondaryBuf);
    }
    else {
        sum = simulateAndStats(p->numSimulations, nameAndNum);
    }
    p->successes[p->taskNum] = sum; // store number of successes in respective location

    return NULL;
}
