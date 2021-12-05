#ifndef LINKEDLIST_H
#define LINKEDLIST_H

typedef struct Node
{
    void *data;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct LinkedList
{
    Node *start;
    Node *end;
    size_t data_sz;
} LinkedList;

void list_init(LinkedList *list, size_t data_sz);
void list_free(LinkedList *list);
Node *list_add(LinkedList *list, void *data);
void list_reverse_to(LinkedList *list, Node *stop);

#endif