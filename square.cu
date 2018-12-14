// Elapsed Real Time for input-4.txt: 1.381 seconds

#include <stdio.h>
#include <stdbool.h>
#include <cuda_runtime.h>

// Size of the square we're looking for.
#define SQUARE_WIDTH 6
#define SQUARE_HEIGHT 6

// Maximum width of a row.  Makes it easier to allocate the whole
// grid contiguously.
#define MAX_WIDTH 16384

// Type used for a row of the grid.  Makes it easier to declare the
// grid as a pointer.
typedef char Row[ MAX_WIDTH ];

// Size of the grid of characters.
int rows, cols;

// Grid of letters.
Row *grid;

// Kernel, run by each thread to count complete squares in parallel.
__global__ void countSquares( int rows, int cols, bool report, int *gpuResults, Row *gridCpy ) {
  // Unique index for this worker.
  int r0 = blockDim.x * blockIdx.x + threadIdx.x;
  int c = 0;

  // Make sure I actually have something to work on.
  if ( r0 + SQUARE_HEIGHT - 1 < rows ) {
    for(int col = 0; col < cols; col++) {
	  if(col + 6 <= cols) {
	    bool check = false;//check if that grid actually makes a square or not. 
	    char letters[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'}; // array of alphabets. 
	    for( int i = r0; i < r0 + 6; i++) {
		  for( int j = col; j < col + 6; j++) {
			for( int k = 0; k < 26; k++) {
			  if(gridCpy[i][j] == letters[k]) {  //if cell in grid has any aplhabet, change that alphabet bucket to 0. 
			    letters[k] = '0';//if the letter is not yet, remove it. 
			  }
			}
		  } 
	    }
		
		for(int k = 0; k < 26; k++) {
		  if(letters[k] != '0') {
		    check = false;//if any letter in the array is not 0, the grid is not a square. 
			break;
		  } else {
		    check = true;//else it is true and continue to make sure all if true. 
		  }
		}
		
		if(check == true) {
	      c += 1;//if it is a perfect square grid, increment the count for that index
		  if(report) {
		    printf("%d %d\n", r0, col);//if report is true, print the index
		  }
		}
	  }
	}
  }
  gpuResults[r0] = c;//put number of squares in the array
}

// Read the grid of characters.
void readGrid() {
  // Read grid dimensions.
  scanf( "%d%d", &rows, &cols );
  if ( cols > MAX_WIDTH ) {
    fprintf( stderr, "Input grid is too wide.\n" );
    exit( EXIT_FAILURE );
  }

  // Make space to store the grid as a big, contiguous array.
  grid = (Row *) malloc( rows * sizeof( Row ) );
  
  // Read each row of the grid as a string, then copy everything
  // but the null terminator into the grid array.
  int rowCount = 0;
  char buffer[ MAX_WIDTH + 1 ];
  while ( rowCount < rows ) {
    scanf( "%s", buffer );
    memcpy( grid[ rowCount++ ], buffer, cols );
  }
}

// General function to report a failure and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Print out a usage message, then exit.
static void usage() {
  printf( "usage: square [report]\n" );
  exit( 1 );
}

//main
int main( int argc, char *argv[] ) {
  // If there's an argument, it better be "report"
  bool report = false;
  if ( argc == 2 ) {
    if ( strcmp( argv[ 1 ], "report" ) != 0 )
      usage();
    report = true;
  }

  readGrid();
  
  // Need to add code to allocate memory on the device and copy the grid
  // over.
  Row *gridCpy = NULL;
  cudaMalloc( (void **)&gridCpy,  rows * sizeof( Row ) );
  cudaMemcpy( gridCpy, grid, rows * sizeof( Row ), cudaMemcpyHostToDevice );
  
  //allocate memory for results array
  int *gpuResults = NULL;
  cudaMalloc((void **)&gpuResults, rows * sizeof(int));

  // Block and grid dimensions.
  int threadsPerBlock = 250;
  // Round up.
  int blocksPerGrid = ( rows + threadsPerBlock - 1 ) / threadsPerBlock;

  // Run our kernel on these block/grid dimensions
  countSquares<<<blocksPerGrid, threadsPerBlock>>>( rows, cols, report, gpuResults, gridCpy );
  if ( cudaGetLastError() != cudaSuccess )
    fail( "Failure in CUDA kernel execution." );

  int *results = (int *) malloc( rows * sizeof( int ) );

  // Need to add code to copy the results list back to the host and
  // add them up.
  cudaMemcpy( results, gpuResults, rows * sizeof(int), cudaMemcpyDeviceToHost);
  int total = 0;
  for(int i = 0; i < rows; i++) {
    total += results[i];//add all integers in results array to get total 
  }
  printf( "Squares: %d\n", total );

  // Free memory on the device and the host.
  free(grid);
  free(results);
  cudaFree(gridCpy);
  cudaFree(gpuResults);

  cudaDeviceReset();
  
  return 0;
}
