# Assignment4-Hybrid-MPI-OMP
<h1>Hybring MPI and OMP Implementation of N body problem</h1>

<h2>Aim of the Assignment</h2>
The aim of this project is to parallelise the N body problem which has been provided in the form of serial code. The serial code is available <a href="https://github.com/luketowell/Assignment4-Hybrid-MPI-OMP/blob/master/nBodyProblem.c">here</a>.

________________
<h2>Assignment Brief</h2>
The brief for this project is available <a href="https://cgi.csc.liv.ac.uk/~mkbane/COMP528/assignments/comp528-assignment4v1.pdf">here</a>.

________________
<h2>Compiling on the Machine</h2>

initial compilation of the Serial Code using the intel compiler
`icc serialCode.c -qopt-report3 -O2`

This produced the compiler output file [serialCode.optrpt](serialCode.optrpt)

From this file I am then able to see that the compiler has made changes various changes to the loops within the main function. these changes include vectorisation to multiple loops including the loop for working out new locations and the loops for initialising the variables in the testInit() function.  

Throughout the output report the compiler has highlighted that its changed should have resulted in the potential speedup of functions. below are the results of comparing the serialCode with no optimisations and using the intel compiler with -O2 optimisation.

<img src="images/compilerTimings.png" width=400px/>

compiling in parallel: 

In order to compile in parallel I used the following command which compiles with the mpi intel compiler and also uses -qopenmp to indicate the use of openMp within the code base. 
`mpiicc -qopenmp -O2 -qopt-report3 nproblem.c -o ./nproblem-parallel.exe`

<h2>Profiling</h2>

<h3>Profiling the code on the machine</h3>

Profiled the code at the beginning of the project using the VTune application to view analyse the code hotspots when running the program. figure 2 shows the breakdown of code usage amongst functions. 

In order to profile the code I ran the following command:
`/opt/software/intel/vtune_amplifier_2018.0.2.525261/bin64/amplxe-cl -collect hotspots -app-working-dir /home/sgltowel/COMP528/Assignment4 -- /home/sgltowel/COMP528/Assignment4/SerialNbody-icc-O0.exe`

<h3>Accessing profiling files from remote servers </h3>

In order to profile the code ran on chadwick remotely on my local machine I have had to save the results from the profiled code run on chadwick and then transfer them from the chadwick login node through to my personal computer. Due to the fact that chadwick sits behind a firewall which is not accessible from outside the university network I have to move the file onto my account on the linux farm and then from the linux farm onto my own laptop. I have made use of 2 scp commands in order to do this.

From Chadwick I enter the following command:

    scp sgltowel@chadwick.liv.ac.uk:/home/sgltowel/COMP528/Assignment4/profile.file sgltowel@lxfarm.csc.liv.ac.uk:/home/sgltowel

- This code will copy the file specified from the Assignment4 folder over to the sgltowel user folder on the linux farm. In order to copy the file it will ask for authentication for both the chadwick machine and the linux farm.

From my host machine I enter the following: 

    scp sgltowel@lxfarm.csc.liv.ac.uk:/home/sgltowel/profile.file /Users/luketowell/desktop

- This code will copy the specified the file from the linux file onto the desktop of my laptop.

Once I have transfered the profile file from chadwick through to my own laptop I am able to use whichever profiling tool i prefer in order to profile the code.

