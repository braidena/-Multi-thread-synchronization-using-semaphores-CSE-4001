//
// Example from: http://www.amparo.net/ce155/sem-ex.c
//
// Adapted using some code from Downey's book on semaphores
//
// Compilation:
//
//       g++ main.cpp -lpthread -o main -lm
// or 
//      make
//

#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <semaphore.h>  /* Semaphore */
#include <iostream>
using namespace std;

/*
 This wrapper class for semaphore.h functions is from:
 http://stackoverflow.com/questions/2899604/using-sem-t-in-a-qt-project
 */
class Semaphore {
public:
    // Constructor
    Semaphore(int initialValue)
    {
        sem_init(&mSemaphore, 0, initialValue);
    }
    // Destructor
    ~Semaphore()
    {
        sem_destroy(&mSemaphore); /* destroy semaphore */
    }
    
    // wait
    void wait()
    {
        sem_wait(&mSemaphore);
    }
    // signal
    void signal()
    {
        sem_post(&mSemaphore);
    }

    
private:
    sem_t mSemaphore;
};

class Lightswitch {
    public:
    int counter = 0;
    Semaphore Mutex = Semaphore(1);

    void lock(Semaphore sema) {
        Mutex.wait();
        counter += 1;
        if (counter ==1) {
            sema.wait();
        }
        Mutex.signal();
    }

    void unlock(Semaphore sema) {
        Mutex.wait();
        counter -= 1;
        if (counter == 0) {
            sema.signal();
        }
        Mutex.signal();
    }
    
};


/* global vars */
int readers = 0;
const int bufferSize = 5;
const int numReaders = 5; 
const int numWriters = 5; 

/* semaphores are declared global so they can be accessed
 in main() and in thread routine. */

Semaphore turnstile(1);
Semaphore roomEmpty(1);
Lightswitch readSwitch;


/*
    Reader function 
*/
void *Reader ( void *threadID )
{
    // Thread number 
    int x = (long)threadID;

    while( 1 )
    {   
        sleep(2);
        turnstile.wait();
        turnstile.signal();
        readSwitch.lock(roomEmpty);
        printf("Reader %d: Reading. \n", x);
        fflush(stdout);
        readSwitch.unlock(roomEmpty);
        
    }

}

/*
    Writer function 
*/
void *Writer ( void *threadID )
{
    // Thread number 
    int x = (long)threadID;
    
    while( 1 )
    {
        turnstile.wait();
        roomEmpty.wait();
        printf("Writer %d: Writing. \n",x);
        fflush(stdout);
        turnstile.signal();
        roomEmpty.signal();
        sleep(2);
    }

}


int main(int argc, char **argv )
{
    pthread_t readerThread[ numReaders ];
    pthread_t writerThread[ numWriters ];

    // Create the producers 
    for( long p = 0; p < numReaders; p++ )
    {
        int rc = pthread_create ( &readerThread[ p ], NULL, 
                                  Reader, (void *) (p+1) );
        if (rc) {
            printf("ERROR creating reader thread # %d; \
                    return code from pthread_create() is %d\n", p, rc);
            exit(-1);
        }
    }

    // Create the consumers 
    for( long c = 0; c < numWriters; c++ )
    {
        int rc = pthread_create ( &writerThread[ c ], NULL, 
                                  Writer, (void *) (c+1) );
        if (rc) {
            printf("ERROR creating writer thread # %d; \
                    return code from pthread_create() is %d\n", c, rc);
            exit(-1);
        }
    }

    printf("Main: program completed. Exiting.\n");


    // To allow other threads to continue execution, the main thread 
    // should terminate by calling pthread_exit() rather than exit(3). 
    pthread_exit(NULL); 


} /* main() */







