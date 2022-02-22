#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));

    // check malloc not return null (implies error or size is 0)
    if (head)
        INIT_LIST_HEAD(head);

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;  // should not free NULL list


    // traverse and delete all entries and its value in the list
    element_t *entry = NULL, *next = NULL;
    list_for_each_entry_safe (entry, next, l, list) {
        free(entry->value);
        free(entry);
    }

    // free list
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;  // head should not be null


    element_t *node = malloc(sizeof(element_t));
    if (!node)
        return false;  // malloc for new node failed


    node->value = strdup(s);
    if (!node->value) {
        free(node);
        return false;  // malloc for string failed
    }

    // insert new node after head
    list_add(&node->list, head);

    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;  // cannot insert a node into NULL list

    // try to create new node
    element_t *node = malloc(sizeof(element_t));
    if (!node)
        return false;  // malloc for new node failed


    // try to allocate space for copying string
    node->value = strdup(s);
    if (!node->value) {
        free(node);
        return false;  // malloc for string failed
    }

    // insert node after the last node in the list
    list_add_tail(&node->list, head);

    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;  // cannot remove a node from NULL or empty list


    // get node and unlink (remove) this node from list
    element_t *first_entry = list_first_entry(head, element_t, list);
    list_del(head->next);

    // copy target node's value if need
    if (sp) {
        strncpy(sp, first_entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return first_entry;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;  // cannot remove a node from NULL or empty list


    // get node and unlink (remove) this node from list
    element_t *last_entry = list_last_entry(head, element_t, list);
    list_del(head->prev);

    // copy target node's value if need
    if (sp) {
        strncpy(sp, last_entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return last_entry;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int count = 0;
    for (struct list_head *ptr = head->next; ptr != head; ptr = ptr->next)
        ++count;

    return count;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/

    if (!head || list_empty(head))
        return false;  // NULL or empty list

    // move to mid node
    struct list_head *ptr = head->next;
    for (struct list_head *rev = head->prev; rev != ptr && rev != ptr->next;) {
        ptr = ptr->next;
        rev = rev->prev;
    }

    // remove this node
    list_del(ptr);

    // delete entry
    element_t *entry = list_entry(ptr, element_t, list);
    free(entry->value);
    free(entry);

    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;


    LIST_HEAD(duplist);

    bool prev = false;
    element_t *ptr = list_entry(head->next, element_t, list), *next = ptr;

    for (bool same; next->list.next != head; ptr = next) {
        next = list_entry(ptr->list.next, element_t, list);
        same = !strcmp(ptr->value, next->value);
        if (same || prev)
            list_move(&ptr->list, &duplist);
        prev = same;
    }
    // don't forget last node
    if (prev)
        list_move(&ptr->list, &duplist);

    // delete each element in dup list
    list_for_each_entry_safe (ptr, next, &duplist, list) {
        free(ptr->value);
        free(ptr);
    }

    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/

    if (!head || head->next == head->prev)
        return;  // NULL, empty or single node no need to swap

    struct list_head *left = head->next, *right = left->next;

    do {
        // swap
        left->prev->next = right;
        right->next->prev = left;
        left->next = right->next;
        right->prev = left->prev;
        left->prev = right;
        right->next = left;

        // move to next
        left = left->next;
        right = left->next;

    } while (left != head && right != head);
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;  // cannot return NULL or empty list


    // traverse list and swap their next and prev
    struct list_head *ptr = head, *tmp;
    do {
        tmp = ptr->next;
        ptr->next = ptr->prev;
        ptr->prev = tmp;
        ptr = ptr->prev;
    } while (ptr != head);
}

struct list_head *mergesort(struct list_head *start, struct list_head *end)
{
    // if only one node, just return
    if (start == end)
        return start;

    // if 2 or more nodes, split into 2 parts and do mergesort on both parts
    struct list_head *mid = start, *flag = start;
    for (; flag->next && flag->next->next; flag = flag->next->next)
        mid = mid->next;

    flag = mid->next;
    mid->next = NULL;

    struct list_head *left = mergesort(start, mid);
    struct list_head *right = mergesort(flag, end);

    // merge 2 sorted list
    struct list_head *new_head = start, **phead = &new_head;

    for (element_t *l, *r; left && right; phead = &(*phead)->next) {
        l = list_entry(left, element_t, list);
        r = list_entry(right, element_t, list);

        struct list_head **next;
        next = strcmp(l->value, r->value) < 0 ? &left : &right;
        *phead = *next;
        *next = (*next)->next;
    }
    *phead = left ? left : right;

    return new_head;
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || head->prev == head->next)
        return;

    // do merge sort on list
    struct list_head *start = head->next, *end = head->prev;
    end->next = NULL;

    // connect head and sorted list
    head->next = mergesort(start, end);
    head->next->prev = head;

    // repair all prev links
    for (end = head; end->next; end = end->next)
        end->next->prev = end;
    end->next = head;
    head->prev = end;
}