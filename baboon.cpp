/*
 * baboon.cpp
 *
 *  Created on: Feb 16, 2011
 *      Author: Paul
 */

/*** Eastbound and westbound functions were written	***
 *** using a modified version of the readers and	***
 *** writers solution (Fig. 2-47, p. 168). 			***/			// <- Side note, I am talking about Modern Operating Systems 3rd Edition
																	// by Andrew S. Tanenbaum

#include <semaphore.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <pthread.h>

#define CROSSING_TIME 1										// number of seconds to cross rope
#define AVERAGE_CREATION_TIME 2								// average (mean) number of seconds between baboons' arrivals at rope
#define BABOONS_TO_CROSS 30									// number of baboons that will need to cross rope during simulation

sem_t crossers_mut;											// semaphore - controls access to crossers
sem_t rope_mut;												// semaphore - controls eastbound access to rope
sem_t write_mut;											// semaphore - controls access to std::cout
int crossers = 0;											// number of eastbound baboons (negative indicates number of westbound baboons)

void cross_rope(int crosser){
	std::string direction;
	if (crosser > 0)
		direction = "Eastbound";
	else{
		direction = "Westbound";
		crosser = -crosser;									// flip sign for output
	}
	sem_wait(&write_mut);									// get access to std::cout
	std::cout << direction << " baboon crossing the rope...\t" << crosser << " " << direction << " baboon(s) are currently crossing." << std::endl;
	sem_post(&write_mut);									// release access to std::cout
	sleep(CROSSING_TIME);									// crossing the rope
}

void *eastbound_baboon(void*){
	bool done = false;
	int crossers_copy;
	while (!done){
		sem_wait(&crossers_mut);							// get access to crossers
		if (crossers >= 0){
			++crossers;										// increase number of eastbound crossers, if rope is eastbound or unoccupied
			crossers_copy = crossers;						// create local copy of crossers to pass to cross_rope
			if (crossers == 1)
				sem_wait(&rope_mut);						// if you are first eastbound crosser, gain exclusive eastbound access to rope
			sem_post(&crossers_mut);						// release exclusive access to crossers
			cross_rope(crossers_copy);						// cross the rope
			sem_wait(&crossers_mut);						// get access to crossers again, after crossing
			--crossers;										// number of eastbound crossers decreases after leaving the rope
			if (crossers == 0)
				sem_post(&rope_mut);						// if you are the last eastbound crosser, release exclusive eastbound access
			done = true;									// stop loop
		}
		else
			sleep(0);
		sem_post(&crossers_mut);							// release exclusive access to crossers
	}
	// die
	return 0;
}

void *westbound_baboon(void*){
	bool done = false;
	int crossers_copy;
	while (!done){
		sem_wait(&crossers_mut);							// get access to crossers
		if (crossers <= 0){
			--crossers;										// increase number of westbound crossers, if rope is westbound or unoccupied
			crossers_copy = crossers;
			if (crossers == -1)
				sem_wait(&rope_mut);						// if you are the first westbound crosser, gain exclusive westbound access to rope
			sem_post(&crossers_mut);						// release exclusive access to crossers
			cross_rope(crossers_copy);						// cross the rope
			sem_wait(&crossers_mut);						// get access to crossers again, after crossing
			++crossers;										// number of westbound crossers decreases after leaving the rope
			if (crossers == 0)
				sem_post(&rope_mut);						// if you are the last westbound crosser, release exclusive westbound access
			done = true;
		}
		else
			sleep(0);
		sem_post(&crossers_mut);							// release exclusive access to crossers
	}
	// die
	return 0;
}

int main(){
	sem_init(&crossers_mut, 0, 1);							// initialize crossers mutex
	sem_init(&rope_mut, 0, 1);								// initialize eastbound rope mutex
	sem_init(&write_mut, 0, 1);								// initialize write mutex
	pthread_t baboons[BABOONS_TO_CROSS];					// array of baboons (threads)
	srand(time(NULL));										// seed random number generator

	// create baboon
	for (int i = 0; i < BABOONS_TO_CROSS; ++i){
															// wait a random amount of time to create baboon
		sleep(-AVERAGE_CREATION_TIME * log((double)rand()/((double)RAND_MAX + 1)));
		double direction = (double)rand()/(double)RAND_MAX;	// create baboon in random direction
		if (direction < .5){								// create eastbound baboon
			pthread_create(&baboons[i], NULL, eastbound_baboon, (void *)i);
		}
		else												// create westbound baboon
			pthread_create(&baboons[i], NULL, westbound_baboon, (void *)i);
	}

	// ensure that all baboons cross rope by joining remaining threads
	for (int i = 0; i < BABOONS_TO_CROSS; ++i){
		pthread_join(baboons[i], NULL);
				// Most baboons will have already crossed,
				// so, for those, pthread_join() will return immediately.
	}
}
