/** ****************************************************************
 * @file    aufgabe2/main.c
 * @author  Stefan Belic (Stefan.Belic@haw-hamburg.de)
 * @author  Philip Scheer (Philip.Scheer@haw-hamburg.de)
 * @version 1.0
 * @date    26.04.2017
 * @brief   Producer - Consumer System
 * Synchronisation und Thread Generierung mit
 * 1. Mutex und Semaphore
 * 2. Mutex und Conditional Variable
 ******************************************************************
 */
#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

//#define SEMAPHORE 1

#include "FIFO.h"


//Die laenge des string mit dem buchstaben
#define STRING_LAENGE 26
#define SLEEP_CONSUMER 2
#define SLEEP_PRODUCER 3

//Der mutex der fuer den FIFO Puffer
static pthread_mutex_t haupt_mutex = PTHREAD_MUTEX_INITIALIZER;

//Der mutex der denn producer 1 blockt
static pthread_mutex_t producer_1_m = PTHREAD_MUTEX_INITIALIZER;
//Der mutex der denn producer 2 blockt
static pthread_mutex_t producer_2_m = PTHREAD_MUTEX_INITIALIZER;
//Der mutex der denn consumer stopt
static pthread_mutex_t consumer_m = PTHREAD_MUTEX_INITIALIZER;

static int producer_1_isRunning;
static int producer_2_isRunning;
static int consumer_isRunning;

#ifdef SEMAPHORE
//Der semaphor fuer einfugen in den puffer
sem_t puffer_input;
//Der semaphore fuer rausnehmen aus den puffer
sem_t puffer_output;

#else
//Die conditional variable fuer einfuegen in den puffer
pthread_cond_t cond_p = PTHREAD_COND_INITIALIZER;
//Die conditional variable fuer rausnehmen aus den puffer
pthread_cond_t cond_c = PTHREAD_COND_INITIALIZER;

#endif

/**
 * Schreibt alle 3 Sekunden einen String in den FIFO Puffer
 */
void producerFunction(char*string, pthread_mutex_t* producer_m,int *producer_isRunning){

	int charZeiger = 0;// Momentane position im string

		while(*producer_isRunning){
			pthread_mutex_lock(producer_m);// check ob der producer geblockt ist
			pthread_mutex_unlock(producer_m);
			sleep(SLEEP_PRODUCER);

				#ifdef SEMAPHORE
					sem_wait(&puffer_input);// check ob der puffer frei ist

					pthread_mutex_lock(&haupt_mutex);
					FIFO_push(string[charZeiger]);
					pthread_mutex_unlock(&haupt_mutex);

					sem_post(&puffer_output);

				#else
					//Critical section anfang
					pthread_mutex_lock(&haupt_mutex);
					while(FIFO_getLength() == PUFFER_SIZE) pthread_cond_wait(&cond_p,&haupt_mutex); //Condition variable
					FIFO_push(string[charZeiger]);
					pthread_mutex_unlock(&haupt_mutex);
					//Critical section ende

					pthread_cond_signal(&cond_c);

				#endif

				charZeiger++;// Geht zur naechste position im String
				if(charZeiger==STRING_LAENGE){
					charZeiger=0;
				}
		}

}

/**
 * Schreibt alle 3 Sekunden ein kleinbuchstaben des Alphabets in den Puffer
 */
void *producer_1_f(void *a){
	char* string = "abcdefghijklmnopqrstuvwxyz";//Der string
	producerFunction(string,&producer_1_m,&producer_1_isRunning);
	return NULL;
}

/**
 * Schreibt alle 3 Sekunden einen großbuchstaben des Alphabets in den Puffer
 */
void *producer_2_f(void *a){
	char* string = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	producerFunction(string,&producer_2_m,&producer_2_isRunning);
	return NULL;
}

void *consumer_f(void *a){
	char var;// Variable die man als naechstes aus dem Puffer holen will

	while(consumer_isRunning){
				sleep(SLEEP_CONSUMER);
				pthread_mutex_lock(&consumer_m);// check ob der producer geblockt ist
				pthread_mutex_unlock(&consumer_m);
					#ifdef SEMAPHORE
						sem_wait(&puffer_output);//check ob der puffer leer ist

						pthread_mutex_lock(&haupt_mutex);
						var = FIFO_pop();
						pthread_mutex_unlock(&haupt_mutex);

						sem_post(&puffer_input);
					#else
						// Critical section anfang
						pthread_mutex_lock(&haupt_mutex);
						while(FIFO_getLength() == 0) pthread_cond_wait(&cond_c,&haupt_mutex);
						var = FIFO_pop();
						pthread_mutex_unlock(&haupt_mutex);
						// Critical section ende
						pthread_cond_signal(&cond_p);

					#endif
					printf("%c",var);
					fflush(stdout);

			}
	return NULL;
}

