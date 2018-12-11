#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

#define BODIES 5000
#define TIMESTEPS 100
#define GRAVCONST 0.0000001

// global vars
float mass[BODIES];
float vx[BODIES], vy[BODIES];
float x[BODIES], y[BODIES];
float dx, dy, d, F, ax, ay;

void testInit();
void testInit2();
void randomInit();
void outputBody(int);

int main(int argc, char** argv) {
  int time, i, j;
  int mpiId, numMPI;
  int ompId, numOMP;
  double timings[10];
  int N = BODIES*TIMESTEPS;

  //init MPI
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpiId);
  MPI_Comm_size(MPI_COMM_WORLD, &numMPI);

  #pragma omp parallel default(none) shared(numOMP)
  {
     numOMP = omp_get_num_threads();
     printf("%d \n", numOMP);
  }
  
  if (N < numMPI*numOMP) {
    if (mpiId ==0 ) printf("too trivial\n");    // we do not cater for idle cores
  }
  else if (N%(numMPI*numOMP) != 0) {
    if (mpiId ==0 ) printf("too complex\n");    // we only have MPI_Scatter
  }
  else {
    if (mpiId == 0){
    // testInit2();
    timings[0] = omp_get_wtime();
    randomInit();
    //this needs to be parallelized with MPI
    //Scatter the variables to the nodes
    }
    for (time=0; time<TIMESTEPS; time++) {
      printf("Timestep %d\n",time);
      timings[2]= omp_get_wtime();
      // can this be parallelised by openMP?
      for (i=0; i<BODIES; i++) {
        // calc forces on body i due to bodies (j != i)
        for (j=0; j<BODIES; j++) {
            if (j != i) {
              dx = x[j] - x[i];
              dy = y[j] - y[i];
              d = sqrt(dx*dx + dy*dy); 
              if (d<0.01) {
                printf("too close - resetting\n");
                d=1.0;
              }
              F = GRAVCONST * mass[i] * mass[j] / (d*d);
              ax = (F/mass[i]) * dx/d;
              ay = (F/mass[i]) * dy/d;
              vx[i] += ax;
              vy[i] += ay;
            }
        } // body j
      } // body i
      //gather the variables back in
      timings[3] = omp_get_wtime();

      // having worked out all velocities we now apply and determine new position
      timings[4] = omp_get_wtime();
      //openMP the variables.
      for (i=0; i<BODIES; i++) {
        x[i] += vx[i];
        y[i] += vy[i];
        //DEBUG ONLY: outputBody(i);
      }
      timings[5] = omp_get_wtime();

      printf("---\n");
    } // time
    if (mpiId ==0 ){
      printf("Final data\n");
      for (i=0; i<BODIES; i++) {
        outputBody(i);
      }
    timings[1] = omp_get_wtime();
    printf("Whole execution: %0.6f\n", (timings[1] - timings[0])*1000);
    printf("timing for init: %0.6f\n", (timings[2] - timings[1])*1000);
    printf("timing for velocities: %0.6f\n", (timings[3] - timings[2])*1000);
    printf("timing for new position: %0.6f \n", (timings[5]-timings[4])*1000);
    }
  }

  MPI_Finalize();
}


//Parallelise the printing
void randomInit() {
  int i;
  #pragma omp parallel for private(i) reduction(+:mass, x, y, vx, vy) shared(BODIES)
  for (i=0; i<BODIES; i++) {
    mass[i] = 0.001 + (float)rand()/(float)RAND_MAX;            // 0.001 to 1.001

    x[i] = -250.0 + 500.0*(float)rand()/(float)RAND_MAX;   //  -10 to +10 per axis
    y[i] = -250.0 + 500.0*(float)rand()/(float)RAND_MAX;   //

    vx[i] = -0.2 + 0.4*(float)rand()/(float)RAND_MAX;   // -0.25 to +0.25 per axis
    vy[i] = -0.2 + 0.4*(float)rand()/(float)RAND_MAX;   
  }
  printf("Randomly initialised\n");
  return;
}


//Can also Parallelise the init function
void testInit() {
  /* test: initial zero velocity ==> attraction only ie bodies should move closer together */
  int i;
  for (i=0; i<BODIES; i++) {
    mass[i] = 1.0;

    x[i] = (float) i;
    y[i] = (float) i;

    vx[i] = 0.0;
    vy[i] = 0.0;
  }
}

void testInit2() {
  /* test data */
  mass[0] = 1.0;
  x[0]=0.0;
  y[0]=0.0;
  vx[0]=0.01;
  vy[0]=0.0;
  mass[1] = 0.1;
  x[1]=1.;
  y[1]=1.;
  vx[1]=0.;
  vy[1]=0.;
  mass[2] = .001;
  x[2]=0.;;
  y[2]=1.;
  vx[2]=.01;
  vy[2]=-.01;
}

void outputBody(int i) {
  printf("Body %d: Position=(%f,%f) Velocity=(%f,%f)\n", i, x[i],y[i], vx[i],vy[i]);
  return;
}
