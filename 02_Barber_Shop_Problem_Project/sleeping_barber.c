/*
    Programmer : Sunil Sutar
    Subject    : Linux System Programming
    Project    : Solution to Sleeping Barber Problem using Multi-threading, Semaphore and Mutex
*/

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

//Maximum Number of Customers
#define MAX_CUSTOMERS 5

//Function Prototypes
void* customer(void *num);
void* barber(void* );

void randwait(int secs);

//Define Semaphores

//Waiting Room limits 5 number of customers to allowed to enter waiting room at a time
//As here Shared pool of resources hence I am using Counting Semaphore here
sem_t waitingRoom;

//barberChair ensure the mutually exlusive access to baber chair
//Here Mutex is Used
sem_t barberChair;

//barberSleep is used to allow the barber to sleep until customer arrives
sem_t sleepBarber;

//customerWait is used to make the customer to wait until the barber is done cutting hair
sem_t waitCustomer;

//Flag to stop the barber thread when all customers have been serviced
int allDone = 0;


int main(int argc, char* argv[])
{
    pthread_t btid;                     //for barber thread
    pthread_t ctid[MAX_CUSTOMERS];      //for customer thread

    long RandSeed;
    int i, numCustomers, numChairs;
    int Number[MAX_CUSTOMERS];

    //Check Command Line Argument
    if(argc != 4)
    {
        printf("Use : %s <Num Customers> <Num Chairs> <randseed>\n", argv[0]);
        exit(-1);
    }

    //Get the cmd line args and convert them into integers
    numCustomers = atoi(argv[1]);
    numChairs = atoi(argv[2]);
    RandSeed = atoi(argv[3]);
    
    //Make sure number of custmer less tham MAX_CUSTOMERS
    if(numCustomers > MAX_CUSTOMERS)
    {
        printf("Maximum Number of Customers is : %d\n", MAX_CUSTOMERS);
        exit(-1);
    }

    printf("*************  SleepingBarber  **********************\n");
    printf("A Solution to Sleeping Barber Problem using Semaphore\n");
    printf("=====================================================\n");

    //Initialize Random Number generator with a new seed
    //srand48(RandSeed);

    //Initialize the Numbers array
    for(int i=0; i<MAX_CUSTOMERS; i++)
    {
        Number[i] = i;
    }

    //Initialize the Semaphore with Initial Values
    sem_init(&waitingRoom, 0, numChairs);   //Counting Semaphore
    sem_init(&barberChair, 0, 1);           //Mutex
    sem_init(&sleepBarber, 0, 0);           
    sem_init(&waitCustomer, 0, 0);

    //Create a Barber THread
    pthread_create(&btid, NULL, barber, NULL);

    //Create the Custoner Threads
    for(int i=1; i<=numCustomers; i++)
    {
        pthread_create(&ctid[i], NULL, customer, (void *)&Number[i]);
    }

    //Join each of the thread to wwait for them to finish 
    for(int i=1; i<=numCustomers; i++)
    {
        pthread_join(ctid[i], NULL);
    }

    //When all of the customers are finished kill the barber thread
    allDone = 1;
    sem_post(&waitCustomer);
    pthread_join(btid, NULL);

}

void* customer(void *number)
{
    int num = *(int *) number;
    
    //Leave for the shop and take some random amount of time to arrive
    printf("Customer %d is leaving for barber shop\n", num);
    randwait(5);
    printf("Customer %d is arrived at barber shop\n", num);

    //Wait for space to open up in the waiting room
    sem_wait(&waitingRoom);
    printf("Customer %d is entering in the waiting room.\n", num);

    //Wait for barber shop to become a free
    sem_wait(&barberChair);

    //The chair is free next customer in waiting room get chance
    sem_post(&waitingRoom);

    //Wakeup the Barber if barber is sleeping
    sem_post(&sleepBarber);
    printf("Barber Wakes up\n");

    //Wait for the barber to finish the cutting hair
    sem_wait(&waitCustomer);

    //Give up the Chair
    sem_post(&barberChair);
    printf("Customer %d is Leaving Barber Shop\n", num);
}

void* barber(void *parm)
{
    while(!allDone)
    {
        //Baber sleep until someone arrives and wake up
        printf("Barber is Sleeping\n");
        sem_wait(&sleepBarber);
        
        if(!allDone)
        {
            //Barber take random ammount of time to cut hair
            printf("The Barber is cutting hair\n");
            randwait(3);
            //sleep(1);
            printf("Barber has finished cutting hair\n");

            //Release the customer when done cutting
            sem_post(&waitCustomer);
        }
        else
        {
            printf("The Barber is going home for the day\n");
        }
    }
}


void randwait(int secs)
{
    int len;

    //Generate Randome Number
    len = (int) ((drand48() * secs) + 1);
    sleep(len);
}

