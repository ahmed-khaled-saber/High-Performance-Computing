# High-Performance-Computing
High-Performance-Computing FCIS-ASU Project

# Problem Definition:-
Given an Image, The Apllication should apply Low Pas Convolutional Operation using All Processing Unit Cores since Parallelization is the main Evaluation point.

# Core Idea:-
- Used Master Core Processor Unit to initiate Local Variables,, specifies the Problem Parameters and Broadcasting necessary Data to all cores, then it combines by gathering Solutions sent back. 
- Massage Passing Model/Protocol is used to implement Distributed Parallel Architecture
- Decomposing Image into Independent Components that can precessed in Isolation, rows of The Image divided by NO. of core processing Unit was the main Idea to Enable Parallelism.

# Testing:-
