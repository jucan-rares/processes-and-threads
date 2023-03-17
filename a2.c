#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <stdlib.h>
#include <pthread.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <fcntl.h>

sem_t sem1, sem2, *sem3, *sem4, sem5;

void unlock(sem_t *sem){ //decrement

    sem_wait(sem);
}
void lock(sem_t *sem){ //increment

    sem_post(sem);
}

void* function_for_P7(void *arg){
    
    int id = *(int*) arg;

    //2_start -> 3_start -> 3_end -> 2_end

    if(id == 2){

        info(BEGIN, 7, 2);
        lock(&sem2);
        unlock(&sem1);
        info(END, 7, 2);
    }
    else{
        if(id == 3){

            unlock(&sem2);
            info(BEGIN, 7, 3);
            info(END, 7, 3);
            lock(&sem1);
        }   
        else{
            if(id == 1){
   
                sem_wait(sem3);
                info(BEGIN, 7, 1);
                info(END, 7, 1);
                sem_post(sem4);
            }
            else{

                info(BEGIN, 7, id);
                info(END, 7, id);
            }
        }
    }   
    return NULL;
}
void* function_for_P6(void *arg){
    
    int id = *(int*) arg;

    unlock(&sem5); //with this sempahore we ensure that only up to 5 threads are allowed in the critical zone

    info(BEGIN, 6, id);
    info(END, 6, id);

    lock(&sem5);

    return NULL;
}
void* function_for_P5(void *arg){
    
    int id = *(int*) arg;

    //5_1_end -> 7_1_start -> 7_1_end -> 5_4_start

    if(id == 1){

        info(BEGIN, 5, 1);
        info(END, 5, 1);
        sem_post(sem3); //we lock the first semaphor
    }
     else{
        if(id == 4){

            sem_wait(sem4); //we unlock the second semaphor
            info(BEGIN, 5, 4);
            info(END, 5, 4);
        }    
        else{

            info(BEGIN, 5, id);
            info(END, 5, id);
        }
    }
    return NULL;
}

void P7(){

    info(BEGIN, 7, 0);

    pthread_t tid[4]; //we will have 4 threads
    int id[4];

    //we initialize the semaphores
    if (sem_init(&sem1, 0, 0) < 0)
        exit(1);
    if (sem_init(&sem2, 0, 0) < 0)
        exit(1);
    sem3 = sem_open("/SEMAPHORE3", O_CREAT, 0777, 0);
    if (sem3 == SEM_FAILED) 
        exit(1);
    sem4 = sem_open("/SEMAPHORE4", O_CREAT, 0777, 0);
    if (sem4 == SEM_FAILED) 
        exit(1);

    for(int i=0; i<4; i++){ //we create 4 threads

        id[i]=i+1;
        if(pthread_create(&tid[i], NULL, function_for_P7, &id[i]) != 0)
            exit(1);
    }

    for(int i=0; i<4; i++) //with this we basically tell the main thread to wait for the rest of the threads to finish so he cant end before them
        if(pthread_join(tid[i], NULL) != 0)
            exit(1);

    info(END, 7, 0);
}
void P6(){

    info(BEGIN, 6, 0);
    
    pthread_t tid[42]; //we will have 42 threads
    int id[42];

    //we initialize the semaphore
    if (sem_init(&sem5, 0, 5) < 0)
        exit(1);

    for(int i=0; i<42; i++){ //we create 42 threads

        id[i]=i+1;
        if(pthread_create(&tid[i], NULL, function_for_P6, &id[i]) != 0)
            exit(1);
    }

    for(int i=0; i<42; i++) //with this we basically tell the main thread to wait for the rest of the threads to finish so he cant end before them
        if(pthread_join(tid[i], NULL) != 0)
            exit(1);

    info(END, 6, 0);
}
void P5(){ 

    info(BEGIN, 5, 0);
    
    pthread_t tid[5]; //we will have 5 threads
    int id[5];

    //we initialize the semaphores
    sem3 = sem_open("/SEMAPHORE3", O_CREAT, 0777, 0);
    if (sem3 == SEM_FAILED) 
        exit(1);
    sem4 = sem_open("/SEMAPHORE4", O_CREAT, 0777, 0);
    if (sem4 == SEM_FAILED) 
        exit(1);

    for(int i=0; i<5; i++){ //we create 5 threads

        id[i]=i+1;
        if(pthread_create(&tid[i], NULL, function_for_P5, &id[i]) != 0)
            exit(1);
    }

    for(int i=0; i<5; i++) //with this we basically tell the main thread to wait for the rest of the threads to finish so he cant end before them
        if(pthread_join(tid[i], NULL) != 0)
            exit(1);

    info(END, 5, 0);
}

//P3 and P4 function the same way
void P4(){

    info(BEGIN, 4, 0);
    
    int pid5;

    pid5 = fork();
    switch(pid5){

        case -1: //error case
            exit(1); 

        case 0: //child
            P5();
            break;

        default: //parent
            waitpid(pid5, NULL, 0); //wait for the child to end
            info(END, 4, 0);
    }
}
void P3(){

    info(BEGIN, 3, 0);
    
    int pid6;

    pid6 = fork();
    switch(pid6){

        case -1: //error case
            exit(1); 

        case 0: //child
            P6();
            break;

        default: //parent
            waitpid(pid6, NULL, 0); //wait for the child to end
            info(END, 3, 0);
    }
}
//-------------------------------

void P2(){ //P1 and P2 function the same way

    info(BEGIN, 2, 0);

    int pid3, pid4;

    pid3 = fork();
    switch(pid3){

        case -1: //error case
            exit(1); 

        case 0: //child
            P3();
            break;

        default: //parent
            pid4 = fork();
            switch(pid4){

                case -1: //error case
                    exit(1); 

                case 0: //child
                    P4();
                    break;

                default: //parent
                    //wait for the children to end
                    waitpid(pid3, NULL, 0);
                    waitpid(pid4, NULL, 0);

                    info(END, 2, 0);
            }         
    }
}

int main(){

    init();
    
    info(BEGIN, 1, 0);

    int pid2, pid7;

    pid2 = fork();
    switch(pid2){

        case -1: //error case
            exit(1); 

        case 0: //child
            P2();
            break;

        default: //parent
            pid7 = fork();
            switch(pid7){

                case -1: //error case
                    exit(1); 
          
                case 0: //child
                    P7();
                    break;

                default: //parent
                    //wait for the children to end
                    waitpid(pid2, NULL, 0);
                    waitpid(pid7, NULL, 0);

                    //we need to destroy all the semaphores we used and we are going to do it here because theoretically this is where the whole application ends
                    sem_destroy(&sem1);
                    sem_destroy(&sem2);
                    sem_unlink("SEMAPHORE3");
                    sem_unlink("SEMAPHORE4");
                    sem_destroy(&sem5);

                    info(END, 1, 0);
            }         
    }

    return 0;
}
