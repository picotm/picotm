/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "compiler.h"
#include <stddef.h>

/**
 * \ingroup group_lib
 * \file
 *
 * \brief Contains `struct picotm_slist` and helpers.
 *
 * The data structure `struct picotm_slist` represents a singly-linked
 * list, or *slist.* This can either be the list head, or an entry in a
 * list.
 *
 * The head of a singly-linked list is the main reference to a list. It
 * does not itself contain data. A call to `picotm_slist_init_head()`
 * initializes a list head.
 *
 * ~~~{.c}
 *  struct picotm_slist head;
 *  picotm_slist_init_head(&head);
 * ~~~
 *
 * Or alternatively, the C pre-processor macro `PICOTM_SLIST_HEAD_INITIALIZER`
 * initializes static or stack-allocated list heads.
 *
 * ~~~{.c}
 *  static struct picotm_slist head = PICOTM_SLIST_HEAD_INITIALIZER;
 * ~~~
 *
 * Calls to `picotm_slist_init_item()` or `PICOTM_SLIST_ITEM_INITIALIZER`
 * initialize list entries in a similar way.
 *
 * ~~~{.c}
 *  struct picotm_slist item;
 *  picotm_slist_init_item(&item);
 * ~~~
 *
 * ~~~{.c}
 *  static struct picotm_slist item = PICOTM_SLIST_ITEM_INITIALIZER;
 * ~~~
 *
 * Typically, slist items are stored in a container structure that also
 * holds additional application data. For example, a singly-linked list
 * that stores values of type `unsigned long` could use the following
 * data entries.
 *
 * ~~~{.c}
 *  struct ulong_item {
 *      struct picotm_slist item;   // slist element
 *      unsigned long       value;  // data value
 *  };
 *
 *  struct ulong_item*
 *  ulong_item_of_slist(struct picotm_slist* slist)
 *  {
        return picotm_containerof(item, struct ulong_item, slist);
 *  }
 * ~~~
 *
 * The singly-linked list refers from `item` to `item` field of the contained
 * list entries. The helper function `ulong_item_of_slist()` takes a list item
 * and returns the corresponding data element of type `struct ulong_item`.
 *
 * Both, slist head and item elements, require cleanup by their respective
 * clean-up function `picotm_slist_uninit_head()` or
 * `picotm_slist_uninit_item()`.
 *
 * ~~~{.c}
 *  picotm_slist_uninit_item(&item);
 *  picotm_slist_uninit_head(&head);
 * ~~~
 *
 * List item elements that are to be uninitialized may not be enqueued in a
 * list; head elements may not refer to any items.
 *
 * The functions `picotm_slist_enqueue_front()` and
 * `picotm_slist_enqueue_back` insert an entry at the front, resp. the back,
 * of an slist. If we already know an existing entry from an slist, we can
 * insert an entry before or after the existing one with a call to
 * `picotm_enqueue_slist_before()`, resp. `picotm_slist_enqueue_after()`.
 * Here's an example with `picotm_slist_enqueue_front()`.
 *
 * ~~~{.c}
 *  picotm_slist_enqueue_front(&head, &item);
 * ~~~
 *
 * A call to `picotm_slist_dequeue()` removes an entry from an slist.
 *
 * ~~~{.c}
 *  picotm_slist_dequeue(&item);
 * ~~~
 *
 * To see if an item is enqueued in an slist, we call
 * `picotm_slist_is_enqueued()`, whch returns `true` in this case. Likewise,
 * a call to `picotm_slist_empty()` returns `true` if a list is empty.
 *
 * ~~~{.c}
 *  bool is_enqueued = picotm_slist_is_enqueued(&item); // is in an slist
 *
 *  bool is_empty = picotm_slist_empty(&head); // no items in the slist
 * ~~~
 *
 * For iterating over a singly-linked list, the functions
 * `picotm_slist_begin()` and `picotm_slist_end()` return the slist's first
 * and final element. The final element is a terminator and not a useable
 * slist item. Reaching this value signals the callers to stop iterating.
 * We use `picotm_slist_next()` and `picotm_slist_prev()` for moving between
 * consecutive elements. singly-linked lists can only be traversed in forward
 * direction, so the latter function `picotm_slist_prev()` is of linear
 * complexity. I should be avoided within loop constructs.
 *
 * Here's a typical example of a loop that walks over all elements in an
 * slist. In this case, it sums up the values of type `unsigned long` that
 * are enqueued in the list.
 *
 * ~~~{.c}
 *  unsigned long long sum = 0;
 *
 *        struct picotm_slist* beg = picotm_slist_begin(&head);
 *  const struct picotm_slist* end = picotm_slist_end(&head);
 *
 *  while (beg != end) {
 *      const struct ulong_item* ulong = ulong_item_of_slist(beg);
 *
 *      sum += ulong->value;
 *
 *      beg = picotm_slist_next(beg);
 *  }
 *
 *  // do something with 'sum'
 * ~~~
 *
 * Singly-linked lists provide a number of built-in algorithms. With the
 * *walk* function `picotm_slist_walk_1()` we can walk over all entries in
 * a list. With *walk,* the previous example can be rewritten in the
 * following way.
 *
 * ~~~{.c}
 *  size_t
 *  sum_cb(struct picotm_slist* item, void* data)
 *  {
 *      const struct ulong_item* ulong = ulong_item_of_slist(item);
 *      unsigned long long* sum = data;
 *
 *      *sum += ulong->value;
 *
 *      return 1;
 *  }
 *
 *  unsigned long long sum = 0;
 *  picotm_slist_walk_1(head, sum_cb, &sum); // call sum_cb() for each item
 *
 *  // do something with 'sum'
 * ~~~
 *
 * The overall iteration over the slist entries is performed by
 * `picotm_slist_walk_1()`. The function calls `sum_cb()` for each item
 * and the pointer to `sum` is passed as additional data parameter.
 *
 * The function `sum_cb()` accumulates the list entries' values in
 * `sum` and returns the number of items to increment. Returning values
 * larger than *1* therefore allows to skip successive list entries.
 * Returning *0* prematurely stops the iteration. The value returned
 * by `picotm_slist_walk1()` is the element at which the iteration stopped.
 *
 * With the *find* function `picotm_slist_find_1()` we can search an slist
 * for the first item that matches a specific criteria. In the following
 * example, we search for the first element with a value of *0.*
 *
 * ~~~{.c}
 *  _Bool
 *  test_cb(struct picotm_slist* item, void* data)
 *  {
 *      const struct ulong_item* ulong = ulong_item_of_slist(item);
 *      const unsigned long* value = data;
 *
 *      return ulong->value = value; // return 'true' if values match
 *  }
 *
 *  unsigned long value = 0;
 *  struct picotm_slist* pos = picotm_slist_find_1(&head, test_cb, &value);
 *  if (pos == picotm_slist_end(&head)) {
 *      // value not found in list; bail out.
 *  }
 *
 *  // so something with 'pos'
 * ~~~
 *
 * The function `test_cb()` compares the value of an slist item to a
 * reference value, which is *0* in our case. If both match, it will return
 * 'true'. The function `picotm_slist_find1()` calls `test_cb()` for each
 * item in the slist and returns the first item for which the test function
 * returned 'true'. If none is found, the find function returns the final
 * terminator entry. Our example code checks and handles this case explicitly.
 *
 * Finally, there's the *clean-up* function `picotm_slist_cleanup_0()`, which
 * dequeues all list entries one by one and calls a clean-up function on
 * each. Once called, the clean-up function acquires ownership of the given
 * item. Here's an example of cleaning up a list of items that have been
 * allocated with `malloc()`. The corresponding `free()` is performed in the
 * item clean-up function.
 *
 * ~~~{.c}
 *  void
 *  cleanup_cb(struct picotm_slist* item)
 *  {
 *      struct ulong_item* ulong = ulong_item_of_slist(item);
 *      free(ulong);
 *  }
 *
 *  picotm_slist_cleanup_0(&head, cleanup_cb);
 * ~~~
 */

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_lib
 * \brief Entry in a singly-linked list.
 */
