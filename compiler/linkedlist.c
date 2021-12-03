#include <stdlib.h>
#include <stdio.h>
#include "linkedlist.h"

static void node_free(Node *node)
{
    if (node == NULL)
    {
        return;
    }

    if (node->data != NULL)
    {
        free(node->data);
    }

    free(node);
}

void list_init(LinkedList *list, size_t data_sz)
{
    list->start = NULL;
    list->end = NULL;
    list->data_sz = data_sz;
}

void list_free(LinkedList *list)
{
    if (list == NULL)
    {
        return;
    }

    Node *node = list->start;
    if (node == NULL)
    {
        return;
    }

    Node *next;
    do
    {
        next = node->next;
        node_free(node);
        node = next;
    } while (next != NULL);

    list->start = NULL;
    list->end = NULL;
}

Node *list_add(LinkedList *list, void *data)
{
    // Create new node
    Node *new_node = malloc(sizeof(*new_node));
    if (new_node == NULL)
    {
        fprintf(stderr, "Unable to allocate memory for new node.\n");
        return NULL;
    }

    // Copy data into node
    new_node->data = malloc(list->data_sz);
    for (size_t i = 0; i < list->data_sz; i++)
    {
        *((char *)(new_node->data) + i) = *((char *)(data) + i);
    }
    new_node->next = NULL;

    // Insert the new node just after the previous end node
    Node *end_node = list->end;
    if (end_node != NULL)
    {
        new_node->prev = end_node;
        end_node->next = new_node;
    }
    else
    {
        new_node->prev = NULL;
        list->start = new_node;
    }
    list->end = new_node;

    return new_node;
}

void list_reverse_to(LinkedList *list, Node *stop)
{
    Node *node = list->end;
    if (node == NULL)
    {
        return;
    }

    Node *prev = node;

    while (node != stop)
    {
        prev = node->prev;
        node_free(node);
        node = prev;
    }

    list->end = stop;
    list->end->next = NULL;
}