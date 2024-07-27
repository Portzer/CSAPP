#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "../header/algorithm.h"

/*======================================*/
/*      Base Class Methods              */
/*======================================*/

void linkedlist_internal_free(linkedlist_base *list,
                              linkedlist_node_access *node_access)
{
    if (list == NULL)
    {
        return;
    }
    assert(list->update_head != NULL);
    assert(node_access != NULL);
    assert(node_access->compare_nodes != NULL);
    assert(node_access->destruct_node != NULL);
    assert(node_access->get_node_prev != NULL);
    assert(node_access->set_node_prev != NULL);
    assert(node_access->get_node_next != NULL);
    assert(node_access->set_node_next != NULL);

    int count_copy = list->count;
    for (int i = 0; i < count_copy; ++ i)
    {
        uint64_t node = list->head;
        list->update_head(list,
                          node_access->get_node_next(list->head));

        if (node_access->compare_nodes(node, list->head) == 0)
        {
            // TODO: logic can be removed
            // only one element
            node_access->destruct_node(node);
            // do not update list->count during deleting
        }
        else
        {
            // at least 2 elements
            uint64_t prev = node_access->get_node_prev(node);
            uint64_t next = node_access->get_node_next(node);

            node_access->set_node_next(prev, next);
            node_access->set_node_prev(next, prev);

            node_access->destruct_node(node);
            // do not update list->count during deleting
        }
    }

    free(list);
}

linkedlist_base *linkedlist_internal_add(linkedlist_base *list,
                                         linkedlist_node_access *node_access,
                                         uint64_t value)
{
    if (list == NULL)
    {
        return NULL;
    }
    assert(list->update_head != NULL);
    assert(node_access != NULL);
    assert(node_access->get_node_prev != NULL);
    assert(node_access->set_node_prev != NULL);
    assert(node_access->set_node_next != NULL);
    assert(node_access->construct_node != NULL);
    assert(node_access->set_node_value != NULL);

    uint64_t node = node_access->construct_node();
    node_access->set_node_value(node, value);
    return linkedlist_internal_insert(list, node_access, node);
}

linkedlist_base *linkedlist_internal_insert(linkedlist_base *list,
                                            linkedlist_node_access *node_access,
                                            uint64_t node)
{
    if (list == NULL)
    {
        return NULL;
    }
    assert(list->update_head != NULL);
    assert(node_access != NULL);
    assert(node_access->get_node_prev != NULL);
    assert(node_access->set_node_prev != NULL);
    assert(node_access->set_node_next != NULL);

    if (list->count == 0)
    {
        // create a new head
        list->update_head(list, node);
        list->count = 1;
        // circular linked list initialization
        node_access->set_node_prev(node, node);
        node_access->set_node_next(node, node);
    }
    else
    {
        // insert to head
        uint64_t head = list->head;
        uint64_t head_prev = node_access->get_node_prev(head);

        node_access->set_node_next(node, head);
        node_access->set_node_prev(head, node);

        node_access->set_node_prev(node, head_prev);
        node_access->set_node_next(head_prev, node);

        list->update_head(list, node);
        list->count ++;
    }

    return list;
}

int linkedlist_internal_delete(linkedlist_base *list,
                               linkedlist_node_access *node_access,
                               uint64_t node)
{
    if (list == NULL || node == NULL_ID)
    {
        return 0;
    }
    assert(list->update_head != NULL);
    assert(node_access != NULL);
    assert(node_access->destruct_node != NULL);
    assert(node_access->compare_nodes != NULL);
    assert(node_access->get_node_prev != NULL);
    assert(node_access->set_node_prev != NULL);
    assert(node_access->get_node_next != NULL);
    assert(node_access->set_node_next != NULL);

    // update the prev and next pointers
    // same for the only one node situation
    uint64_t prev = node_access->get_node_prev(node);
    uint64_t next = node_access->get_node_next(node);

    if (prev != NULL_ID)
    {
        node_access->set_node_next(prev, next);
    }

    if (next != NULL_ID)
    {
        node_access->set_node_prev(next, prev);
    }

    // if this node to be free is the head
    if (node_access->compare_nodes(list->head, node) == 0)
    {
        list->update_head(list, next);
    }

    // free the node managed by the list
    node_access->destruct_node(node);

    // reset the linked list status
    list->count --;

    if (list->count == 0)
    {
        list->update_head(list, NULL_ID);
    }

    return 1;
}