void *control_f(void *a){
	char var;
	int running = 1;
	int producer_1_isBlocked = 0;
	int producer_2_isBlocked = 0;
	int consumer_mutex_isBlocked = 0;

	while(running == 1){
		scanf("%c",&var);
		switch(var){
			case 'q': case 'Q':
				consumer_isRunning = 0;
				if(consumer_mutex_isBlocked){
					pthread_mutex_unlock(&consumer_m);
				}
				//pthread_cond_signal(&cond_c);

				producer_1_isRunning = 0;
				if(producer_1_isBlocked){
					pthread_mutex_unlock(&producer_1_m);
				}
				//pthread_cond_signal(&cond_p);

				producer_2_isRunning = 0;
				if(producer_2_isBlocked){
					pthread_mutex_unlock(&producer_2_m);
				}
				//pthread_cond_signal(&cond_p);

				return NULL;
				break;
			case 'h':
				printf("Mögliche Eingaben:\n");
				printf("-1: Starte / Stoppe Producer_1\n");
				printf("-2: Starte / Stoppe Producer_2\n");
				printf("-c/C: Starte / Stoppe Consumer\n");
				printf("-q/Q: Programm beenden\n");
				;break;
			case '1': // Starte / Stoppe Producer_1
				if(producer_1_isBlocked==1) {
					pthread_mutex_unlock(&producer_1_m);
					producer_1_isBlocked = 0;
				} else {
					pthread_mutex_lock(&producer_1_m);
					producer_1_isBlocked = 1;
				}
				break;
			case '2': // Starte / Stoppe Producer_2
				if(producer_2_isBlocked==1) {
					pthread_mutex_unlock(&producer_2_m);
					producer_2_isBlocked = 0;
				} else {
					pthread_mutex_lock(&producer_2_m);
					producer_2_isBlocked = 1;
				}
				break;
			case 'c': case 'C': // Starte / Stoppe Consumer
				if(consumer_mutex_isBlocked==1) {
					pthread_mutex_unlock(&consumer_m);
					consumer_mutex_isBlocked = 0;
				} else {
					pthread_mutex_lock(&consumer_m);
					consumer_mutex_isBlocked = 1;
				}
		}


	}
	return NULL;
}

int main(){
	printf("Programm ist gestartet worden\n");
	fflush(stdout);
	FIFO_init();

	producer_1_isRunning = 1;
	producer_2_isRunning = 1;
	consumer_isRunning = 1;

	#ifdef SEMAPHORE
		sem_init(&puffer_input, 0, PUFFER_SIZE);
		sem_init(&puffer_output, 0, 0);
	#endif

	pthread_t producer_1_t;
	pthread_t producer_2_t;
	pthread_t consumer_t;
	pthread_t control_t;

	// Startet alle Threads
	if(pthread_create(&producer_1_t,NULL,producer_1_f,NULL)){
		printf("Fehler thread koente nicht gestartet werden");
		fflush(stdout);
		return -1;
	}
	if(pthread_create(&producer_2_t,NULL,producer_2_f,NULL)){
		printf("Fehler thread koente nicht gestartet werden");
		fflush(stdout);
		pthread_cancel(producer_1_t);
		return -1;
	}
	if(pthread_create(&consumer_t,NULL,consumer_f,NULL)){
		printf("Fehler thread koente nicht gestartet werden");
		fflush(stdout);
		pthread_cancel(producer_1_t);
		pthread_cancel(producer_2_t);
		return -1;
	}
	if(pthread_create(&control_t,NULL,control_f,NULL)){
		printf("Fehler thread koente nicht gestartet werden");
		fflush(stdout);
		pthread_cancel(producer_1_t);
		pthread_cancel(producer_2_t);
		pthread_cancel(consumer_t);
		return -1;
	}


	//Joint alle threads
	if(pthread_join(control_t, NULL)) {
			printf("Fehler beim joinen des control threads");
			fflush(stdout);
			pthread_cancel(producer_1_t);
			pthread_cancel(producer_2_t);
			pthread_cancel(consumer_t);
			pthread_cancel(control_t);
			return -1;
		}

	pthread_cancel(consumer_t);
	if(pthread_join(consumer_t,NULL)){
		printf("Fehler beim joinen des consumer threads");
		fflush(stdout);
		return -1;
	}
	printf("\nConsumer thread exited");
	fflush(stdout);

	pthread_cancel(producer_1_t);
	if(pthread_join(producer_1_t,NULL)){
		printf("Fehler beim joinen des producer_1 threads");
		fflush(stdout);
		pthread_cancel(producer_2_t);
		pthread_cancel(consumer_t);
		return -1;
	}
	printf("\nProducer 1 thread exited");
	fflush(stdout);

	pthread_cancel(producer_2_t);
	if(pthread_join(producer_2_t,NULL)){
		printf("Fehler beim joinen des producer_2 threads");
		fflush(stdout);
		pthread_cancel(consumer_t);
		return -1;
	}
	printf("\nProducer 2 thread exited");
	fflush(stdout);

	#ifdef SEMAPHORE
		sem_destroy(&puffer_input);
		sem_destroy(&puffer_output);
	#else
		pthread_cond_destroy(&cond_c);
		pthread_cond_destroy(&cond_p);
	#endif

	pthread_mutex_destroy(&producer_1_m);
	pthread_mutex_destroy(&producer_2_m);
	pthread_mutex_destroy(&consumer_m);
	pthread_mutex_destroy(&haupt_mutex);

	printf("\nDas program wurde beendet");
	fflush(stdout);

	return 0;
}



