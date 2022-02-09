#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "harness.h"
#include "queue.h"

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
queue_t *q_new()
{
    queue_t *q = malloc(sizeof(queue_t));

    /**
     *  malloc return NULL if block cannot be allocated
     *
     *  reference :
     *  https://www.gnu.org/software/libc/manual/html_node/Basic-Allocation.html
     */
    if (q != NULL) {
        q->head = NULL;
        q->tail = NULL;
        q->size = 0;
    }
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    if (q != NULL) {
        /* traverse list and free nodes and their data */
        for (list_ele_t *ptr = q->head, *next; ptr != NULL; ptr = next) {
            next = ptr->next;
            free(ptr->value);
            free(ptr);
        }
        /* Free queue structure */
        free(q);
    }
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
    if (q != NULL) {
        list_ele_t *new_head = malloc(sizeof(list_ele_t));

        if (new_head != NULL) {
            new_head->next = NULL;
            new_head->value = malloc(strlen(s) + 1);

            if (new_head->value != NULL) {
                memcpy(new_head->value, s, strlen(s) + 1);
                new_head->next = q->head;

                /* let the new node be the new head */
                q->head = new_head;

                /* special case : empty queue */
                if (q->tail == NULL)
                    q->tail = new_head;

                /* not forget to increase size */
                q->size++;

                return true;
            }
            /* malloc for string failed */
            free(new_head);
        }
        /* malloc for node failed */
    }
    /* NULL queue */
    return false;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
    if (q != NULL) {
        list_ele_t *new_tail = malloc(sizeof(list_ele_t));

        if (new_tail != NULL) {
            new_tail->next = NULL;
            new_tail->value = malloc(strlen(s) + 1);

            if (new_tail->value != NULL) {
                /* copy value */
                memcpy(new_tail->value, s, strlen(s) + 1);

                /* special case : empty queue */
                if (q->head == NULL) {
                    q->head = new_tail;
                    q->tail = new_tail;
                }

                /* normal case, append and set new tail */
                else {
                    q->tail->next = new_tail;
                    q->tail = new_tail;
                }

                /* not forget to increase size */
                q->size++;

                return true;
            }
            /* malloc for string failed */
            free(new_tail);
        }
        /* malloc for node failed */
    }
    /* NULL queue */
    return false;
}

/*
 * Attempt to remove element from head of queue.
 * Return true if successful.
 * Return false if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 * The space used by the list element and the string should be freed.
 */
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
    /* not forget to check queue not NULL or empty */
    if (q != NULL && q->size != 0) {
        /* keep current head and use next node to be the new head */
        list_ele_t *old_head = q->head;
        q->head = old_head->next;

        /* copy content if needed */
        if (sp != NULL) {
            strncpy(sp, old_head->value, bufsize - 1);
            sp[bufsize - 1] = '\0';
        }

        /* free old head and its content */
        free(old_head->value);
        free(old_head);

        /* don't forget decrease size */
        q->size--;

        return true;
    }
    return false;
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    /* Remember: It should operate in O(1) time */
    if (q != NULL)
        return q->size;
    return 0;
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
    if (q != NULL && q->size > 1) {
        /* traverse queue */
        list_ele_t *prev = NULL, *ptr = q->head, *next = ptr->next;

        for (; next != NULL; prev = ptr, ptr = next, next = next->next)
            ptr->next = prev;
        ptr->next = prev; /* last node (ptr) should also point to prev */

        /* don't forget swap head and tail */
        q->tail = q->head;
        q->head = ptr;
    }
}

list_ele_t *merge(list_ele_t *a, list_ele_t *b)
{
    list_ele_t head, *pHead = &head;

    for (; a && b; pHead = pHead->next) {
        if (strcmp(a->value, b->value) < 0) {
            pHead->next = a;
            a = a->next;
        } else {
            pHead->next = b;
            b = b->next;
        }
    }
    pHead->next = a ? a : b;

    return head.next;
}

list_ele_t *mergesort(list_ele_t *start, list_ele_t *end)
{
    // break end
    end->next = NULL;

    // if only 1 node
    if (start == end)
        return start;

    else {
        // find middle and its next node
        list_ele_t *mid = start, *mid2 = start;

        for (; mid2->next && mid2->next->next;) {
            mid = mid->next;
            mid2 = mid2->next->next;
        }
        mid2 = mid->next;

        // break list into 2 parts
        mid->next = NULL;

        // sort saperately
        list_ele_t *left = mergesort(start, mid);
        list_ele_t *right = mergesort(mid2, end);

        return merge(left, right);
    }
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(queue_t *q)
{
    if (q != NULL && q->size > 1) {
        q->head = mergesort(q->head, q->tail);

        // find new end
        for (; q->tail->next != NULL; q->tail = q->tail->next)
            ;
    }
}