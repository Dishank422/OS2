#include "types.h"
#include "stat.h"
#include "user.h"

#define N 3000
int a[N];

int main(){
    printf (1, "Before fork:\n");
    pgtprint();
    printf(1, "Value: %d\n", a[N-1]);
    int pid = fork();
    if(pid==0) {
        printf(1, "After fork, in child process:\n");
        pgtprint();
        printf(1, "Value: %d\n", a[N-1]);
        a[N-1] = 10;
        printf(1, "After fork, in child process after modifying:\n");
        pgtprint();
        printf(1, "Value: %d\n", a[N-1]);
    } else {
        wait();
        printf(1, "After fork, in parent process after child terminates:\n");
        pgtprint();
        printf(1, "Value: %d\n", a[N-1]);
    }
    exit();
}
