/**
 * MollenOS
 *
 * Copyright 2019, Philip Meulengracht
 *
 * This program is free software : you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation ? , either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Linked List Implementation
 *  - Implements standard list functionality. List is implemented using a doubly
 *    linked structure, allowing O(1) add/remove, and O(n) lookups.
 */

#ifndef __COMMON_LIST_H__
#define __COMMON_LIST_H__

#include <stdint.h>

typedef struct element {
    struct element* next;
    struct element* previous;
    
    void* key;
    void* value;
} element_t;
#define ELEMENT_INIT(elem, _key, _value) (elem)->next = NULL; (elem)->previous = NULL; (elem)->key = (void*)_key; (elem)->value = (void*)_value

typedef int (*list_cmp_fn)(void*, void*);

typedef struct list {
    struct element* head;
    struct element* tail;
    list_cmp_fn     cmp;
    int             count;
} list_t;

#define LIST_INIT          { NULL, NULL, list_cmp_default, 0 }
#define LIST_INIT_CMP(cmp) { NULL, NULL, cmp, 0 }

#define LIST_ENUMERATE_CONTINUE (int)0x0
#define LIST_ENUMERATE_STOP     (int)0x1
#define LIST_ENUMERATE_REMOVE   (int)0x2

#define foreach(i, collection)           element_t *i; for (i = (collection)->head; i != NULL; i = i->next)
#define foreach_reverse(i, collection)   element_t *i; for (i = (collection)->tail; i != NULL; i = i->previous)
#define _foreach(i, collection)          for (i = (collection)->head; i != NULL; i = i->next)
#define _foreach_reverse(i, collection)  for (i = (collection)->tail; i != NULL; i = i->previous)

#define foreach_nolink(i, collection)  element_t *i; for (i = (collection)->head; i != NULL; )
#define _foreach_nolink(i, collection) for (i = (collection)->head; i != NULL; )

// Default provided comparators
int list_cmp_default(void*,void*);
int list_cmp_string(void*,void*);

void list_construct(list_t*);
void list_construct_cmp(list_t*, list_cmp_fn);
void list_clear(list_t*, void(*)(element_t*, void*), void*);
int  list_count(list_t*);
int  list_append(list_t*, element_t*);
int  list_remove(list_t*, element_t*);

/**
 * list_splice
 * * Returns a spliced node list that can be appended to a new list. The list will
 * * at maximum have the requested count or the length of the list
 * @param list_in  [In] The list that will be spliced.
 * @param count    [In] The maximum number of list nodes that will be extracted.
 * @param list_out [In] The list which the number of elements will be spliced into.
 */
void list_splice(list_t*, int, list_t*);

element_t* list_front(list_t*);
element_t* list_back(list_t*);
element_t* list_find(list_t*, void*);
void*      list_find_value(list_t*, void*);
void       list_enumerate(list_t*, int(*)(int, element_t*, void*), void*);

#endif //!__COMMON_LIST_H__
