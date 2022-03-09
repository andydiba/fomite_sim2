# Fomite Simulation

Source code (C) for a stochastic simulation of fomite-mediated disease transmission.

For help/details contact andrew.di.battista@ultraleap.com

This code is available as open-source under Apache 2.0 license. (See LICENSE for details)

Please note the code is used for experimental purposes and is not a refined piece of software! Why was it written in C ? Stochastic simulating are computationally intense; originally developed using Matlab/Octave scripts, precompiled C code runs ~500 times faster!


Building source code (using gcc and make) (Linux/Mac):

1. Extract source code files to a preferred directory.
2. Navigate to the source directory in a terminal
3. Run the makefile using 'make'


Running the code:

The executable 'sim' requires arguments {config file, output_filename, #of reps, logflg, key,value }
Often you will want to run the sim several times, while changing parameter values to see what happens.
To facilitate this, see 'sim_script.sh' which can be run from a terminal (without arguments) as ./sim_script.sh
(a default simulation scenario is already set)


CSV output files:

The output csv file from sim_script.sh shows the risk (cumulative incidence rate CI) for each
pool of individuals put into the simulation. (minor) bug alert: if the KEY/VAL requires multiple
fields (i.e. csv columns),there will be a column misalignment (labels should be right-justified).
It works fine when key/val occupy a single column each.

A sample output file from the current sim_script.sh is included (and contains this bug) for illustration.

Config files:

There is a simple example configuration file (example1.conf) as well as several used in then
associated journal article for this research. Explanatory comments are included in each file.

example1.conf  - 3 screens (1 contaminated) used over and over again in sequential loop (shows secondary+  transfer)
3z_risk - another example showing how to implement multiple 'zones'

basic.conf   
- single touchscreen used over and over again -> results in dynamic equilibrium of bioburden levels
- try with/without pathogen removal (die off/on  and face/env touch)

2z_risk (used predominantly in associated research paper) - 2 screens (1 contaminated)
