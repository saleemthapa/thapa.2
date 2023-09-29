#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
// Define the SystemClock structure
struct SystemClock {
    int seconds;
    int nanoseconds;
};
int main(int argc, char *argv[]) {
    // Parse command line arguments for seconds and nanoseconds
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <seconds> <nanoseconds>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int seconds = atoi(argv[1]);
    int nanoseconds = atoi(argv[2]);

    // Attach to shared memory for the system clock (assuming you have the shared memory ID)
   #define SHM_KEY 1234 // You can choose any positive integer as the key
   #define SEM_KEY 123456
	// Specify the size of the shared memory segment
size_t shm_size = sizeof(struct SystemClock);

// Create or obtain the shared memory segment ID
int shm_id = shmget(SHM_KEY, shm_size, IPC_CREAT | 0666);
if (shm_id == -1) {
    perror("shmget");
    exit(EXIT_FAILURE);
}
    struct SystemClock *clock; // Assuming SystemClock is the shared memory struct

    // Attach to shared memory
    clock = (struct SystemClock *)shmat(shm_id, (void *)0, 0);
    if (clock == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

     sem_t clockSemaphore;
     if (sem_init(&clockSemaphore, 1, 1) == -1) { // Initialize with value 1 for mutual exclusion
    perror("sem_init");
    exit(EXIT_FAILURE);
} 	 

     // Calculate the termination time
    int terminationSeconds = clock->seconds + seconds;
    int terminationNanoseconds = clock->nanoseconds + nanoseconds;

    // Check if nanos	econds overflowed (greater than or equal to 1 billion)
    if (terminationNanoseconds >= 1000000000) {
        terminationSeconds++;               // Increment seconds
        terminationNanoseconds -= 1000000000; // Subtract one billion nanoseconds
    }

    // Output initial information
    printf("WORKER PID:%d PPID:%d SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d\n",
           getpid(), getppid(), clock->seconds, clock->nanoseconds, terminationSeconds, terminationNanoseconds);
    printf("--Just Starting\n");
while (1) {
    // Wait for access to the shared clock
if (sem_wait(&clockSemaphore) == -1) {
    perror("sem_wait");
    // Handle the error as needed (e.g., exit or continue)
}
 else {
        // Access the shared clock here (protected by the semaphore)
        int currentSeconds = clock->seconds;
        int currentNanoseconds = clock->nanoseconds;

        // Check if it's time to terminate
        if (currentSeconds > terminationSeconds ||
            (currentSeconds == terminationSeconds && currentNanoseconds >= terminationNanoseconds)) {
            printf("WORKER PID:%d PPID:%d SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d\n",
                getpid(), getppid(), currentSeconds, currentNanoseconds, terminationSeconds, terminationNanoseconds);
            printf("--Terminating\n");
            break;
        }

        // Periodic output
        static int previousSeconds = -1; // Initialize to an invalid value
        if (currentSeconds != previousSeconds) {
            int elapsedSeconds = currentSeconds - clock->seconds;
            printf("WORKER PID:%d PPID:%d SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d\n",
                getpid(), getppid(), currentSeconds, currentNanoseconds, terminationSeconds, terminationNanoseconds);
            printf("--%d seconds have passed since starting\n", elapsedSeconds);
            previousSeconds = currentSeconds;
        }

       if (sem_post(&clockSemaphore) == -1) {
    perror("sem_post");
    // Handle the error as needed (e.g., exit or continue)
}
    }
}
    sem_close(&clockSemaphore);
    // Detach from shared memory
    if (shmdt(clock) == -1) {
        perror("shmdt");
    }

    // Destroy the semaphore
    if (sem_destroy(&clockSemaphore) == -1) {
        perror("sem_destroy");
    }

    return 0;
}
