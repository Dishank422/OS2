#include <iostream>
#include <fstream>
#include <chrono>
#include <set>
#include <cmath>
#include <omp.h>
#include <pthread.h>

using namespace std;

int * is_row_valid; // global variable to store whether row is valid
int * is_col_valid; // global variable to store whether column is valid
int * is_grid_valid; // global variable to store whether grid is valid
int ** sudoku; // global variable to store sudoku
int dim;
int n_threads;

// task object that is passed by main thread to child thread
class task{
public:
    int row_col_or_grid = -1; // 0 - row, 1 - col, 2 - grid
    int task_num = -1; // 1 to N
    int thread_num = -1;
};

// processing done on each child thread
void * validator(void * assigned_task){
    set <int> a; // set to store the numbers from 1 to N
    task assigned_t = * (task *) assigned_task;

    // verifying the task
    if(assigned_t.row_col_or_grid==0){
        for(int i=0; i<dim; i++){
            // storing numbers in set
            if((sudoku[assigned_t.task_num][i]<=dim)&&(sudoku[assigned_t.task_num][i]>=1)) a.insert(sudoku[assigned_t.task_num][i]);
        }
        // if size of set is N, then task is valid
        if(a.size() == dim) is_row_valid[assigned_t.task_num] = 1;
        else is_row_valid[assigned_t.task_num] = 0;
    }
    else if(assigned_t.row_col_or_grid==1){
        for(int i=0; i<dim; i++){
            // storing numbers in set
            if((sudoku[i][assigned_t.task_num]<=dim)&&(sudoku[i][assigned_t.task_num]>=1)) a.insert(sudoku[i][assigned_t.task_num]);
        }
        // if size of set is N, then task is valid
        if(a.size() == dim) is_col_valid[assigned_t.task_num] = 1;
        else is_col_valid[assigned_t.task_num] = 0;
    }
    else{
        for(int i=0; i<sqrt(dim); i++){
            for(int j=0; j<sqrt(dim); j++) {
                // storing numbers in set
                int grid_i, grid_j;
                grid_i = int(sqrt(dim))*(assigned_t.task_num/int(sqrt(dim)));
                grid_j = int(sqrt(dim))*(assigned_t.task_num%int(sqrt(dim)));
                if ((sudoku[grid_i+i][grid_j+j] <= dim) && (sudoku[grid_i+i][grid_j+j] >= 1)) a.insert(sudoku[grid_i+i][grid_j+j]);
            }
        }
        // if size of set is N, then task is valid
        if(a.size() == dim) is_grid_valid[assigned_t.task_num] = 1;
        else is_grid_valid[assigned_t.task_num] = 0;
    }
    return nullptr;
}

int main() {

    ifstream input_file;
    input_file.open("inp.txt"); // opening input file
    if(input_file.is_open()) {
        input_file >> dim; // reading from input file
        input_file >> n_threads;

        // allocating memory to sudoku
        sudoku = (int **) malloc(sizeof(int *)*dim);
        for(int k=0; k<dim; k++) sudoku[k] = (int *) malloc(sizeof(int)*dim);

        // reading sudoku from input file
        for(int i=0; i<dim; i++){
            for(int j=0; j<dim; j++){
                input_file >> sudoku[i][j];
            }
        }
        input_file.close(); // closing input file
    }
    else {
        cout << "Input file could not be opened. Please make sure that the input file is placed correctly.";
        exit(0);
    }

    is_row_valid = (int *) malloc(sizeof(int)*dim); // initializing row flags
    is_col_valid = (int *) malloc(sizeof(int)*dim); // initializing column flags
    is_grid_valid = (int *) malloc(sizeof(int)*dim); // initializing grid flags

    auto begin = chrono::high_resolution_clock::now(); // noting starting time

    // creating array of 3*N tasks and initializing them to correct initial values
    task tasks[dim*3];
    for(int i=0; i<dim; i++) {
        tasks[i].row_col_or_grid = 0;
        tasks[i].task_num = i;
    }
    for(int i=0; i<dim; i++) {
        tasks[dim+i].row_col_or_grid = 1;
        tasks[dim+i].task_num = i;
    }
    for(int i=0; i<dim; i++) {
        tasks[2*dim+i].row_col_or_grid = 2;
        tasks[2*dim+i].task_num = i;
    }

    // Using OpenMP to parallelize the following for loop. validator validates the assigned task.
#pragma omp parallel for num_threads(n_threads)
        for(int i=0; i<3*dim; i++)
    {
        tasks[i].thread_num = omp_get_thread_num();
        validator(&tasks[i]);
    }

    // opening output file
    ofstream output_file;
    output_file.open("output.txt"); //opening output file

    // checking whether sudoku is valid, all numbers in is_row_valid, is_col_valid, is_grid_valid should be 1.
    int valid_sudoku = 1;
    for(int i=0; i<dim; i++) valid_sudoku = min(valid_sudoku, is_row_valid[i]);
    for(int i=0; i<dim; i++) valid_sudoku = min(valid_sudoku, is_col_valid[i]);
    for(int i=0; i<dim; i++) valid_sudoku = min(valid_sudoku, is_grid_valid[i]);


    auto end = chrono::high_resolution_clock::now(); // noting end time
    auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end-begin); // calculating elapsed time

    //writing logs for each thread
    for(int i=0; i<3*dim; i++){
        output_file << "Thread " << tasks[i].thread_num+1 << " checks ";
        if(tasks[i].row_col_or_grid==0) {
            output_file << "row " << tasks[i].task_num+1 << " and is ";
            if(is_row_valid[tasks[i].task_num]) output_file << "valid.\n";
            else output_file << "invalid.\n";
        }
        else if(tasks[i].row_col_or_grid==1) {
            output_file << "column " << tasks[i].task_num+1 << " and is ";
            if(is_col_valid[tasks[i].task_num]) output_file << "valid.\n";
            else output_file << "invalid.\n";
        }
        else {
            output_file << "grid " << tasks[i].task_num+1 << " and is ";
            if(is_grid_valid[tasks[i].task_num]) output_file << "valid.\n";
            else output_file << "invalid.\n";
        }
    }

    // writing whether sudoku is valid or invalid.
    if(valid_sudoku) output_file << "\nSudoku is valid.\n";
    else output_file << "\nSudoku is invalid.\n";

    output_file << "Time: " << elapsed.count()*1e-9 << " s\n"; // writing time to output file

    output_file.close(); //closing output file
    return 0;
}
