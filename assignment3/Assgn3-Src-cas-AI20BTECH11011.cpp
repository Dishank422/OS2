#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <thread>
#include <ctime>
#include <atomic>
using namespace std;

int n, k;
float lambda1, lambda2;
default_random_engine gen;
exponential_distribution<float> distribution1(float(1.0)/lambda1); //distribution to generate t1
exponential_distribution<float> distribution2(float(1.0)/lambda2); //distribution to generate t2
FILE *output_file; //output file pointer
atomic<int> lock(0); //atomic variable lock
long avg, worst; //avg and worst wait times

void testCS(int id)
{
    float t1, t2;
    t1 = distribution1(gen);//t1 is time to execute critical section
    t2 = distribution2(gen);//t2 is time to wait before requesting to enter critical section again

    for(int i=0; i<k; i++){

        auto reqEnterTime = chrono::system_clock::to_time_t(chrono::system_clock::now());//request time
        string t = ctime(&reqEnterTime);
        string s;
        int j = 0;
        int index = 0;
        while(j < 3){
            if(t[index] == ' ') j++;
            index++;
        }
        while(t[index]!=' ') {s += t[index]; index++;}//converting time to required string format manually
        fprintf(output_file, "%d th CS Request at %s by thread %d\n", i+1, s.c_str(), id+1);//printing request time

        auto time1 = chrono::high_resolution_clock::now();
        while (true)//spinning around lock
        {
            int a = 0, b = 1;
            if(atomic_compare_exchange_weak(&lock, &a, b))
            {
                break;
            }
        }

        auto time2 = chrono::high_resolution_clock::now();
        auto actEnterTime = chrono::system_clock::to_time_t(chrono::system_clock::now());//enter time
        t = ctime(&actEnterTime);
        s = "";
        j = 0;
        index = 0;
        while(j < 3){
            if(t[index] == ' ') j++;
            index++;
        }
        while(t[index]!=' ') {s += t[index]; index++;}//converting time to required string format manually
        fprintf(output_file, "%d th CS Entry at %s by thread %d\n", i+1, s.c_str(), id+1);//printing enter time
        this_thread::sleep_for(chrono::milliseconds(int(t1)));//putting thread to sleep

        auto exitTime = chrono::system_clock::to_time_t(chrono::system_clock::now());//exit time
        t = ctime(&exitTime);
        s = "";
        j = 0;
        index = 0;
        while(j < 3){
            if(t[index] == ' ') j++;
            index++;
        }
        while(t[index]!=' ') {s += t[index]; index++;}//converting time to required string format
        fprintf(output_file, "%d th CS Exit at %s by thread %d\n", i+1, s.c_str(), id+1);//printing exit time

        lock = 0;//releasing the lock

        auto duration = chrono::duration_cast<chrono::microseconds>(time2-time1);
        avg += duration.count();//updating average wait time
        worst = max(worst, duration.count());//updating worst wait time
        this_thread::sleep_for(chrono::milliseconds(int(t2)));//waiting for t2 time before requesting again
    }
}

int main()
{
    ifstream input_file;
    input_file.open("inp.txt");//opening input file
    if(input_file.is_open()) {
        input_file >> n >> k >> lambda1 >> lambda2; // reading from input file
        input_file.close(); // closing input file
    }
    else {
        cout << "Input file could not be opened. Please make sure that the input file is placed correctly.";
        exit(0);
    }

    lambda1 *= 1000;
    lambda2 *= 1000;
    output_file = fopen("output.txt", "w");//opening output file
    fprintf(output_file, "Bounded CAS ME Output:\n");
    thread threads[n];
    for(int i=0; i<n; i++){
        threads[i] = thread(testCS, i);//creating n threads
    }

    for(int i=0; i<n; i++){
        threads[i].join();//waiting for all threads to join
    }

    cout << "Average wait time: " << avg/(n*k) << " microseconds\n";//printing average wait time
    cout << "Worst wait time: " << worst << " microseconds\n";//printing worst wait time
    fclose(output_file); //closing output file
    return 0;
}