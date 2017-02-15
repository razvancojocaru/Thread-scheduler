/*
 *	Operating Systems - Assignment 4
 *	Author: Razvan Cojocaru 333CA
 */


#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>

#include "so_scheduler.h"
#include "priq.h"


struct thread_struct {
	int priority;
	so_handler *handler;
	sem_t sem;
	unsigned quantum;
	tid_t id;
};


/* Linked list implementation with search function by ID */
/* List node structure */
struct node {
	struct node *next;
	struct thread_struct *data;
};

/* List structure */
struct list {
	struct node *pfirst;
	struct node *plast;
};

/* Initialize the list. Must be called before any other operation */
void init_list(struct list *l)
{
	l->pfirst = NULL;
	l->plast = NULL;
}

/* Remove the first element from the list */
static void pop_free(struct list *l)
{
	struct node *paux;

	if (l->pfirst == NULL)
		return;
	paux = l->pfirst;
	l->pfirst = (l->pfirst)->next;

	free(paux->data);
	free(paux);
}

static struct thread_struct *pop(struct list *l)
{
	struct node *paux;
	struct thread_struct *aux;

	if (l->pfirst == NULL)
		return NULL;
	paux = l->pfirst;
	l->pfirst = paux->next;

	aux = paux->data;
	return aux;
}

/* Empty the list and free memory */
static void free_list(struct list *l)
{
	while (l->pfirst != NULL) {
		pop_free(l);
	}
}

/* Return 1 if list is empty, 0 otherwise */
static int is_empty(struct list *l)
{
	if (l->pfirst == NULL)
		return 1;
	return 0;
}

/* Add an element at the end of the list */
static void add_elem(struct list *l, struct thread_struct *data)
{
	struct node *paux;

	paux = (struct node *)malloc(sizeof(struct node));
	paux->data = data;
	paux->next = NULL;
	if (l->plast != NULL)
		l->plast->next = paux;
	l->plast = paux;
	if (l->pfirst == NULL)
		l->pfirst = paux;
}

/* Remove the given element from the list */
static void remove_elem(struct list *l, struct thread_struct *data)
{
	struct node *paux = l->pfirst;

	if (paux == NULL)
		return;

	if ((paux->next == NULL) && (paux->data == data)) {
		free(paux->data);
		free(paux);
		l->pfirst = NULL;
		l->plast = NULL;
	} else {
		struct node *prev = paux;

		do {
			if (paux->data == data) {
				prev->next = paux->next;
				if (l->pfirst == paux)
					l->pfirst = paux->next;
				if (l->plast == paux)
					l->plast = prev;
				free(paux->data);
				free(paux);
				break;
			}
			prev = paux;
			paux = paux->next;
		} while (paux != NULL);
	}
}

/* Find the element which contains the pointer ptr as the "start" attribute */
static struct thread_struct *find_thread(struct list *l, tid_t id)
{
	struct node *paux;

	paux = l->pfirst;
	if (paux == NULL)
		return NULL;
	if ((paux->data)->id == id)
		return paux->data;

	while (paux != NULL) {
		if ((paux->data)->id == id)
			return paux->data;
		paux = paux->next;
	}
	return NULL;
}


/* Global vars used for thread management */
unsigned QUANTUM, IO;
char IS_RUNNING;
char FIRST_FORK;
/* List used to keep track of all the active threads */
struct list thread_list;
/* Priority queue used for the Round Robin scheduler - holds READY threads*/
struct pri_queue *queue;
/* Semaphore used to sync threads during fork */
sem_t parent_sync;
/* Array of list used for IO event sync - holds WAITING threads*/
struct list *io_events;


/* Schedule a task and reset quantum if necessary */
static void call_scheduler(struct thread_struct *calling_thread)
{
	struct thread_struct* thread;
	thread = (struct thread_struct *)priq_pop(queue);

	if (thread == NULL)
		return;

	if (calling_thread == NULL) {
		sem_post(&thread->sem);
		return;
	}

	if (thread != calling_thread) {
		calling_thread->quantum = 0;
	} else {
		if (calling_thread->quantum == QUANTUM)
			calling_thread->quantum = 0;
	}

	sem_post(&thread->sem);
}


/* Puts current thread in queue and calls handler when scheduled */
static void start_thread(void* args)
{
	struct thread_struct *new_thread = (struct thread_struct*)args;

	priq_push(queue, (void *)new_thread, new_thread->priority);
	sem_post(&parent_sync);

	sem_wait(&new_thread->sem);	
	(new_thread->handler)(new_thread->priority);
	
	remove_elem(&thread_list, new_thread);
	call_scheduler(NULL);

}


/* Dummy function used to add first thread to queue */
static void dummy(unsigned priority) {}


