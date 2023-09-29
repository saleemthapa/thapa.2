#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>  // Include for semaphores
#include <sys/ipc.h>  // Include for semaphores
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <getopt.h>
#include <semaphore.h>
#include <fcntl.h>
#define MAX_CHILDREN 20 // Define MAX_CHILDREN here
#define SEM_KEY 12345
// Global variable to track whether it's time to terminate
volatile sig_atomic_t timeToTerminate = 0;
volatile sig_atomic_t signalReceived = 0;
//struct SystemClock *sharedClock;
// Signal handler for the alarm signal
void alarmHandler() {
    // Set the flag to indicate it's time to terminate
    timeToTerminate = 1;
}
// Signal handler for SIGINT
void sigintHandler() {
    signalReceived = 1;
}
int isChildProcessTerminated(pid_t pid) {
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);
    
    if (result == -1) {
        perror("waitpid");
        // Handle the error as needed
        return -1; // Indicates an error
    } else if (result == 0) {
        // The child process is still running
        return 0;
    } else {
        // The child process has terminated
        if (WIFEXITED(status)) {
            // Child exited normally
            int exit_status = WEXITSTATUS(status);
            printf("Child process %d exited with status %d\n", (int)pid, exit_status);
        } else if (WIFSIGNALED(status)) {
            // Child was terminated by a signal
            int term_signal = WTERMSIG(status);
            printf("Child process %d terminated by signal %d\n", (int)pid, term_signal);
        }
        return 1; // Indicates that the child has terminated
    }
}
int isTimeToTerminate(void) {
    static time_t start_time = 0; // Static variable to store the start time
    time_t current_time;
    
    if (start_time == 0) {
        // Initialize the start time on the first call to this function
        start_time = time(NULL);
    }
    
    current_time = time(NULL);
    
    // Calculate the elapsed time in seconds
    int elapsed_time_in_seconds = (int)(current_time - start_time);
    
    // Implement your logic here to decide when it's time to terminate
    // For example, terminate after 60 seconds
    if (elapsed_time_in_seconds >= 60) {
        return 1; // It's time to terminate
    } else {
        return 0; // Not yet time to terminate
    }
}

/*// Function to terminate child processes
void terminateChildProcesses(struct PCB *processTable) {
    // Implement the logic to terminate child processes and perform cleanup
    // For example, you can use kill() to send signals to child processes
    // and wait for them to exit
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (processTable[i].occupied) {
            pid_t child_pid = processTable[i].pid;
            if (kill(child_pid, SIGTERM) == -1) {
                perror("kill");
                // Handle the error as needed
            }
        }
    }

    // Optionally, wait for child processes to exit if needed
    int status;
    while (wait(&status) > 0) {
        // Process exited
    }
}
*/

// Define the PCB (Process Control Block) structure
struct PCB {
    int occupied;      // Indicates if the entry is in use (1) or not (0)
    pid_t pid;         // Process ID of the child
    int startSeconds;  // Time when it was forked (seconds)
    int startNano;     // Time when it was forked (nanoseconds)
};
//struct PCB processTable[MAX_CHILDREN];
// Define the SystemClock structure
struct SystemClock {
    int seconds;       // Seconds component of the system clock
    int nanoseconds;   // Nanoseconds component of the system clock
};
struct PCB processTable[MAX_CHILDREN];

// Function to terminate child processes
void terminateChildProcesses(struct PCB *processTable) {
    // Implement the logic to terminate child processes and perform cleanup
    // For example, you can use kill() to send signals to child processes
    // and wait for them to exit
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (processTable[i].occupied) {
            pid_t child_pid = processTable[i].pid;
            if (kill(child_pid, SIGTERM) == -1) {
                perror("kill");
                // Handle the error as needed
            }
        }
    }

    // Optionally, wait for child processes to exit if needed
    int status;
    while (wait(&status) > 0) {
        // Process exited
    }
}


/*
// Function to create and initialize shared memory for the system clock and process table
int createSharedMemory() {
    int shm_id;  // Shared memory ID

    // Create shared memory for the system clock
    shm_id = shmget(IPC_PRIVATE, sizeof(struct SystemClock), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach to shared memory for the system clock
    struct SystemClock *clock = (struct SystemClock *)shmat(shm_id, (void *)0, 0);
    if (clock == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Initialize the system clock (set initial values as needed)
    clock->seconds = 0;
    clock->nanoseconds = 0;

    // Create shared memory for the process table
	    shm_id = shmget(IPC_PRIVATE, sizeof(struct PCB) * MAX_CHILDREN, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach to shared memory for the process table
    struct PCB *processTable = (struct PCB *)shmat(shm_id, (void *)0, 0);
    if (processTable == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE*/
// Function to terminate child processes
struct SystemClock* createSharedMemory() {
    int shm_id;  // Shared memory ID
    // Create shared memory for the system clock
    shm_id = shmget(IPC_PRIVATE, sizeof(struct SystemClock), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach to shared memory for the system clock
    struct SystemClock *clock = (struct SystemClock *)shmat(shm_id, (void *)0, 0);
    if (clock == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Initialize the system clock (set initial values as needed)
    clock->seconds = 0;
    clock->nanoseconds = 0;

    return clock;  // Return a pointer to the shared clock structure
}
void cleanup(struct SystemClock *clock) {
    // Detach from shared memory
    if (shmdt((const void *)clock) == -1) {
        perror("shmdt");
    } else {
        printf("Detached from shared memory successfully.\n");
    }
}
/*void cleanup(struct SystemClock *clock, int shm_id) {
    // Detach from shared memory or perform cleanup
    if (shmdt((const void *)clock) == -1) {
        perror("shmdt");
    } else {
        printf("Detached from shared memory successfully.\n");
    }

      // Remove shared memory
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    } else {
        printf("Removed shared memory successfully.\n");
    }
}*/
int createSemaphores();
// Function declarations
int isTimeToTerminate(void);
void terminateChildProcesses(struct PCB *processTable);
int isChildProcessTerminated(pid_t pid);
int main(int argc, char *argv[]) {
    // Parse command line arguments for -n, -s, -t, and other options
    int opt;
    //int numProcesses = 0;
    //int simul = 0;
    //int timeLimit = 0;
    while ((opt = getopt(argc, argv, "hn:s:t:")) != -1) {
        switch (opt) {
            case 'h':
                printf("Usage: %s [-h] [-n proc] [-s simul] [-t timelimit]\n", argv[0]);
                printf("Options:\n");
                printf("  -h: Display this help message\n");
                printf("  -n proc: Number of child processes to launch\n");
                printf("  -s simul: Maximum number of child processes running simultaneously\n");
                printf("  -t timelimit: Bound of time for child process execution\n");
                exit(EXIT_SUCCESS);
                break;
            case 'n':
		//numProcesses = atoi(optarg);
                break;
            case 's':
                //simul = atoi(optarg);
                break;
            case 't':
                //timeLimit = atoi(optarg);
                break;
            default:
         	fprintf(stderr, "Invalid option or missing argument. Use -h for help.\n");
    		exit(EXIT_FAILURE);
                break;
        }
    }

    // Create and initialize shared memory for the system clock and process table
//struct SystemClock *sharedClock = createSharedMemory();
struct SystemClock *sharedClock ;
sharedClock = createSharedMemory();
/*
// Initialize semaphores for synchronization (if using semaphores)
sem_t clockSemaphore;
//int sem_id = createSemaphores();
// Initialize clockSemaphore
if (sem_init(&clockSemaphore, 0, 1) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
}
*/
//little modification
// Initialize semaphores for synchronization (if using semaphores)

/*sem_t clockSemaphore;
//int sem_id = createSemaphores();
// Initialize clockSemaphore
if (sem_init(&clockSemaphore, 1, 1) == -1) {
    perror("sem_init");
    exit(EXIT_FAILURE);
}
*/
   sem_t clockSemaphore;
   if (sem_init(&clockSemaphore, 1, 1) == -1) { // Initialize with value 1 for mutual exclusion
    perror("sem_init");
    exit(EXIT_FAILURE);
}

    // Initialize variables for managing child processes
    //int childCount = 0;
    bool stillChildrenToLaunch = true;

    while (stillChildrenToLaunch) {
       // Increment the simulated system clock
    sem_wait(&clockSemaphore); // Wait for access to the shared clock

    // Access the shared clock and update its values
    sharedClock->nanoseconds += 1000; // Increment nanoseconds (e.g., by 1000 nanoseconds)
    
    // Check if nanoseconds overflowed (greater than or equal to 1 billion)
    if (sharedClock->nanoseconds >= 1000000000) {
        sharedClock->seconds++;             // Increment seconds
        sharedClock->nanoseconds -= 1000000000; // Subtract one billion nanoseconds
    }

    sem_post(&clockSemaphore); // Release access to the shared clock
    signal(SIGALRM, alarmHandler);
    alarm(60);
    // Set up signal handler to call cleanup on termination
    //signal(SIGINT, cleanup);
// Set up signal handler for SIGINT
    signal(SIGINT, sigintHandler);
    while (stillChildrenToLaunch && !timeToTerminate) {
        // Check if any child processes have terminated using non-blocking waitpid
	 for (int i = 0; i < MAX_CHILDREN; i++) {
        if (processTable[i].occupied) {
            // Check if the process has terminated
            if (isChildProcessTerminated(processTable[i].pid)) {
	                // Update the process table for the terminated process
                processTable[i].occupied = 0;
                // Handle any other necessary updates
            }
        }
    }

        // Check if it's time to terminate the program
        if (isTimeToTerminate() || timeToTerminate || signalReceived) {
            // Terminate all child processes gracefully
            terminateChildProcesses(processTable);
            stillChildrenToLaunch = false; // Exit the loop
        }



    if (sharedClock->nanoseconds >= 1000000000) {
    sharedClock->seconds++;             // Increment seconds
    sharedClock->nanoseconds -= 1000000000; // Subtract one billion nanoseconds
}



// Display the process table every half a second
    if (sharedClock->seconds > 0 && sharedClock->nanoseconds >= 500000000) {
        printf("OSS PID:%d SysClockS: %d SysClockNano: %d\n", getpid(), sharedClock->seconds, sharedClock->nanoseconds);
        printf("Process Table:\n");
        printf("Entry Occupied PID StartS StartN\n");
        
        for (int i = 0; i < MAX_CHILDREN; i++) {
            if (processTable[i].occupied) {
                printf("%d %d %d %d %d \n", i, processTable[i].occupied, processTable[i].pid, processTable[i].startSeconds,
processTable[i].startNano);
            } else {
                printf("%d %d 0 0 0\n", i, processTable[i].occupied);
            }
        }

        printf("\n");
        sharedClock->nanoseconds = 0;
    }


    }//while child loop close



	}//while loop close
    // Clean up shared memory, semaphores, and other resources
/*int shm_id;
shm_id =  createSharedMemory(); // Assign the result to sharedClock
sharedClock = (struct SystemClock *)shmat(shm_id, (void *)0, 0);
    if (sharedClock == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    // Cleanup and termination section
    if (shmdt(sharedClock) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    // Detach from shared memory
    cleanup(sharedClock, shm_id);*/
  cleanup(sharedClock);
 // sem_destroy(&clockSemaphore); 
  // Clean up the semaphore
// Cleanup and close the semaphore when done
/*if (sem_close(&clockSemaphore) == -1) {
    perror("sem_close");
}
*/
return 0;

}
