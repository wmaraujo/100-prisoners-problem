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

#ifndef UNION
#define UNION
#include "union-find/union-find.h"
#endif

#ifdef PRNG

#if PRNG == 1
#include "MRG32k3a/MRG32k3a.h"
#endif

#if PRNG == 2
#include "dSFMT/dSFMT.h"
dsfmt_t dsfmt;
#endif

#if PRNG == 3
#include "Lfib4/Lfib4.h"
#endif

#endif

#define DEFAULT_NUM_PRISONERS 100
#define MAX_TRIALS 50
#define MAX_uint32 ((1UL << (sizeof(unsigned int)*8)) - 1)
#define DEBUG 0


/* ignore enums for now
enum PRNG_enum {
    c_random,
    MRG32k3a,
    dSFMT
};
*/

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
         "\tsimuBestop numSimulations processOrNot numProcess\n"
         "\teg. Simulate 1234 with 4 processes\n"
         "\tsimuBestop 1234 p 4\n"
         "\teg. Simulate 1234 sequentially (1 process)\n"
         "\tsimuBestop 1234 s");
}

int simulateAndStats(int n, char* caller) {
    int sum = 0;

    seed(); // seed to randomize boxes array in simulation
    set_union s;
    for (int i=0; i<n; i++) {
        sum += runSimulation(&s); // simulation performed here
    }
#if DEBUG == 1
    printStats(sum, n, caller);
#endif
    return sum;
}

enum found_t runSimulation(set_union* s) {
    return single_simulation(s, DEFAULT_NUM_PRISONERS);
}

enum found_t runNaiveSimulation(void) {
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

enum found_t single_simulation(set_union* s, int size) {
    int currentIndex = size - 1;
    int randomIndex;

    set_union_init(s, DEFAULT_NUM_PRISONERS);
    while (currentIndex > 0) {
        randomIndex = randomInt(currentIndex);

        union_set(s, currentIndex, randomIndex);
        if (s->size[find(s, currentIndex)] > MAX_TRIALS) {
            return NOT_FOUND;
        }

        currentIndex--;
    }
    return FOUND;
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
    return MRG32k3a() * (currentIndex+1);
#elif PRNG == 2 // dSFMT (successor of mersenne twister)
    return dsfmt_genrand_close_open(&dsfmt) * (currentIndex+1);
#elif PRNG == 3 // Marsa Lfib4 PRNG
    // to removing inherit bias of modulus, uncomment below,
    // although there isn't much point since one needs to
    // be performing simulations to obtain 8 digits of
    // precision or more... (100/((2^32) - 1) ~= 10^-8)

    /*unsigned int randVal = Lfib4();
    unsigned int modOfMax = MAX_uint32 % (currentIndex+1);
    while (randVal >= MAX_uint32 - modOfMax) randVal = Lfib4();

    return randVal % (currentIndex+1);*/
    return Lfib4() % (currentIndex+1); // comment this out if above is uncommented
#endif
}

void seed(void) {
    FILE* urandom = fopen("/dev/urandom", "r");
    if (urandom == NULL) {
        perror("Couldn't open urandom file");
        exit(EXIT_FAILURE);
    }

    unsigned int seedVal;
    if (fread(&seedVal, sizeof(seedVal), 1, urandom) == 0) {
        perror("Couldn't read urandom file");
        exit(EXIT_FAILURE);
    }
#if PRNG == 0
    srandom(seedVal);
#elif PRNG == 1
    unsigned int seeds[6];
    seeds[0] = seedVal; // store first rand value at 0

    // store remaining 5 rand values in seeds[1] to seeds[5]
    if (fread(seeds + 1, sizeof(unsigned int), 5, urandom) == 0) {
        perror("Couldn't read urandom file for MRG");
        exit(EXIT_FAILURE);
    }
    mrg_seed_array(seeds);
#elif PRNG == 2
    dsfmt_init_gen_rand(&dsfmt, seedVal);
#elif PRNG == 3
    unsigned int seeds[1 << 8];
    if (fread(seeds, sizeof(unsigned int), 1 << 8, urandom) == 0) {
        perror("Couldn't read urandom file for Lfib4");
        exit(EXIT_FAILURE);
    }
    Lfib4_seed((unsigned char)seedVal, seeds);
#endif

    fclose(urandom);
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