uint64_t linkedlist_internal_index(linkedlist_base *list,
                                   linkedlist_node_access *node_access,
                                   uint64_t index)
{
    if (list == NULL || index >= list->count)
    {
        return NULL_ID;
    }
    assert(node_access != NULL);
    assert(node_access->get_node_next != NULL);

    uint64_t p = list->head;
    for (int i = 0; i <= index; ++ i)
    {
        p = node_access->get_node_next(p);
    }

    return p;
}

// traverse the linked list
uint64_t linkedlist_internal_next(linkedlist_base *list,
                                  linkedlist_node_access *node_access)
{
    if (list == NULL || node_access->compare_nodes(list->head, NULL_ID) == 0)
    {
        return NULL_ID;
    }
    assert(list->update_head != NULL);
    assert(node_access != NULL);
    assert(node_access->get_node_next != NULL);

    uint64_t old_head = list->head;
    list->update_head(list,
                      node_access->get_node_next(old_head));

    return old_head;
}

/*======================================*/
/*      Default Implementation          */
/*======================================*/

// Implementation of the list node access

static uint64_t construct_node()
{
    return (uint64_t)malloc(sizeof(linkedlist_node_t));
}

static int destruct_node(uint64_t node_id)
{
    if (node_id == NULL_ID)
    {
        return 0;
    }
    linkedlist_node_t *node = (linkedlist_node_t *)node_id;

    free(node);
    return 1;
}

static int compare_nodes(uint64_t first, uint64_t second)
{
    return !(first == second);
}

static uint64_t get_node_prev(uint64_t node_id)
{
    if (node_id == NULL_ID)
    {
        return NULL_ID;
    }
    return (uint64_t)(((linkedlist_node_t *)node_id)->prev);
}

static int set_node_prev(uint64_t node_id, uint64_t prev_id)
{
    if (node_id == NULL_ID)
    {
        return 0;
    }
    *(uint64_t *)&(((linkedlist_node_t *)node_id)->prev) = prev_id;
    return 1;
}

static uint64_t get_node_next(uint64_t node_id)
{
    if (node_id == NULL_ID)
    {
        return NULL_ID;
    }
    return (uint64_t)(((linkedlist_node_t *)node_id)->next);
}

static int set_node_next(uint64_t node_id, uint64_t next_id)
{
    if (node_id == NULL_ID)
    {
        return 0;
    }
    *(uint64_t *)&(((linkedlist_node_t *)node_id)->next) = next_id;
    return 1;
}

static uint64_t get_node_value(uint64_t node_id)
{
    if (node_id == NULL_ID)
    {
        return NULL_ID;
    }
    return (uint64_t)(((linkedlist_node_t *)node_id)->value);
}

static int set_node_value(uint64_t node_id, uint64_t value)
{
    if (node_id == NULL_ID)
    {
        return 0;
    }
    ((linkedlist_node_t *)node_id)->value = value;
    return 1;
}

static linkedlist_node_access node_access =
        {
                .construct_node = &construct_node,
                .destruct_node = &destruct_node,
                .compare_nodes = &compare_nodes,
                .get_node_prev = &get_node_prev,
                .set_node_prev = &set_node_prev,
                .get_node_next = &get_node_next,
                .set_node_next = &set_node_next,
                .get_node_value = &get_node_value,
                .set_node_value = &set_node_value
        };

// child class of base class
static int update_head(linkedlist_base *this, uint64_t new_head)
{
    if (this == NULL)
    {
        return 0;
    }
    this->head = new_head;
    return 1;
}

// constructor and destructor
linkedlist_t *linkedlist_construct()
{
    linkedlist_t *list = malloc(sizeof(linkedlist_t));
    list->base.count = 0;
    list->base.head = 0;
    list->base.update_head = &update_head;
    return list;
}

void linkedlist_free(linkedlist_t *list)
{
    linkedlist_internal_free(&(list->base), &node_access);
}

linkedlist_t *linkedlist_add(linkedlist_t *list, uint64_t value)
{
    linkedlist_internal_add(
            &list->base,
            &node_access,
            value
    );
    return list;
}

int linkedlist_delete(linkedlist_t *list, linkedlist_node_t *node)
{
    return linkedlist_internal_delete(
            &list->base,
            &node_access,
            (uint64_t)node
    );
}

linkedlist_node_t *linkedlist_next(linkedlist_t *list)
{
    return (linkedlist_node_t *)linkedlist_internal_next(
            &list->base,
            &node_access);
}

linkedlist_node_t *linkedlist_index(linkedlist_t *list, uint64_t index)
{
    return (linkedlist_node_t *)linkedlist_internal_index(
            &list->base,
            &node_access,
            index);
}