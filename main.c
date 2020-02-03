/** Adam Casto, Kevin Ehresman
    CSC 345-01
    Project 2  			**/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

//Global variables
int sudoku[9][9];
int valid[27];
int check, threads;
sem_t sem;

//Data struct
typedef struct {
	int row;
	int column;
} parameters;

//Comparator function used for quicksort
int cmpfunc (const void * a, const void * b) {
	return ( *(int*)a - *(int*)b );
}

//Method for checking columns
void* columns(void *param) {
	parameters *datacopy1 = param;
	parameters *datacopy = (parameters *) malloc(sizeof(parameters));
	datacopy->row = datacopy1->row;
	datacopy->column = datacopy1->column;
	
	int error = 0;

	//Copies column into array
	for (int i = 0; i < check && !error; i++) {
		int copy[9];
		for (int j = 0; j < 9; j++) {
			copy[j] = sudoku[datacopy->row][datacopy->column];
			datacopy->row++;
		}

		//Quicksorts array and checks for nums 1-9
		qsort(copy, 9, sizeof(int), cmpfunc);
		for (int k = 0; k < 9; k++)
			if (copy[k] != k + 1) {
				error++;
				break;
			}
		datacopy->column++;
		datacopy->row = 0;
	}
	
	//Updates valid array
	for (int l = 0; l < threads & !error; l++) {
		if (valid[l] == 0) {
			sem_wait(&sem);
			valid[l]++;
			sem_post(&sem);
			break;
		}
	}
	
	free(datacopy);
	pthread_exit(0);
}

//Method for checking rows
void* rows(void *param) {
	parameters *datacopy1 = param;	
	parameters *datacopy = (parameters *) malloc(sizeof(parameters));
	datacopy->row = datacopy1->row;
	datacopy->column = datacopy1->column;
	
	//Copies row into array
	for (int i = 0; i < check; i++) {
		int copy[9];
		for (int j = 0; j < 9; j++) {
			copy[j] = sudoku[datacopy->row][datacopy->column];
			datacopy->column++;
		}

		//Quicksorts array and checks for nums 1-9
		qsort(copy, 9, sizeof(int), cmpfunc);
		for (int k = 0; k < 9; k++)
			if (copy[k] != k + 1) pthread_exit(0);
		datacopy->row++;
		datacopy->column = 0;
	}
	
	//Updates valid array
	for (int l = 0; l < threads; l++) {
		if (valid[l] == 0) {
			sem_wait(&sem);
			valid[l]++;
			sem_post(&sem);
			break;
		}
	}
	
	free(datacopy);
	pthread_exit(0);
}

//Method for checking 3x3 boxes
void* boxes(void *param) {
	parameters *datacopy1 = param;	
	parameters *datacopy = (parameters *) malloc(sizeof(parameters));
	datacopy->row = datacopy1->row;
	datacopy->column = datacopy1->column;

	int copy[9];
	int count = 0;

	//Copies 3x3 box into array
	for (int i = datacopy->row; i < datacopy->row+3; i++) {
		for (int j = datacopy->column; j < datacopy->column+3; j++) {
			copy[count] = sudoku[i][j];
			count++;
		}
	}
	
	//Quicksorts the array and checks for nums 1-9
	qsort(copy, 9, sizeof(int), cmpfunc);
	for (int k = 0; k < 9; k++)
		if (copy[k] != k + 1) pthread_exit(0);	
	
	//Updates valid array
	for (int l = 0; l < threads; l++) {
		if (valid[l] == 0) {
			sem_wait(&sem);
			valid[l]++;
			sem_post(&sem);
			break;
		}
	}
	
	free(datacopy);
	pthread_exit(0);
}

int main(int argc, char** argv) {
	
	//Starts timer	
	clock_t start = clock();

	//Checks input option
	if (atoi(argv[1]) == 1) check = 9;
	if (atoi(argv[1]) == 2) check = 1;

	//Initializes semaphore
	sem_init(&sem, 0, 1);

	//Creates data struct
	parameters *data = (parameters *) malloc(sizeof(parameters));
	data->row = 0;
	data->column = 0;
    
	//Opens input file
	FILE *input;
	input = fopen("input.txt", "r");
    
	//Reads sudoku board from input.txt
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			fscanf(input, "%d", &sudoku[i][j]);
    
	//Prints out sudoku board
	printf("BOARD STATE IN input.txt:\n");
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++)
			printf("%d ", sudoku[i][j]);
	printf("\n");
	}
    
    
    	//Creates threads for option 1
	if (check == 9) {
		threads = 11;
        	pthread_t tid[threads];

		pthread_create(&tid[0], NULL, columns, data);
		pthread_create(&tid[1], NULL, rows, data);
		pthread_join(tid[0], NULL);
		pthread_join(tid[1], NULL);
		
		for (int i = 2; i < threads; i++) {
			pthread_create(&tid[i], NULL, boxes, data);
			pthread_join(tid[i], NULL);
			if (data->column == 6) {
				data->column = 0;
				data->row += 3;
			} else {
				data->column += 3;
			}
		}
	
	}

    	//Creates threads for option 2
	if (check == 1) {
		threads = 27;
        	pthread_t tid[threads];
		
		for (int i = 0; i <= 8; i++) {
			pthread_create(&tid[i], NULL, columns, data);
			pthread_join(tid[i], NULL);
			data->column++;
		}
		data->column = 0;
		for (int i = 9; i <= 17; i++) {
			pthread_create(&tid[i], NULL, rows, data);
			pthread_join(tid[i], NULL);
			data->row++;
		}
		data->row = 0;
		for (int i = 18; i < threads; i++) {
			pthread_create(&tid[i], NULL, boxes, data);
			pthread_join(tid[i], NULL);
			if (data->column == 6) {
				data->column = 0;
				data->row += 3;
			} else {
				data->column += 3;
			}
		}
        	
	}
    	
	//Cleaning up
	fclose(input);
	sem_destroy(&sem);
	free(data);
	
	//Checks if board is a solution
	int solution = 1;
	for (int i = 0; i < threads; i++)
		if (valid[i] == 0) solution = 0;

	//Ends timer and calculates seconds
	clock_t end = clock() - start;
	double time = (double) end / CLOCKS_PER_SEC;
    	
	//Prints if board is a solution
	if (solution == 1)
		printf("SOLUTION: %s (%f seconds)\n", "YES", time);
	if (solution == 0)
		printf("SOLUTION: %s (%f seconds)\n", "NO", time);
    	
	return 0;
}
