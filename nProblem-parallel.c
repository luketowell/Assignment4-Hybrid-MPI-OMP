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

  //figured out the numLocal;
  int numLocal = BODIES/numMPI;
  float local_x[numLocal], local_y[numLocal];
  printf("%d \n", numLocal);	
  #pragma omp parallel default(none) shared(numOMP)
  {
     numOMP = omp_get_num_threads();
     printf("%d \n", numOMP);
  }
  
  if (N < numMPI*numOMP) {
    if (mpiId ==0 ) printf("too trivial\n");    // we do not cater for idle cores
  }
  if (N%(numMPI*numOMP)!=0){
		printf("the array length provided is not evenly divisible between nodes");
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
  else {
    //thread 0 initialises all the values
    if (mpiId == 0){
    randomInit();
    }
    //work out the start positions and the end positions for all of the nodes
    int start = mpiId*numLocal;
    int end = (mpiId * numLocal) + numLocal;
    //broadcast out the values of the arrays.
    MPI_Bcast(&vy,BODIES,MPI_FLOAT,0,MPI_COMM_WORLD);
    MPI_Bcast(&vx,BODIES,MPI_FLOAT,0,MPI_COMM_WORLD);
    MPI_Bcast(&x,BODIES,MPI_FLOAT,0,MPI_COMM_WORLD);
    MPI_Bcast(&y,BODIES,MPI_FLOAT,0,MPI_COMM_WORLD);
    MPI_Bcast(&mass,BODIES,MPI_FLOAT,0,MPI_COMM_WORLD);
    

    // each time step is performed by every mpi thread
    for (time=0; time<TIMESTEPS; time++) {
      printf("Timestep %d\n",time);
      //each thread core works out the position of a different collection of bodies e.g. the zero node will work out the positions and velocities of 625 bodies

      for (i=start; i<end; i++) {
        // calc forces on body i due to bodies (j != i)
        for (j=start; j<end; j++) {
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

      // having worked out all velocities we now apply and determine new position
      //#pragma omp parallel for default(none) private(i, start, end) shared(local_x, local_y, x, y, vx, vy)
      for (i=0; i<numLocal; i++) {
        local_x[i] = x[i+start] += vx[i+start];
        local_y[i] = y[i+start] += vy[i+start];
        // outputBody(i);
      }
     
      MPI_Allgather(&local_x, numLocal, MPI_FLOAT, &x, numLocal, MPI_FLOAT, MPI_COMM_WORLD);
      MPI_Allgather(&local_y, numLocal, MPI_FLOAT, &y, numLocal, MPI_FLOAT, MPI_COMM_WORLD);
      printf("---\n");
    } // time
    if (mpiId ==0 ){
      printf("Final data\n");
      for (i=0; i<BODIES; i++) {
        outputBody(i);
      }
    }
  }

  MPI_Finalize();
}


//Parallelise the printing
void randomInit() {
  int i;
  //#pragma omp parallel for private(i) reduction(+: mass, x, y, vx, vy) shared(BODIES)
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