struct picotm_slist {
    struct picotm_slist* next;
};

/**
 * \ingroup group_lib
 * \internal
 * Static-initializer macro for `struct picotm_slist`.
 * \param   next_   A pointer to the next entry, or NULL.
 */
#define __PICOTM_SLIST_INITIALIZER(next_)   \
    {                                       \
        .next = (next_)                     \
    }

/**
 * \ingroup group_lib
 * Static-initializer macro for singly-linked list items.
 */
#define PICOTM_SLIST_ITEM_INITIALIZER   \
    __PIOTM_SLIST_INITIALIZER(NULL)

/**
 * \ingroup group_lib
 * Static-initializer macro for singly-linked list heads.
 * \param   head_   The singly-linked list's head entry.
 */
#define PICOTM_SLIST_HEAD_INITIALIZER(head_)    \
    __PIOTM_SLIST_INITIALIZER(head_)

/**
 * \ingroup group_lib
 * Initialize an entry of a singly-linked list.
 * \param   item    The singly-linked list's entry.
 * \returns The list entry.
 */
static inline struct picotm_slist*
picotm_slist_init_item(struct picotm_slist* item)
{
    item->next = NULL;
    return item;
}

/**
 * \ingroup group_lib
 * Uninitialize an entry of a singly-linked list.
 * \param   item    The singly-linked list's entry.
 */
