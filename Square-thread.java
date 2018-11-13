import java.util.Scanner;

/**
 * Square Class
 * @author Sanchit Razdan
 */
public class Square {
	public static final int SQUARE_HEIGHT = 6; //max sqaure height
	public static final int SQUARE_WIDTH = 6; //max square grid width
	public static final int MAX_WIDTH = 16384; //max width of total grid
	public static int rows = -1; //rows of the grid
	public static int cols = -1; //columns of the grid
	public static int total = 0; //total number of squares
	public static char[][] grid; //actual whole grid
	
	/**
	 * Fail method. Prints the fail method. 
	 * @param message the message to be printed. 
	 */
	public static void fail( String message ) {
		System.err.println( message ); //print the message. 
		System.exit( 1 ); //exit because program failed. 
	}
	
	/**
	 * usage method. Prints the usage of the program. 
	 */
	public static void usage() {
	    System.out.println( "usage: Square <workers>" ); //prints the usage message. 
	    System.out.println( "       Square <workers> report" ); //prints the usage method. 
	    System.exit( 1 ); //exit because program failed. 
	}
	
	/**
	 * Work class. Instance of a Runnable Interface. 
	 * @author Sanchit Razdan
	 */
	static class Work implements Runnable {
		private boolean report; // report boolean if points are to be printed or not. 
		private int workers; //number of workers. 
		private int worker_number; //current worker number. 
		
		/**
		 * Work Constructor. 
		 * @param report the boolean report. 
		 * @param workers the total number of workers. 
		 * @param worker_number current worker number.
		 */
		public Work(boolean report, int workers, int worker_number) {
			this.report = report; //initializes the report boolean variable. 
			this.workers = workers;//initializes the number of workers. 
			this.worker_number = worker_number; //initializes the current worker number. 
		}

		/**
		 * run method. Runs the thread. 
		 */
		public void run() {
			for( int r = 0; (worker_number + (r * workers)) < rows; r++) {
				for( int c = 0; c < cols; c++) {
					if( worker_number + (r * workers) + 6 <= rows && c + 6 <= cols && checkGrid(worker_number + (r * workers), c)) {
						if(report) {
							System.out.println((worker_number + (r * workers)) + " " + c); // if left top corner can form a 6*6 grid, and report is true, report is printed. 
						}
						total++;//total count incremented. 
					}
				}
			}
		}

		/**
		 * checkGrid method. Checks if the 6*6 grid is correct. 
		 * @param row the row that the grid is starting from. 
		 * @param col the column that the grid is starting from. 
		 * @return true if the grid is square and false if not. 
		 */
		boolean checkGrid(int row, int col) {
			char[] letters = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'}; // array of alphabets. 
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
	}
	
	/**
	 * readGrid method. Makes the grid from the file. 
	 */
	public static void readGrid() {
		Scanner s =  new Scanner(System.in); //Scanner for file. 
		String line = s.nextLine(); //gets the line of rows and columns numbers. 
		Scanner sl = new Scanner(line); //scans the line. 
		rows = sl.nextInt(); //get the number of rows. 
		cols = sl.nextInt(); //get the number of columns. 
		sl.close();//close the first line scanner. 
		grid = new char[rows][cols]; //makes the grid. 
		if ( cols > MAX_WIDTH ) {
	        System.err.println("Input grid is too wide.");//if columns are more then maximum width, error out.
	        System.exit( 1 );//exit. 
	    }
		char[] buffer = new char[MAX_WIDTH + 1];
		for ( int r = 0; r < rows; r++) {
			buffer = s.nextLine().toCharArray(); //get each line into buffer. 
			 grid[r] = buffer;//copy cahr of buffer into grid. 
		}
		s.close();//close the original file scanner. 
	}
	
	/**
	 * main method. 
	 * @param args the command line arguments array. 
	 */
	public static void main(String[] args) {
		boolean report = false; //if program is to reported or not. 
	    int workers = 4; //number of workers.

	    // Parse command-line arguments.
	    if ( args.length < 1 || args.length > 2 ) {
	    	usage(); // if number of args are incorrect, print usage. 
	    }
	  
	    if ( !(Character.isDigit(args[0].charAt(0))) || workers < 1 ) {
	    	usage();//if number of workers not listed or less than 1, print usage. 
	    } else {
	    	workers = Integer.parseInt(args[0]);
	    }

	    // If there's a second argument, it better be "report"
	    if ( args.length == 2 ) {
			if ( !(args[1].equals("report"))) {
				usage();//if report is incorrectly listed, print usage. 
			} else {
				report = true;
			}
		}
	    
	    readGrid();//read from file and make the character grid. 
	    
	    for ( int i = 0; i < workers; i++) {
	    	Thread t = new Thread(new Work(report, workers, i));//make threads according to number of workers. 
	    	t.start();//start the threads. 
	    	try {
	    		t.join();//terminate the threads. 
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
	    }
		System.out.println("Squares: " + total);//print the total number of squares. 
	}
}
