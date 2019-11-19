//SortedList.c

#include "SortedList.h"
#include <sched.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int opt_yield;

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
    if (opt_yield & INSERT_YIELD) {
        sched_yield();
    }

    if (!list || !element)
        return;
    SortedList_t* p = list->next; 
    while (p != list) {
        if (strcmp(p->key, element->key) <= 0)
            p = p->next;
        else {
            break;
        }
    } 
    element->prev = p->prev;
    element->next = p;
    p->prev->next = element;
    p->prev = element;
    return;
}

int SortedList_delete( SortedListElement_t *element) {
    if (opt_yield & DELETE_YIELD) {
        sched_yield();
    }

    if (element && element->next && element->prev && 
        element->next->prev == element && element->prev->next == element) {
        element->next->prev = element->prev;
        element->prev->next = element->next;
        return 0;
    }
    return 1;

}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
    if (opt_yield & LOOKUP_YIELD) {
        sched_yield();
    }

    if (!list || !key)
        return NULL;

    SortedList_t* p = list->next;

    while (p != list) {
        
        if (strcmp(p->key, key) == 0) {
            return p;
        }
        else
            p = p->next;
    }
    return NULL;

}

int SortedList_length(SortedList_t *list) {
    if (opt_yield & LOOKUP_YIELD) {
        sched_yield();
    }

    int length = 0;
    SortedList_t* p = list->next;
    while (p != list) {
        
        if (p->next->prev != p || p->prev->next != p) {
            return -1;
        }
        p = p->next;
        length++;
    }

    return length;
}