static inline void
picotm_slist_uninit_item(struct picotm_slist* item)
{
    /* no-op for now */
}

/**
 * \ingroup group_lib
 * Initialize the head entry of a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \returns The list head.
 */
static inline struct picotm_slist*
picotm_slist_init_head(struct picotm_slist* head)
{
    head->next = head;
    return head;
}

/**
 * \ingroup group_lib
 * Uninitialize the head entry of a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 */
static inline void
picotm_slist_uninit_head(struct picotm_slist* head)
{
    /* no-op for now */
}

/**
 * \ingroup group_lib
 * Returns true if a singly-linked list item is enqueued.
 * \param   item    The singly-linked list entry.
 * \returns True if the entry is enqueued, or false otherwise.
 */
static inline _Bool
picotm_slist_is_enqueued(const struct picotm_slist* item)
{
    return !!item->next;
}

/**
 * \ingroup group_lib
 * Returns the next entry in a singly-linked list.
 * \param   item    The singly-linked list's current entry.
 * \returns The next entry in the singly-linked list, or the list head
 *          at the end of the list.
 */
static inline struct picotm_slist*
picotm_slist_next(const struct picotm_slist* item)
{
    return item->next;
}

/**
 * \ingroup group_lib
 * Returns the previous entry in a singly-linked list.
 * \param   item    The singly-linked list's current entry.
 * \returns The next entry in the singly-linked list, or the list head
 *          at the beginning of the list.
 */
static inline struct picotm_slist*
picotm_slist_prev(const struct picotm_slist* item)
{
    struct picotm_slist* prev = (struct picotm_slist*)item;
    while (picotm_slist_next(prev) != item) {
        prev = picotm_slist_next(prev);
    }
    return prev;
}

/**
 * \ingroup group_lib
 * Returns the first entry in a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \returns The first entry in the singly-linked list, or the list head
 *          if the list is empty.
 */
static inline struct picotm_slist*
picotm_slist_begin(const struct picotm_slist* head)
{
    return picotm_slist_next(head);
}

/**
 * \ingroup group_lib
 * Returns the terminating entry in a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \returns The terminating entry in the singly-linked list.
 */
static inline const struct picotm_slist*
picotm_slist_end(const struct picotm_slist* head)
{
    return head;
}

/**
 * \ingroup group_lib
 * Returns true if a singly-linked list is empty.
 * \param   head    The singly-linked list's head entry.
 * \returns True if the singly-linked list is empty, or false otherwise.
 */
static inline _Bool
picotm_slist_is_empty(const struct picotm_slist* head)
{
    return picotm_slist_begin(head) == picotm_slist_end(head);
}

/**
 * \ingroup group_lib
 * Enqueues an item to a singly-linked list before an existing item.
 * \param   item    The singly-linked list's exiting entry.
 * \param   newitem The new entry for the singly-linked list.
 */
static inline void
picotm_slist_enqueue_before(struct picotm_slist* item,
                            struct picotm_slist* newitem)
{
    struct picotm_slist* prev = picotm_slist_prev(item);

    newitem->next = item;
    prev->next = newitem;
}

/**
 * \ingroup group_lib
 * Enqueues an item to a singly-linked list after an existing item.
 * \param   item    The singly-linked list's exiting entry.
 * \param   newitem The new entry for the singly-linked list.
 */
static inline void
picotm_slist_enqueue_after(struct picotm_slist* item,
                           struct picotm_slist* newitem)
{
    newitem->next = picotm_slist_next(item);
    item->next = newitem;
}

/**
 * \ingroup group_lib
 * Enqueues an item at the beginning of a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \param   newitem The new entry for the singly-linked list.
 */
