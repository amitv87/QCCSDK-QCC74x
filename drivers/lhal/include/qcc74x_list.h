#ifndef _QCC74x_LIST_H
#define _QCC74x_LIST_H

#include "string.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * container_of - return the member address of ptr, if the type of ptr is the
 * struct type.
 */
#define qcc74x_container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/**
 * Double List structure
 */
struct qcc74x_dlist_node {
    struct qcc74x_dlist_node *next; /**< point to next node. */
    struct qcc74x_dlist_node *prev; /**< point to prev node. */
};
typedef struct qcc74x_dlist_node qcc74x_dlist_t; /**< Type for lists. */

/**
 * @brief initialize a list
 *
 * @param l list to be initialized
 */
static inline void qcc74x_dlist_init(qcc74x_dlist_t *l)
{
    l->next = l->prev = l;
}

/**
 * @brief insert a node after a list
 *
 * @param l list to insert it
 * @param n new node to be inserted
 */
static inline void qcc74x_dlist_insert_after(qcc74x_dlist_t *l, qcc74x_dlist_t *n)
{
    l->next->prev = n;
    n->next = l->next;

    l->next = n;
    n->prev = l;
}

/**
 * @brief insert a node before a list
 *
 * @param n new node to be inserted
 * @param l list to insert it
 */
static inline void qcc74x_dlist_insert_before(qcc74x_dlist_t *l, qcc74x_dlist_t *n)
{
    l->prev->next = n;
    n->prev = l->prev;

    l->prev = n;
    n->next = l;
}

/**
 * @brief remove node from list.
 * @param n the node to remove from the list.
 */
static inline void qcc74x_dlist_remove(qcc74x_dlist_t *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;

    n->next = n->prev = n;
}

/**
 * @brief move node from list.
 * @param n the node to remove from the list.
 */
static inline void qcc74x_dlist_move_head(qcc74x_dlist_t *l, qcc74x_dlist_t *n)
{
    qcc74x_dlist_remove(n);
    qcc74x_dlist_insert_after(l, n);
}

/**
 * @brief move node from list.
 * @param n the node to remove from the list.
 */
static inline void qcc74x_dlist_move_tail(qcc74x_dlist_t *l, qcc74x_dlist_t *n)
{
    qcc74x_dlist_remove(n);
    qcc74x_dlist_insert_before(l, n);
}

/**
 * @brief tests whether a list is empty
 * @param l the list to test.
 */
static inline int qcc74x_dlist_isempty(const qcc74x_dlist_t *l)
{
    return l->next == l;
}

/**
 * @brief get the list length
 * @param l the list to get.
 */
static inline unsigned int qcc74x_dlist_len(const qcc74x_dlist_t *l)
{
    unsigned int len = 0;
    const qcc74x_dlist_t *p = l;

    while (p->next != l) {
        p = p->next;
        len++;
    }

    return len;
}

/**
 * @brief initialize a dlist object
 */
#define DLIST_OBJECT_INIT(object) \
    {                             \
        &(object), &(object)      \
    }
/**
 * @brief initialize a dlist object
 */
#define DLIST_DEFINE(list) \
    qcc74x_dlist_t list = { &(list), &(list) }

/**
 * @brief get the struct for this entry
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define qcc74x_dlist_entry(node, type, member) \
    qcc74x_container_of(node, type, member)

/**
 * dlist_first_entry - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define qcc74x_dlist_first_entry(ptr, type, member) \
    qcc74x_dlist_entry((ptr)->next, type, member)
/**
 * dlist_first_entry_or_null - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define qcc74x_dlist_first_entry_or_null(ptr, type, member) \
    (qcc74x_dlist_isempty(ptr) ? NULL : qcc74x_dlist_first_entry(ptr, type, member))

/**
 * dlist_for_each - iterate over a list
 * @pos:    the dlist_t * to use as a loop cursor.
 * @head:   the head for your list.
 */
#define qcc74x_dlist_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * dlist_for_each_prev - iterate over a list
 * @pos:    the dlist_t * to use as a loop cursor.
 * @head:   the head for your list.
 */
#define qcc74x_dlist_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * dlist_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:    the dlist_t * to use as a loop cursor.
 * @n:      another dlist_t * to use as temporary storage
 * @head:   the head for your list.
 */
#define qcc74x_dlist_for_each_safe(pos, n, head)                  \
    for (pos = (head)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

#define qcc74x_dlist_for_each_prev_safe(pos, n, head)             \
    for (pos = (head)->prev, n = pos->prev; pos != (head); \
         pos = n, n = pos->prev)
/**
 * dlist_for_each_entry  -   iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define qcc74x_dlist_for_each_entry(pos, head, member)                 \
    for (pos = qcc74x_dlist_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head);                                \
         pos = qcc74x_dlist_entry(pos->member.next, typeof(*pos), member))

/**
 * dlist_for_each_entry_reverse  -   iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define qcc74x_dlist_for_each_entry_reverse(pos, head, member)         \
    for (pos = qcc74x_dlist_entry((head)->prev, typeof(*pos), member); \
         &pos->member != (head);                                \
         pos = qcc74x_dlist_entry(pos->member.prev, typeof(*pos), member))

/**
 * dlist_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define qcc74x_dlist_for_each_entry_safe(pos, n, head, member)          \
    for (pos = qcc74x_dlist_entry((head)->next, typeof(*pos), member),  \
        n = qcc74x_dlist_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head);                                 \
         pos = n, n = qcc74x_dlist_entry(n->member.next, typeof(*n), member))

/**
 * dlist_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:      another type * to use as temporary storage
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define qcc74x_dlist_for_each_entry_safe_reverse(pos, n, head, member)  \
    for (pos = qcc74x_dlist_entry((head)->prev, typeof(*pos), field),   \
        n = qcc74x_dlist_entry(pos->member.prev, typeof(*pos), member); \
         &pos->member != (head);                                 \
         pos = n, n = qcc74x_dlist_entry(pos->member.prev, typeof(*pos), member))

#ifdef __cplusplus
}
#endif

#endif