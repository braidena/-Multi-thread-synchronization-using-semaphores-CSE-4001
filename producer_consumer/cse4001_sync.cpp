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
Lightswitch writeSwitch;
Semaphore noReaders(1);
Semaphore noWriters(1);


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
        sleep(2);
        turnstile.wait();
        roomEmpty.wait();
        printf("Writer %d: Writing. \n",x);
        fflush(stdout);
        turnstile.signal();
        roomEmpty.signal();
    }

}

/*
    Reader function 
*/
void *Reader2 ( void *threadID )
{
    // Thread number 
    int x = (long)threadID;

    while( 1 )
    {   
        sleep(2);
        noReaders.wait();
        readSwitch.lock(noWriters);
        noReaders.signal();
        printf("Reader %d: Reading. \n", x);
        fflush(stdout);
        readSwitch.unlock(noWriters);
        
    }

}


/*
    Writer function 
*/
void *Writer2 ( void *threadID )
{
    // Thread number 
    int x = (long)threadID;
    
    while( 1 )
    {
        sleep(2);
        writeSwitch.lock(noReaders);
        noWriters.wait();
        printf("Writer %d: Writing. \n",x);
        fflush(stdout);
        noWriters.signal();
        writeSwitch.unlock(noReaders);
    }

}


// philo section
Semaphore forks[5] = { Semaphore(1), Semaphore(1), Semaphore(1), Semaphore(1), Semaphore(1) };
Semaphore footman(4);

int left(int i) {
    return i;
}
int right(int i) {
    return (i+1)%5;
}

void think1(int x) {
    printf("Philosopher %d: Thinking. \n",x);
    return;
}
void get_forks1(int x) {
    footman.wait();
    forks[right(x)].wait();
    forks[left(x)].wait();
    return;
}
void eat1(int x) {
    printf("Philosopher %d: Eating. \n",x);
    return;
}
void put_forks1(int x) {
    forks[right(x)].signal();
    forks[left(x)].signal();
    footman.signal();
    return;
}
void get_forks2(int x) {
    if (x % 2 == 0) {
        forks[left(x)].wait();
        forks[right(x)].wait();
    } else {
        forks[right(x)].wait();
        forks[left(x)].wait();
    };
    return;
}
void put_forks2(int x) {
    forks[right(x)].signal();
    forks[left(x)].signal();
    return;
}

void *Philosopher1 ( void *threadID) {
    int x = (long) threadID;
    while (1) {
        think1(x);
        sleep(1);
        get_forks1(x);
        eat1(x);
        sleep(1);
        put_forks1(x);
    }
}
void *Philosopher2 ( void *threadID) {
    int x = (long) threadID;
    while (1) {
        think1(x);
        sleep(1);
        get_forks2(x);
        eat1(x);
        sleep(1);
        put_forks2(x);
    }
}


int main(int argc, char **argv )
{
    
    if (argc > 2) {
        printf("Too many arguments.");
        exit(-1);
    }
    int probNum = 0;
    try {
        probNum = std::stoi(argv[1]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error converting argument into integer: " << e.what() << std::endl;
        exit(-1);
    }

    pthread_t readerThread[ numReaders ];
    pthread_t writerThread[ numWriters ];

    if (probNum == 1) {
        // Create the producers 
        for( long p = 0; p < numReaders; p++ )
        {
            int rc = pthread_create ( &readerThread[ p ], NULL, 
                                    Writer, (void *) (p+1) );
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
                                    Reader, (void *) (c+1) );
            if (rc) {
                printf("ERROR creating writer thread # %d; \
                        return code from pthread_create() is %d\n", c, rc);
                exit(-1);
            }
        }
    }
    else if (probNum == 2) {
        // Create the producers 
        for( long p = 0; p < numReaders; p++ )
        {
            int rc = pthread_create ( &readerThread[ p ], NULL, 
                                    Reader2, (void *) (p+1) );
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
                                    Writer2, (void *) (c+1) );
            if (rc) {
                printf("ERROR creating writer thread # %d; \
                        return code from pthread_create() is %d\n", c, rc);
                exit(-1);
            }
        }
    }
    else if (probNum == 3) {
        // Create the philosophers
        for( long p = 0; p < numReaders; p++ )
        {
            int rc = pthread_create ( &readerThread[ p ], NULL, 
                                    Philosopher1, (void *) (p+1) );
            if (rc) {
                printf("ERROR creating reader thread # %d; \
                        return code from pthread_create() is %d\n", p, rc);
                exit(-1);
            }
        }
    }
    else if (probNum == 4) {
        // Create the philosophers
        for( long p = 0; p < numReaders; p++ )
        {
            int rc = pthread_create ( &readerThread[ p ], NULL, 
                                    Philosopher2, (void *) (p+1) );
            if (rc) {
                printf("ERROR creating reader thread # %d; \
                        return code from pthread_create() is %d\n", p, rc);
                exit(-1);
            }
        }
    }
    else {
        printf("Number must be between 1 and 4.");
        exit(-1);
    }


    // To allow other threads to continue execution, the main thread 
    // should terminate by calling pthread_exit() rather than exit(3). 
    pthread_exit(NULL); 


} /* main() */