static inline void
picotm_slist_enqueue_front(struct picotm_slist* head,
                           struct picotm_slist* newitem)
{
    picotm_slist_enqueue_after(head, newitem);
}

/**
 * \ingroup group_lib
 * Enqueues an item at the end of a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \param   newitem The new entry for the singly-linked list.
 */
static inline void
picotm_slist_enqueue_back(struct picotm_slist* head,
                          struct picotm_slist* newitem)
{
    picotm_slist_enqueue_before(head, newitem);
}

/**
 * \ingroup group_lib
 * Enqueues an item to a sorted singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \param   newitem The new entry for the singly-linked list.
 * \param   cmp     The compare-function for sorting the entries.
 */
static inline void
picotm_slist_enqueue_sorted(struct picotm_slist* head,
                            struct picotm_slist* newitem,
                            int (*cmp)(struct picotm_slist*,
                                       struct picotm_slist*))
{
    struct picotm_slist* item = picotm_slist_begin(head);
    struct picotm_slist* prev = head;

    while (item != picotm_slist_end(head)) {
        if (cmp(newitem, item) > 0) {
            break;
        }
        prev = item;
        item = picotm_slist_next(item);
    }

    picotm_slist_enqueue_after(prev, newitem);
}

/**
 * \ingroup group_lib
 * Dequeues an item of a singly-linked list.
 * \param   item    The singly-linked list's exiting entry.
 */
static inline void
picotm_slist_dequeue(struct picotm_slist* item)
{
    struct picotm_slist* prev = picotm_slist_prev(item);
    prev->next = picotm_slist_next(item);
    item->next = NULL;
}

/**
 * \ingroup group_lib
 * Dequeues the front item of a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 */
static inline void
picotm_slist_dequeue_front(struct picotm_slist* head)
{
    struct picotm_slist* next = picotm_slist_next(head);
    head->next = picotm_slist_next(next);
    next->next = NULL;
}

/**
 * \ingroup group_lib
 * Returns the front item of a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \returns The first entry in the singly-linked list, or the list head
 *          if the list is empty.
 */
static inline struct picotm_slist*
picotm_slist_front(const struct picotm_slist* head)
{
    if (picotm_slist_is_empty(head)) {
        return NULL;
    }
    return picotm_slist_begin(head);
}

/**
 * \ingroup group_lib
 * Returns the final item of a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \returns The first entry in the singly-linked list, or the list head
 *          if the list is empty.
 */
static inline struct picotm_slist*
picotm_slist_back(const struct picotm_slist* head)
{
    if (picotm_slist_is_empty(head)) {
        return NULL;
    }
    return picotm_slist_prev(picotm_slist_end(head));
}

/**
 * \ingroup group_lib
 * Finds an item in a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \param   test    The test function for the find operation.
 * \returns An entry in the singly-linked list, or the list head
 *          if the entry could not be found.
 */
static inline struct picotm_slist*
picotm_slist_find_0(const struct picotm_slist* head,
                    _Bool (*test)(const struct picotm_slist*))
{
          struct picotm_slist* beg = picotm_slist_begin(head);
    const struct picotm_slist* end = picotm_slist_end(head);

    for (; beg != end; beg = picotm_slist_next(beg)) {
        if (test(beg)) {
            return beg;
        }
    }
    return beg;
}

/**
 * \ingroup group_lib
 * Finds an item in a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \param   test    The test function for the find operation.
 * \param   data    The data parameter for the test function.
 * \returns An entry in the singly-linked list, or the list head
 *          if the entry could not be found.
 */
static inline struct picotm_slist*
picotm_slist_find_1(const struct picotm_slist* head,
                    _Bool (*test)(const struct picotm_slist*, void*),
                    void* data)
{
          struct picotm_slist* beg = picotm_slist_begin(head);
    const struct picotm_slist* end = picotm_slist_end(head);

    for (; beg != end; beg = picotm_slist_next(beg)) {
        if (test(beg, data)) {
            return beg;
        }
    }
    return beg;
}

/**
 * \ingroup group_lib
 * Finds an item in a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \param   test    The test function for the find operation.
 * \param   data1   The first data parameter for the test function.
 * \param   data2   The second data parameter for the test function.
 * \returns An entry in the singly-linked list, or the list head
 *          if the entry could not be found.
 */
