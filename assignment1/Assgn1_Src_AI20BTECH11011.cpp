#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <pthread.h>

using namespace std;

double * points;  // global variable to store points
int * inside_circle; // global variable to store flags (whether point is inside circle or not)
int n_points;
int n_threads;
default_random_engine gen; // random number generator
uniform_real_distribution<double> distribution(-1.0, 1.0); // distribution to sample random numbers from

// processing done on each child thread
void * pi_computer(void * num){
    double x, y;
    int thread_num = * (int *) num;
    for(int i=0; i<n_points/n_threads; i++){
        x = distribution(gen); // generating a point
        y = distribution(gen);
        if(x*x+y*y <= 1) inside_circle[thread_num*(n_points/n_threads)+i] = 1; // flag is set if point is inside circle
        else inside_circle[thread_num*(n_points/n_threads)+i] = 0; // flag is 0 if point is not inside circle
        points[2*(thread_num*(n_points/n_threads)+i)] = x; // storing point in global variable
        points[2*(thread_num*(n_points/n_threads)+i)+1] = y;
    }
    return nullptr;
}

int main() {
    auto begin = chrono::high_resolution_clock::now(); // noting starting time
    ifstream input_file;
    input_file.open("inp.txt"); // opening input file
    if(input_file.is_open()) {
        input_file >> n_points; // reading from input file
        input_file >> n_threads;
        input_file.close(); // closing input file
    }
    else {
        cout << "Input file could not be opened. Please make sure that the input file is placed correctly.";
        exit(0);
    }

    points = (double *) malloc(sizeof(double)*2*n_threads*(n_points/n_threads)); // initializing global variables
    inside_circle = (int *) malloc(sizeof(int)*n_threads*(n_points/n_threads));

    int nums[n_threads];
    for(int i=0; i<n_threads; i++) nums[i] = i;
    
    pthread_t my_threads[n_threads]; // creating an array of thread objects
    for(int i=0; i<n_threads; i++) {
        pthread_create(&my_threads[i], NULL, pi_computer, &nums[i]);
    } // launching the threads

    ofstream output_file;
    output_file.open("output.txt"); //opening output file

    for(int i=0; i<n_threads; i++) pthread_join(my_threads[i], NULL); // joining the threads in a for loop

    // calculating pi by counting the number of points inside the circle using the flags and using the formula in the textbook
    double pi = 0;
    for(int i=0; i<n_threads * (n_points/n_threads); i++){
        pi += inside_circle[i];
    }
    pi /= n_points;
    pi *= 4;

    auto end = chrono::high_resolution_clock::now(); // noting end time
    auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end-begin); // calculating elapsed time

    output_file << "Time: " << elapsed.count()*1e-9 << " s\n\n"; // writing time to output file
    output_file << "Value computed: " << pi << "\n\n"; // writing value of pi to output file

    //writing logs for each thread
    output_file << "Log:\n\n";
    for(int i=0; i<n_threads; i++){
        int square = 0, circle = 0;
        // counting number of points inside and outside the circle
        for(int j=0; j<n_points/n_threads; j++){
            circle++;
            if (inside_circle[i*(n_points/n_threads)+j] == 1) square++;
        }
        output_file << "Thread " << i+1 << ": " << square << ", " << circle << "\n"; // printing the number of points inside and outside the circle
        //printing points outside the circle
        output_file << "Points inside the square: ";
        for(int j=0; j<n_points/n_threads; j++) output_file << "(" << points[2*(i*(n_points/n_threads)+j)] << ", " << points[2*(i*(n_points/n_threads)+j)+1] << "), ";
        //printing points inside the circle
        output_file << "\nPoints inside the circle: ";
        for(int j=0; j<n_points/n_threads; j++){
            if(inside_circle[i*(n_points/n_threads)+j]) output_file << "(" << points[2*(i*(n_points/n_threads)+j)] << ", " << points[2*(i*(n_points/n_threads)+j)+1] << "), ";
        }
        output_file << "\n\n";
    }
    output_file.close(); //closing output file
    return 0;
}
