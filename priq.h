/*
 *	Generic Priority Queue
 * uses linked list for implementation
 * supports duplicates
 *
 * Author: Razvan Cojocaru
 */


struct q_elem_t {
	struct q_elem_t *next;
	void *data;
	int pri;
};

struct pri_queue {
	struct q_elem_t *first;
	int size;
};

/* Initialize and alloc queue */
struct pri_queue *priq_new();

/* Push element into the queue on the corresponding position */
void priq_push(struct pri_queue *q, void *data, int pri);

/* Remove first item. returns NULL if empty */
void *priq_pop(struct pri_queue *q);

/* Get the first element without removing it from queue */
void *priq_top(struct pri_queue *q);

/* Free queue memory */
void priq_free(struct pri_queue *q);