static inline struct picotm_slist*
picotm_slist_find_2(const struct picotm_slist* head,
                    _Bool (*test)(const struct picotm_slist*, void*, void*),
                    void* data1, void* data2)
{
          struct picotm_slist* beg = picotm_slist_begin(head);
    const struct picotm_slist* end = picotm_slist_end(head);

    for (; beg != end; beg = picotm_slist_next(beg)) {
        if (test(beg, data1, data2)) {
            return beg;
        }
    }
    return beg;
}

/**
 * \ingroup group_lib
 * Walks over all items in a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \param   walk    The call-back function for each item.
 * \returns The singly-linked list's head entry on success, or the
 *          last-processed entry on errors.
 */
static inline struct picotm_slist*
picotm_slist_walk_0(const struct picotm_slist* head,
                    size_t (*walk)(struct picotm_slist*))
{
          struct picotm_slist* beg = picotm_slist_begin(head);
    const struct picotm_slist* end = picotm_slist_end(head);

    while (beg != end) {
        size_t incr = walk(beg);
        if (!incr) {
            break;
        }
        for (; incr && (beg != end); --incr) {
            beg = picotm_slist_next(beg);
        }
    }
    return beg;
}

/**
 * \ingroup group_lib
 * Walks over all items in a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \param   walk    The call-back function for each item.
 * \param   data    The data parameter for the walk function.
 * \returns The singly-linked list's head entry on success, or the
 *          last-processed entry on errors.
 */
static inline struct picotm_slist*
picotm_slist_walk_1(const struct picotm_slist* head,
                    size_t (*walk)(struct picotm_slist*, void* data),
                    void* data)
{
          struct picotm_slist* beg = picotm_slist_begin(head);
    const struct picotm_slist* end = picotm_slist_end(head);

    while (beg != end) {
        size_t incr = walk(beg, data);
        if (!incr) {
            break;
        }
        for (; incr && (beg != end); --incr) {
            beg = picotm_slist_next(beg);
        }
    }
    return beg;
}

/**
 * \ingroup group_lib
 * Walks over all items in a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \param   walk    The call-back function for each item.
 * \param   data1   The first data parameter for the walk function.
 * \param   data2   The second data parameter for the walk function.
 * \returns The singly-linked list's head entry on success, or the
 *          last-processed entry on errors.
 */
static inline struct picotm_slist*
picotm_slist_walk_2(const struct picotm_slist* head,
                    size_t (*walk)(struct picotm_slist*, void* data1,
                                   void* data2),
                    void* data1, void* data2)
{
          struct picotm_slist* beg = picotm_slist_begin(head);
    const struct picotm_slist* end = picotm_slist_end(head);

    while (beg != end) {
        size_t incr = walk(beg, data1, data2);
        if (!incr) {
            break;
        }
        for (; incr && (beg != end); --incr) {
            beg = picotm_slist_next(beg);
        }
    }
    return beg;
}

/**
 * \ingroup group_lib
 * Cleans up all items of a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \param   cleanup The clean-up function for each item.
 */
static inline void
picotm_slist_cleanup_0(struct picotm_slist* head,
                       void (*cleanup)(struct picotm_slist*))
{
    while (!picotm_slist_is_empty(head)) {
        struct picotm_slist* item = picotm_slist_front(head);
        picotm_slist_dequeue_front(head);
        cleanup(item);
    }
}

/**
 * \ingroup group_lib
 * Cleans up all items of a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \param   cleanup The clean-up function for each item.
 * \param   data1   The first data parameter for the walk function.
 */
static inline void
picotm_slist_cleanup_1(struct picotm_slist* head,
                       void (*cleanup)(struct picotm_slist*, void*),
                       void* data1)
{
    while (!picotm_slist_is_empty(head)) {
        struct picotm_slist* item = picotm_slist_front(head);
        picotm_slist_dequeue_front(head);
        cleanup(item, data1);
    }
}

/**
 * \ingroup group_lib
 * Cleans up all items of a singly-linked list.
 * \param   head    The singly-linked list's head entry.
 * \param   cleanup The clean-up function for each item.
 * \param   data1   The first data parameter for the walk function.
 * \param   data2   The second data parameter for the walk function.
 */
static inline void
picotm_slist_cleanup_2(struct picotm_slist* head,
                       void (*cleanup)(struct picotm_slist*, void*, void*),
                       void* data1, void* data2)
{
    while (!picotm_slist_is_empty(head)) {
        struct picotm_slist* item = picotm_slist_front(head);
        picotm_slist_dequeue_front(head);
        cleanup(item, data1, data2);
    }
}

PICOTM_END_DECLS
