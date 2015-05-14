# Simulating the 100 prisoners problem

## Problem definition

The 100 prisoners problem is a problem in probability theory. 100 prisoners are told to give their tag number and it will be put in a boxes at random in some room that contains 100 boxes. A box can only contain 1 tag number. Each of the 100 prisoners are allowed to go into this room and open 50 boxes in the room. No prisoner is allowed to communicate with one another. The prisoners are told that if all of them find their tag number by searching 50 boxes or less, then they will be set free, otherwise all of them remain in prison.

The problem is to find the probability that all 100 prisoners find their tag number.
To better understand the problem, one can read the article on wikipedia, or watch a youtube video, \(or two\), explaining the problem.

[http://en.wikipedia.org/wiki/100_prisoners_problem](http://en.wikipedia.org/wiki/100_prisoners_problem)  
Youtube video 1: [https://www.youtube.com/watch?v=eivGlBKlK6M](https://www.youtube.com/watch?v=eivGlBKlK6M)  
Youtube video 2: [https://www.youtube.com/watch?v=C5-I0bAuEUE](https://www.youtube.com/watch?v=C5-I0bAuEUE)

The true probability is about 0.31182782.

## Simulation

In this simulation, the goal is to simply determine the estimated probability that all 100 prisoners can find their tag number \(or marked dollar bills\).

The simulations can be simulated by a specified number of threads or processes.

The simulation code uses the math, threads \(pthead\), and process \(unistd\) libraries. By using these libraries, the problem can be simulated even faster using threads or processes, although using threads for this simulation is not a good idea, as will be explained below.

Each simulation returns either 1 or 0, and so the estimated probability that all prisoners find their tag number is the number of successful simulation \(simulations that returned 1\) divided by the number of simulations performed.

## Running the simulation

### Compiling the code

This simulation can only be performed on Mac OSX or Linux operating systems. To compile on Linux, use clang and link the math and thread libraries:

`clang 100prisoners.c -o 100prisoners -lm -pthread`

On Mac OSX, the above may be done without explicitly linking the libraries.

### Sequential simulation

To simulate the problem without threads or processes, in other words, to
simulate "sequentially", this can be specified as follows:

`100prisoners number-of-times-to-simulate s`

So for example:

`100prisoners 1000 s`

Would simulate the problem 1,000 times, and return the estimated probability that all prisoners found their tag number. The "s" represents "sequential".

### Multi-threaded or Multi-process simulation

To simulate using threads so some threads can perform a smaller number of simulation, so the total number of simulations is the sum of all the simulations performed by each thread, try the following:

`100prisoners 1000 t 4`

This would create 4 threads, and each thread would simulate 1000/4 = 250 simulations, then once each thread is finished simulating, the total number of successful simulations is summed up and divided by 1000.

To perform the same example as above but using processes instead of threads, type the following:

`100prisoners 1000 p 4`

## Statistics

To find the number of simulations to perform in order to obtain the estimated probability that all 100 prisoners succeed at finding their tag number with 95% confidence and with a half width of 10^-4, \(which will give an estimated accuracy of 4 digits\), we can refer to the confidence interval width formula:

w = z*\(s/sqrt\(n\)\)

Where *w* is the half width \(w = 10^-4\), z is the z score accounting for 95% of the normal distribution, \(z = 1.96\), s is the standard deviation, and n is the number of simulations to perform. Since we know that each simulation returns either 0 or 1, we know that the simulation returns a Bernoulli random variable, and the variance of this random variable is p\*\(1-p\), where p is the success probability, in other words, the ideal probability that all prisoners will find their tag number, and so p is equal to about 0.31182782, and 0.31182782\*\(1-0.31182782\) = 0.21459123, so the variance s^2 = 0.21459123.

Now to solve the above equation for n:

n = \(z/w\)^2\*s^2

\(1.96/10^-4\)^2\*0.21459123 = 82437366.9168

In other words, it is required to simulate about 83 million simulations to obtain the estimated probability with a half width of 10^-4 and a 95% confidence. Below are the statistics on Mac OSX and Linux for running 83 million simulations. The multi threaded and multi process simulations run with 4 threads or processes, respectively:


Mac OSX statistics:  
OS X Yosemite \(10.10.3\), Macbook pro  
2.4 GHz Intel Core i5

|                | Probability Estimate | 95% CI               | Time              |
|----------------|----------------------|----------------------|-------------------|
| Sequential     | 0.311876             | {0.311776, 0.311975} | 10 min, 06.17 sec |
| Multi-threaded | 0.309623             | {0.309523, 0.309722} | 5 min, 0.3 sec    |
| Multi-process  | 0.311856             | {0.311756, 0.311955} | 4 min, 20.20 sec  |


Linux statistics:  
Arch Linux \(3.19.2-1-ARCH\), Macbook pro  
2.4 GHz Intel Core i5

|                | Probability Estimate | 95% CI               | Time              |
|----------------|----------------------|----------------------|-------------------|
| Sequential     | 0.311822             | {0.311722, 0.311922} | 10 min, 36.55 sec |
| Multi-threaded | 0.311836             | {0.311736, 0.311935} | 17 min, 35.46 sec |
| Multi-process  | 0.311839             | {0.311740, 0.311939} | 4 min, 46.54 sec  |


The commands used to generate the statistics above were the following:

`time ./100prisoners 83000000 s`  
`time ./100prisoners 83000000 t 4`  
`time ./100prisoners 83000000 p 4`


It is interesting to note that the threaded simulation on the Mac OSX seems to produce an incorrect estimate, the chances that the 95% confidence interval does not contain the actual probability (0.31182782) is very very low. I thought this was a bug, so i tried to find the bug, but did not find anything and the linux result seems to give a good estimate, so i tried putting a mutex around the function random(). This solved the problem on Mac OSX, however it slowed down considerably that it was not worth doing the simulation with a mutex. It took more than 5 min to simulate a 1 million threaded simulation. The number of simulations required based on the constraints above is 83 million, it would take too long.

To conclude, the simulation above is best done with processes instead of threads or sequentially. When threads are used to simulate and is performed properly, there is too much overhead and consequently takes longer than a sequential simulation. Using processes is the fastest and reliable