int so_init(unsigned quantum, unsigned io)
{
	/* Check initialization params */
	if (quantum < 1) {
		return -1;
	}
	if ((io < 0) || (io > SO_MAX_NUM_EVENTS)) {
		return -1;
	}
	if (IS_RUNNING == 1) {
		return -1;
	}
	int i;

	/* Initialize global vars */
	queue = priq_new();
	QUANTUM = quantum;
	IO = io;
	IS_RUNNING = 1;
	FIRST_FORK = 0;
	init_list(&thread_list);
	sem_init(&parent_sync, 0, 0);
	io_events = (struct list*)malloc((io+1) * sizeof(struct list));
	for (i = 0; i <= io; i++) {
		init_list(&io_events[i]);
	}

	/* Add initial thread to thread structure, with lowest priority */
	struct thread_struct *new_thread = (struct thread_struct*)
										malloc(sizeof(struct thread_struct));
	new_thread->priority = -1;
	new_thread->handler = dummy;
	sem_init(&new_thread->sem, 0, 0);
	new_thread->quantum = 0;
	new_thread->id = pthread_self();
	add_elem(&thread_list, new_thread);



	return 0;
}


tid_t so_fork(so_handler *thread, unsigned priority)
{
	/* Initial parameter check */
	if (thread == INVALID_TID) {
		return INVALID_TID;
	}
	if ((priority < 0) || (priority > SO_MAX_PRIO)) {
		return INVALID_TID;
	}

	int rc;
	tid_t id;
	struct thread_struct *new_thread = (struct thread_struct*)
										malloc(sizeof(struct thread_struct));

	/* Create new thread structure element */
	new_thread->priority = priority;
	new_thread->handler = thread;
	sem_init(&new_thread->sem, 0, 0);
	new_thread->quantum = 0;

	rc = pthread_create(&id, NULL, (void*)start_thread, (void*)new_thread);
	if (rc != 0) {
		sem_destroy(&new_thread->sem);
		free(new_thread);
		return INVALID_TID;
	}

	new_thread->id = id;
	add_elem(&thread_list, new_thread);

	struct thread_struct* this = find_thread(&thread_list, pthread_self());
	this->quantum++;

	sem_wait(&parent_sync);
	/* Check if current thread has to be preempted */
	if (new_thread->priority > this->priority) {
		this->quantum = 0;
		priq_push(queue, (void *)this, this->priority);
		call_scheduler(this);
		sem_wait(&this->sem);	
	} else if (this->quantum == QUANTUM) {
		this->quantum = 0;
		priq_push(queue, (void *)this, this->priority);
		call_scheduler(this);
		sem_wait(&this->sem);
	}

	return id;
}


int so_wait(unsigned io)
{
	if ((io < 0) || (io >= IO))
		return -1;

	/* Put current thread in io structure but not in queue */
	struct thread_struct* this = find_thread(&thread_list, pthread_self());
	add_elem(&io_events[io], this);
	call_scheduler(this);

	sem_wait(&this->sem);

	return 0;
}


int so_signal(unsigned io)
{
	if ((io < 0) || (io >= IO))
		return -1;
	if (is_empty(&io_events[io])) {
		return 0;
	}
	int no_threads = 0;
	int max_prio = -1;
	struct thread_struct *thread;
	struct list* signal_list = &io_events[io];

	/* Empty the corresponding io structure element and put threads in queue */
	while (!is_empty(signal_list)) {
		no_threads++;
		thread = pop(signal_list);
		if (max_prio < thread->priority)
			max_prio = thread->priority;
		priq_push(queue, (void *)thread, thread->priority);
	}

	struct thread_struct* this = find_thread(&thread_list, pthread_self());
	this->quantum++;

	/* Check if current thread has to be preempted */
	if (max_prio > this->priority) {
		priq_push(queue, (void *)this, this->priority);
		call_scheduler(this);
		sem_wait(&this->sem);
	} else if (this->quantum == QUANTUM) {
		this->quantum = 0;
		priq_push(queue, (void *)this, this->priority);
		call_scheduler(this);
		sem_wait(&this->sem);
	}

	return no_threads;
}


void so_exec(void)
{
	struct thread_struct* this = find_thread(&thread_list, pthread_self());
	this->quantum++;

	/* Check if current thread has to be preempted */
	if (this->quantum == QUANTUM) {
		this->quantum = 0;
		priq_push(queue, (void *)this, this->priority);
		call_scheduler(this);
		sem_wait(&this->sem);
	}
	return;
}


void so_end(void)
{	
	if (IS_RUNNING == 0)
		return;

	struct thread_struct* this = find_thread(&thread_list, pthread_self());

	IS_RUNNING = 0;
	sem_destroy(&this->sem);
	remove_elem(&thread_list, this);
	sem_destroy(&parent_sync);
	free(io_events);
	priq_free(queue);
	free_list(&thread_list);

	return;
}

