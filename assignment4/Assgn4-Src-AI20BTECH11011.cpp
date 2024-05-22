#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <thread>
#include <ctime>
#include <semaphore.h>
#include <queue>

using namespace std;

long avg, car_avg;  // Average time taken by passengers and cars to complete their tour
float tc, tp, tr;  // car wait, passenger wait and ride times
int k, c, p;  // k is the number of rides each passenger wants to take, c is the number of cars, and p is the number of passengers
FILE * out_file;  // Output file pointer
sem_t sem_c, sem_p, sem_q;  // Semaphores for controlling car and passenger threads, and the queue of available cars
queue<int> q;  // Queue to hold available cars
sem_t sem_avg, sem_car_avg;

// Function to get the current time as a string
string time(){
    auto reqTime = chrono::system_clock::to_time_t(chrono::system_clock::now());  // Request time
    string t = ctime(&reqTime);  // Convert time to string
    string s;  // String to hold the time string without the date
    int j = 0;
    int index = 0;
    while(j < 4){
        if(t[index] == ' ') j++;
        index++;
    }
    while(t[index]!=' ') {s += t[index]; index++;}
    return s;
}

// Passenger function
void passenger(int p_num){
    int car_num;  // Car number assigned to passenger
    auto time1 = chrono::high_resolution_clock::now();  // Starting time for passenger
    fprintf(out_file, "Passenger %d enters the museum at %s \n", p_num, time().c_str());  // Print that passenger has entered the museum
    for(int i=0; i<k; i++){
        sem_post(&sem_p);  // Increase the count of available passengers
        fprintf(out_file, "Passenger %d made a ride request at %s \n", p_num, time().c_str());  // Print that passenger has made a ride request

        sem_wait(&sem_c);  // Wait for a car to be available

        sem_wait(&sem_q);  // Wait for access to the car queue
        car_num = q.front();  // Assign the first available car to the passenger
        q.pop();  // Remove the car from the queue
        sem_post(&sem_q);  // Release access to the car queue
        fprintf(out_file, "Car %d accepts Passenger %d \n", car_num, p_num);  // Print that the car has accepted the passenger
        fprintf(out_file, "Passenger %d started riding at %s \n", p_num, time().c_str());  // Print that the passenger has started riding
        fprintf(out_file, "Car %d is riding Passenger %d \n", car_num, p_num);  // Print that the car is riding the passenger

        this_thread::sleep_for(chrono::milliseconds (int(tr)));  // Sleep for the duration of the ride
        fprintf(out_file, "Car %d has finished Passenger %d's tour \n", car_num, p_num);  // Print that the car has finished the ride
        fprintf(out_file, "Passenger %d finished riding at %s \n", p_num, time().c_str());  // Print that the passenger has finished riding

        sem_wait(&sem_q);  // Wait
        q.push(car_num); // Push car to car queue since ride is finished
        sem_post(&sem_q);

        this_thread::sleep_for(chrono::milliseconds (int(tp)));  // Sleep for the passenger processing time

    }
    auto time2 = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(time2-time1);
    sem_wait(&sem_avg);
    avg += duration.count();
    sem_post(&sem_avg);
    fprintf(out_file, "Passenger %d exit the museum at %s \n", p_num, time().c_str());
}

void car(){
    while(true) {
        auto time1 = chrono::high_resolution_clock::now();  // Starting time for car
        sem_wait(&sem_p); // wait for passenger to become available
        this_thread::sleep_for(chrono::milliseconds (int(tr))); // sleep for ride time
        this_thread::sleep_for(chrono::milliseconds(int(tc))); // sleep for car processing time
        sem_post(&sem_c); // signal that car is available
        auto time2 = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(time2-time1);
        sem_wait(&sem_car_avg);
        car_avg += duration.count();
        sem_post(&sem_car_avg);
    }
}

int main(){
    ifstream inp_file;
    float lambda_p, lambda_c;
    inp_file.open("inp.txt");

    // read the input parameters from a file named "inp.txt"
    inp_file >> p >> c >> lambda_p >> lambda_c >> k;
    inp_file.close();

    // convert lambda_p and lambda_c from per second to per millisecond
    lambda_p *= 1000;
    lambda_c *= 1000;

    // create random number generator engine
    default_random_engine gen;

    // create exponential distribution with mean value of lambda_c
    exponential_distribution<float> distribution_c(float(1.0)/lambda_c);

    // create exponential distribution with mean value of lambda_p
    exponential_distribution<float> distribution_p(float(1.0)/lambda_p);

    // create exponential distribution with mean value of 100 milliseconds
    exponential_distribution<float> distribution_r(float(1.0)/100);

    // generate random values for tp, tc, and tr
    tp = distribution_p(gen);
    tc = distribution_c(gen);
    tr = distribution_r(gen);

    // initialize semaphores
    sem_init(&sem_c, 0, c); // the maximum number of cars that can ride at the same time is c
    sem_init(&sem_p, 0, 0); // initially, there are no passengers waiting for a ride
    sem_init(&sem_q, 0, 1); // the queue can be accessed by only one thread at a time
    sem_init(&sem_avg, 0, 1);
    sem_init(&sem_car_avg, 0, 1);

    // fill the queue with car numbers from 1 to c
    for(int i=0; i<c; i++) q.push(i+1);

    // open the output file for writing
    out_file = fopen("output.txt", "w");

    // create threads for cars and passengers
    thread cars[c];
    for(int i=0; i<c; i++) cars[i] = thread(&car);

    thread passengers[p];
    for(int i=0; i<p; i++) passengers[i] = thread(&passenger, i+1);

    // wait for all passenger threads to finish
    for(int i=0; i<p; i++) passengers[i].join();

    // calculate the average time taken by passengers to complete their tour
    cout << "Average time taken by passengers to complete their tour = " << avg/p << "\n";

    // calculate the average time taken by passengers to complete their tour
    cout << "Average time taken by car to complete its tour = " << car_avg/c << "\n";

    // close the output file and exit the program
    fclose(out_file);
    exit(0);
}
