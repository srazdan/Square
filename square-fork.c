#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

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

// Size of the grid of characters.
int rows, cols;
int total = 0;

// Maximum width of a row.  Makes it easier to allocate the whole
// grid contiguously.
#define MAX_WIDTH 16384

// Type used for a row of the grid.  Makes it easier to declare the
// grid as a pointer.
typedef char Row[ MAX_WIDTH ];

// Grid of letters.
Row *grid;

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
    char buffer[ MAX_WIDTH + 1 ];
    for ( int r = 0; r < rows; r++ ) {
		scanf( "%s", buffer );
		memcpy( grid[ r ], buffer, cols );
    }
}

// checks the 6*6 grid. 
bool checkGrid(int row, int col) {
	char letters[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'}; // array of alphabets. 
	for( int i = row; i < row + 6; i++) {
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

int main( int argc, char *argv[] ) {
    bool report = false;
    int workers = 4;

    // Parse command-line arguments.
    if ( argc < 2 || argc > 3 )
		usage();
  
    if ( sscanf( argv[ 1 ], "%d", &workers ) != 1 || workers < 1 )
		usage();

    // If there's a second argument, it better be "report"
    if ( argc == 3 ) {
		if ( strcmp( argv[ 2 ], "report" ) != 0 )
			usage();
		report = true;
	}

	readGrid(); // read the grid. 
  
	int pfd[2];
	pipe(pfd); // make a pipe. 
	pid_t pid = 0; //initialize pid to be 0. 
	
	for(int i = 0; i < workers; i++) {
		pid = fork(); // fork to create a child. 
		if ( pid == -1 ) {
			fail( "Can't create child process" ); //if fork is -1, the child failed to create. 
		}
		
		if ( pid == 0 ) {//if 0, child created. 
			close(pfd[0]); //close reading end of pipe of child process. 
			int local = 0; //local count of the child. 
			for( int r = 0; i + (r * workers) < rows; r++) {
				for( int c = 0; c < cols; c++) {
					if( i + (r * workers) + 6 <= rows && c + 6 <= cols && checkGrid(i + (r * workers), c)) {
						if(report) {
							printf("%d %d\n", i + (r * workers), c); // if left top corner can form a 6*6 grid, and report is true, report is printed. 
						}
						local++;//local count incremented. 
					}
				}
			}
			write(pfd[1], &local, sizeof(int));//write local count to the writing end of the pipe. 
			close(pfd[1]);//close writing end of the pipe for the child. 
			break;//break so that a new child can be formed. 
		} else if ( pid > 0) {
			continue;//if it is a parent, continue. 
		}
	}
	
	if ( pid > 0 ) {
		for ( int i = 0; i < workers; i++) {
			wait ( NULL );//wait for all children to finish. 
		}
	
		int *a = (int*) malloc (sizeof(int)); // malloc space for local count to be read and added. 
		for ( int i = 0; i < workers; i++) {
			read(pfd[0], a, sizeof(int));//read from reading end of the parent pipe. 
			total += *a; // add to total. 
		}
		free(a);//free the integer. 
		free(grid);//free the grid. 
		printf("Squares: %d\n", total);//print the total number of squares. 
		close(pfd[0]);//close reading end of the pipe. 
		close(pfd[1]);//close writing end of the pipe. 
	}
	
  // ...

  return 0;
}
