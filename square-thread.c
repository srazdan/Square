#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

// Size of the square we're looking for.
#define SQUARE_WIDTH 6
#define SQUARE_HEIGHT 6

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Print out a usage message, then exit.
static void usage() {
  printf( "usage: square <workers>\n" );
  printf( "       square <workers> report\n" );
  exit( 1 );
}

// True if we're supposed to report what we find.
bool report = false;

// Size of the grid of characters.
int rows, cols;

// Number of rows we've filled in so far.
int rowCount = 0;

int check = 0;

// Maximum width of a row.  Makes it easier to allocate the whole
// grid contiguously.
#define MAX_WIDTH 16384

// Type used for a row of the grid.  Makes it easier to declare the
// grid as a pointer.
typedef char Row[ MAX_WIDTH ];

// Grid of letters.
Row *grid;

// Total number of complete squares we've found.
int total = 0;

sem_t s1;
sem_t s2;
sem_t s3;

// Read the grid of characters.
void readGrid() {
  // Read grdi dimensions.
  scanf( "%d%d", &rows, &cols );
  if ( cols > MAX_WIDTH ) {
    fprintf( stderr, "Input grid is too wide.\n" );
    exit( EXIT_FAILURE );
  }

  // Make space to store the grid as a big, contiguous array.
  grid = (Row *) malloc( rows * sizeof( Row ) );
  
  // Read each row of the grid as a string, then copy everything
  // but the null terminator into the grid array.
  char buffer[ MAX_WIDTH + 1 ];
  while ( rowCount < rows ) {
    scanf( "%s", buffer );
    
    memcpy( grid[ rowCount ], buffer, cols );
    rowCount += 1;
	sem_post(&s1);
  }
}

// gets the row for the thread to work on. 
int getWorker() {
  sem_wait(&s1);
  sem_wait(&s3);
  int c = check;
  check++;
  sem_post(&s3);
  return c;
}

// checks the 6*6 grid. 
bool checkGrid(int row, int col) {
	char letters[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'}; // array of alphabets. 
	for( int i = row - 5; i <= row; i++) {
		for( int j = col; j < col + 6; j++) {
			for( int k = 0; k < 26; k++) {
				if(grid[i][j] == letters[k]) {  //if cell in grid has any aplhabet, change that alphabet bucket to 0. 
					letters[k] = '0';
				}
			}
		}
	}
	
	for( int k = 0; k < 26; k++) {
		if(letters[k] != '0') {
			return false; // if all buckets in the array are not 0, the grid is not a correct 6*6. 
		}
	}
	return true; // if all the buckets in array are 0, the grid is a correct 6*6. 
}

/** Start routine for each worker. */
void *workerRoutine( void *arg ) {

  while( true ) {
	  int r = getWorker();
	  if( r >= rows ) {
		break;
	  }
	  for(int c = 0; c < cols; c++) {
		if(r - 5 >= 0 && c + 6 <= cols && checkGrid(r, c)) {
		  if(report) {
			printf("%d %d\n", r - 5, c); // if left top corner can form a 6*6 grid, and report is true, report is printed. 
		  }
		  sem_wait(&s2);
		  total++;
		  sem_post(&s2);
		}
	  }
  }
  return NULL;
}
  
int main( int argc, char *argv[] ) {
  int workers = 4;
  
  // Parse command-line arguments.
  if ( argc < 2 || argc > 3 )
    usage();
  
  if ( sscanf( argv[ 1 ], "%d", &workers ) != 1 ||
       workers < 1 )
    usage();

  // If there's a second argument, it better be "report"
  if ( argc == 3 ) {
    if ( strcmp( argv[ 2 ], "report" ) != 0 )
      usage();
    report = true;
  }


  // Make each of the workers.
  pthread_t worker[ workers ];
  for ( int i = 0; i < workers; i++ ) {
	if ( pthread_create( &worker[i], NULL, workerRoutine, NULL ) != 0 ) {
	  fail( "Can't create a child thread" );
	}
  }
  
  if(sem_init(&s1, 0, 0) == -1) {
	fail( "Cannot create semaphore.");
  }
  
  if(sem_init(&s2, 0, 1) == -1) {
	fail("Cannot create semaphore.");
  }
  
  if(sem_init(&s3, 0, 0) == -1) {
	fail("Cannot create semaphore.");
  }
    // ...

  // Then, start getting work for them to do.
  readGrid();
  
  for(int i = 0; i < workers; i++) {
	sem_post(&s1);
	sem_post(&s3);
  }

  // Wait until all the workers finish.
  for ( int i = 0; i < workers; i++ )
    // ...
	pthread_join(worker[i], NULL);

  // Report the total and release the semaphores.
  printf( "Squares: %d\n", total );
  
  return EXIT_SUCCESS;
}
