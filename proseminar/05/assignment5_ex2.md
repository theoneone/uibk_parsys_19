#Assignment 5

##Exercise 2

### Description

N-body simulations form a large class of scientific applications, as they are used in research ranging from astrophysics to molecular dynamics. At their core, they model and simulate the interaction of moving particles in physical space. For this assignment, the specific n-body setting relates to astrophysics, where the mutual graviational effect of stars is investigated. Each particle has several properties which include at least
- position,
- velocity, and
- mass.

For each timestep (you can assume `dt = 1`), particles must be moved by first computing the force excerted on them according to the [Newtonian equation for gravity](https://en.wikipedia.org/wiki/Newton%27s_law_of_universal_gravitation), `force = G * (mass_1 * mass_2) / radius^2` where `G` is the gravitational constant (and can be assumed as `G = 1` for simplicity). Second, using the computed force on a particle, its position and velocity can be updated via `velocity = velocity + force / mass` and `position = position + velocity`.

### Tasks
- Study the nature of the problem in Exercise 1, focusing on its characteristics with regard to optimization and parallelization.
- What optimization methods can you come up with in order to improve the performance of Exercise 1?
- What parallelization strategies would you consider for Exercise 1 and why?

The N-body simulation forms a problem, where each element "body" depends to each other element. Thus, a simple implementation contains two nested loops in the form

```
for i = 0 to n
    for j = 0 to n
        <process calculations>
    done
done
```
... which executes with O(n²).

The simulation calculates gravity forces. So, the superposition principle for Newtonian forces _**f** = **f1** + **f2** + ... + **fn**_ applies, where **f** and **f_** are vectors. These forces can be computed step-wise and in random order as follows:


