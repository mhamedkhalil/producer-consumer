#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <fcntl.h> //include libraries to create and manipulate shared memory 
#include <semaphore.h>
using namespace std;

sem_t *semaphore; //declare a semaphore

void producer(int *sharedMemory, int size)
{
    //producer waits for availability before accessing the critical section
    sem_wait(semaphore);
    for(int i = 0; i < size; i++)
    {
        sharedMemory[i] = 0; //initialize an empty shared memory (all elemets = 0)
    }
    sem_post(semaphore);
    int ptr = 0;
    while(1)
    {
        sem_wait(semaphore); //wait before entering the critical section
        if (ptr < size) //check for space availability 
        {
            sharedMemory[ptr] = ptr+1; //when there is available space producer generates integers
            ptr++;
            cout << "Produced " << ptr << endl;
        }
        sem_post(semaphore); //send a signal after leaving the critical section
        if(ptr >= size) //when no more space to generate objects break out of the loop 
        { break; } 
    }
}

int main()
{
    const char *name = "/sharedMemory"; //a unique shared memory name
    int size = sizeof(int) * 10; //shared memory can contain 10 integers 
    int reference = shm_open(name, O_CREAT | O_RDWR, 0666); //create a new memory or open an existing one
    // O_CREAT flag allows creating the memory if it does not exist
    // O_RDWR flag opens shared memory for reading and writing 
    // and 0666 mode gives every user (process) the permission to read and write from the memory
    if (reference == -1)
    {
        cerr << "Error creating shared memory by producer" << endl;
        return 1;
    }
    //Now resize the shared memory
    if (ftruncate(reference, size) == -1)
    {
        cerr << "Error resizing shared memory segment by producer" << endl;
        return 1;
    }
    //Now to treat the shared memory as an array for easier access and modification
    //we need to map the shared memory into the process memory
    void *ptr;
    ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, reference, 0);
    if (ptr == MAP_FAILED)
    {
        cerr << "Error mapping shared memory by producer" << endl;
        return 1;
    }
    int *shared_array = (int *)ptr; //for easier acess and indices 
    // Initialize the semaphore to 1 (for mutual exclusion)
    
    //create and initialize the semaphore to 1 for mutual exclusion 
    semaphore = sem_open("/mutex",O_CREAT,0644,1); 
    if(semaphore == SEM_FAILED)
    {
        cerr << "Error opening semaphore by producer" << endl;
        return 1;
    }
    // Call producer function with correct parameters
    producer(shared_array, 10);
    //after finishing unmap the shared memory from the process memory to be removed later by the OS
    munmap(ptr, size);
    sem_unlink("/mutex");
    cout << "Producer Done" << endl;
    return 0;
}