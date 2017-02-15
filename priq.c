/*
 *	Generic Priority Queue
 * uses linked list for implementation
 * supports duplicates
 *
 * Author: Razvan Cojocaru
 */


#include <stdlib.h>
#include "priq.h"


struct pri_queue *priq_new()
{
	struct pri_queue *pq = (struct pri_queue *)malloc(sizeof(struct pri_queue));

	pq->size = 0;
	pq->first = NULL;
	return pq;
}


void priq_push(struct pri_queue *q, void *data, int pri)
{

	if (q->size == 0) {
		q->size++;
		q->first = (struct q_elem_t *)malloc(sizeof(struct q_elem_t));
		q->first->data = data;
		q->first->pri = pri;
		q->first->next = NULL;
		return;
	}
	q->size++;

	struct q_elem_t *new_elem = (struct q_elem_t *)
								malloc(sizeof(struct q_elem_t));	
	new_elem->data = data;
	new_elem->pri = pri;

	struct q_elem_t *aux = q->first;
	if (aux->pri < pri) {
		new_elem->next = aux;
		q->first = new_elem;
		return;
	}

	struct q_elem_t *prev = aux;
	aux = aux->next;
	while ((aux != NULL) && (aux->pri >= pri)) {
		prev = aux;
		aux = aux->next;		
	}

	new_elem->next = aux;
	prev->next = new_elem;

}


void *priq_pop(struct pri_queue *q)
{
	if (q->size == 0)
		return NULL;
	q->size--;
	struct q_elem_t *aux = q->first;
	void *data = aux->data;
	q->first = aux->next;
	free(aux);
	return data;
}


void *priq_top(struct pri_queue *q)
{
	if (q->size == 0)
		return NULL;
	return q->first->data;
}


void priq_free(struct pri_queue *q)
{
	while (q->size > 0)
		priq_pop(q);
	free(q);
}

