#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> //include libraries to create and manipulate shared memory 
#include <semaphore.h>
using namespace std;

sem_t *semaphore;
void consumer(int *sharedMemory, int size)
{
    int ptr = 0;
    while(1)
    {
        sem_wait(semaphore); //consumer waits for availability before accessing critical section
        if(sharedMemory[ptr] != 0) //check if there is an object to consume
        {
            cout << "Consumed " << sharedMemory[ptr] << endl;
            sharedMemory[ptr] = 0; //when consumed, integer is set to 0
            ptr++;
        }
        else
        {
            cout << "No item to consume yet" << endl; //print a message when no available objects for consumption
        }
        sem_post(semaphore); //signal before exiting the critical section
        if(ptr >= size) //when finished break the loop
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
    int *shared_array = (int *)ptr;

    semaphore = sem_open("/mutex",O_CREAT,0644,1);
    if(semaphore == SEM_FAILED)
    {
        cerr << "Error opening semaphore by consumer" << endl;
        return 1;
    }
    // Call consumer function with correct parameters
    consumer(shared_array, 10);
    munmap(ptr, size); //consumer unmaps shared memory from process memory when finished
    sem_unlink("/mutex");
    sem_close(semaphore); //consumer is responsible for closing the semaphore after finishing as it will finish last
    cout << "Consumer done" << endl;
    return 0;
}