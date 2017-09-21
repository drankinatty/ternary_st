#include "ternary_st.h"

/** max word length to store in ternary search tree, stack size */
#define WRDMAX 128
#define STKMAX (WRDMAX * 2)

/** ternary search tree node. */
typedef struct node_tst {
    char key;               /* char key for node (null for node with string) */
    unsigned refcnt;        /* refcnt tracks occurrence of word (for delete) */
    struct node_tst *lokid, /* ternary low child pointer */
                    *eqkid, /* ternary equal child pointer */
                    *hikid; /* ternary high child pointer */
} node_tst;

/** struct to use for static stack to remove nodes. */
typedef struct tst_stack {
    void *data[STKMAX];
    size_t idx;
} tst_stack;

/** stack push/pop to store node pointers to delete word from tree.
 *  on delete, store all nodes from root to leaf containing word to
 *  allow word removal and reordering of tree.
 */
static void *tst_stack_push (tst_stack *s, void *node)
{
    if (s->idx >= STKMAX)
        return NULL;

    return (s->data[(s->idx)++] = node);
}

static void *tst_stack_pop (tst_stack *s)
{
    if (!s->idx) return NULL;

    void *node = s->data[--(s->idx)];
    s->data[s->idx] = NULL;

    return node;
}

/** delete current data-node and parent, update 'node' to new parent.
 *  before delete the current refcnt is checked, if non-zero, occurrences
 *  of the word remain in buffer the node is not deleted, if refcnt zero,
 *  the node is deleted. root node updated if changed. returns NULL
 *  on success (deleted), otherwise returns the address of victim
 *  if refcnt non-zero.
 */
static void *tst_del_word (node_tst **root, node_tst *node, tst_stack *stk)
{
    node_tst *victim = node,            /* begin deletion w/victim */
             *parent = tst_stack_pop (stk); /* parent to victim */

    if (!victim->refcnt) {              /* if last occurrence */
        if (!victim->key)               /* check key is nul   */
            free (victim->eqkid);       /* free string (data) */

        /* remove unique suffix chain - parent & victim nodes
         * have no children. simple remove until the first parent
         * found with children.
         */
        while (!parent->lokid && !parent->hikid &&
               !victim->lokid && !victim->hikid) {
            parent->eqkid = NULL;
            free (victim);
            victim = parent;
            parent = tst_stack_pop (stk);
            if (!parent) {              /* last word & root node */
                free (victim);
                return (void*)(*root = NULL);
            }
        }

        /* check if victim is prefix for others (victim has lo/hi node).
         * if both lo & hi children, find highest node under low with
         * children and make parent->eqkid equal highest, free victim.
         */
        if (victim->lokid && victim->hikid) {   /* victim has both lokid/hikid */
            if (!victim->lokid->hikid) {        /* check for hikid in lo tree */
                /* rotate victim->hikid to victim->lokid->hikid, and
                 * rotate victim->lokid to place of victim.
                 */
                victim->lokid->hikid = victim->hikid;
                if (!parent)
                    *root = victim->lokid;
                else if (victim == parent->lokid)
                    parent->lokid = victim->lokid;
                else if (victim == parent->hikid)
                    parent->hikid = victim->lokid;
                else
                    parent->eqkid = victim->lokid;
                free (victim);
                victim = NULL;
            }
            else if (!victim->hikid->lokid) {   /* check for lokid in hi tree */
                /* opposite rotation */
                victim->hikid->lokid = victim->lokid;
                if (!parent)
                    *root = victim->hikid;
                else if (victim == parent->lokid)
                    parent->lokid = victim->hikid;
                else if (victim == parent->hikid)
                    parent->hikid = victim->hikid;
                else
                    parent->eqkid = victim->hikid;
                free (victim);
                victim = NULL;
            }
            else    /* can't rotate, return, leaving victim->eqkid NULL */
                return NULL;
        }
        else if (victim->lokid) {   /* only lokid, replace victim with lokid */
            parent->eqkid = victim->lokid;
            free (victim);
            victim = NULL;
        }
        else if (victim->hikid) {   /* only hikid, replace victim with hikid */
            parent->eqkid = victim->hikid;
            free (victim);
            victim = NULL;
        }
        else {  /* victim - no children, but parent has other children */
            if (victim == parent->lokid) {      /* if parent->lokid - trim */
                parent->lokid = NULL;
                free (victim);
                victim = NULL;
            }
            else if (victim == parent->hikid) { /* if parent->hikid - trim */
                parent->hikid = NULL;
                free (victim);
                victim = NULL;
            }
            else {  /* victim was parent->eqkid, but parent->lo/hikid exists */
                parent->eqkid = NULL;               /* set eqkid NULL */
                free (victim);                      /* free current victim */
                victim = parent;                    /* set parent = victim */
                parent = tst_stack_pop (stk);       /* get new parent */
                /* if both victim hi/lokid are present */
                if (victim->lokid && victim->hikid) {
                    /* same checks and rotations as above */
                    if (!victim->lokid->hikid) {
                        victim->lokid->hikid = victim->hikid;
                        if (!parent)
                            *root = victim->lokid;
                        else if (victim == parent->lokid)
                            parent->lokid = victim->lokid;
                        else if (victim == parent->hikid)
                            parent->hikid = victim->lokid;
                        else
                            parent->eqkid = victim->lokid;
                        free (victim);
                        victim = NULL;
                    }
                    else if (!victim->hikid->lokid) {
                        victim->hikid->lokid = victim->lokid;
                        if (!parent)
                            *root = victim->hikid;
                        else if (victim == parent->lokid)
                            parent->lokid = victim->hikid;
                        else if (victim == parent->hikid)
                            parent->hikid = victim->hikid;
                        else
                            parent->eqkid = victim->hikid;
                        free (victim);
                        victim = NULL;
                    }
                    else
                        return NULL;
                }
                /* if only lokid, rewire to parent */
                else if (victim->lokid) {
                    if (parent) {   /* if parent exists, rewire */
                        if (victim == parent->lokid)
                            parent->lokid = victim->lokid;
                        else if (victim == parent->hikid)
                            parent->hikid = victim->lokid;
                        else
                            parent->eqkid = victim->lokid;
                    }
                    else            /* we are new root node, update root */
                        *root = victim->lokid;  /* make last node root */
                    free (victim);
                    victim = NULL;
                }
                /* if only hikid, rewire to parent */
                else if (victim->hikid) {
                    if (parent) {   /* if parent exists, rewire */
                        if (victim == parent->lokid)
                            parent->lokid = victim->hikid;
                        else if (victim == parent->hikid)
                            parent->hikid = victim->hikid;
                        else
                            parent->eqkid = victim->hikid;
                    }
                    else            /* we are new root node, update root */
                        *root = victim->hikid;  /* make last node root */
                    free (victim);
                    victim = NULL;
                }
            }
        }
    }
    else    /* node->refcnt non-zero */
        printf ("  %s  (refcnt: %u) not removed.\n",
                (char*)node->eqkid, node->refcnt);

    return victim;  /* return NULL on successful free, *node otherwise */
}

/** delete all nodes in 'node' chain, update 'node' to new parent.
 *  root node updated if changed. returns NULL on success (deleted),
 *  otherwise returns the address of victim.
 */
static void *tst_del_ref (node_tst **root, node_tst *node, tst_stack *stk)
{
    node_tst *victim = node,            /* begin deletion w/victim */
             *parent = tst_stack_pop (stk); /* parent to victim */

        /* remove unique suffix chain - parent & victim nodes
         * have no children. simple remove until the first parent
         * found with children.
         */
        while (!parent->lokid && !parent->hikid &&
               !victim->lokid && !victim->hikid) {
            parent->eqkid = NULL;
            free (victim);
            victim = parent;
            parent = tst_stack_pop (stk);
            if (!parent) {              /* last word & root node */
                free (victim);
                return (void*)(*root = NULL);
            }
        }

    /* check if victim is prefix for others (victim has lo/hi node).
        * if both lo & hi children, find highest node under low with
        * children and make parent->eqkid equal highest, free victim.
        */
    if (victim->lokid && victim->hikid) {   /* victim has both lokid/hikid */
        if (!victim->lokid->hikid) {        /* check for hikid in lo tree */
            /* rotate victim->hikid to victim->lokid->hikid, and
                * rotate victim->lokid to place of victim.
                */
            victim->lokid->hikid = victim->hikid;
            if (!parent)
                *root = victim->lokid;
            else if (victim == parent->lokid)
                parent->lokid = victim->lokid;
            else if (victim == parent->hikid)
                parent->hikid = victim->lokid;
            else
                parent->eqkid = victim->lokid;
            free (victim);
            victim = NULL;
        }
        else if (!victim->hikid->lokid) {   /* check for lokid in hi tree */
            /* opposite rotation */
            victim->hikid->lokid = victim->lokid;
            if (!parent)
                *root = victim->hikid;
            else if (victim == parent->lokid)
                parent->lokid = victim->hikid;
            else if (victim == parent->hikid)
                parent->hikid = victim->hikid;
            else
                parent->eqkid = victim->hikid;
            free (victim);
            victim = NULL;
        }
        else    /* can't rotate, return, leaving victim->eqkid NULL */
            return NULL;
    }
    else if (victim->lokid) {   /* only lokid, replace victim with lokid */
        parent->eqkid = victim->lokid;
        free (victim);
        victim = NULL;
    }
    else if (victim->hikid) {   /* only hikid, replace victim with hikid */
        parent->eqkid = victim->hikid;
        free (victim);
        victim = NULL;
    }
    else {  /* victim - no children, but parent has other children */
        if (victim == parent->lokid) {      /* if parent->lokid - trim */
            parent->lokid = NULL;
            free (victim);
            victim = NULL;
        }
        else if (victim == parent->hikid) { /* if parent->hikid - trim */
            parent->hikid = NULL;
            free (victim);
            victim = NULL;
        }
        else {  /* victim was parent->eqkid, but parent->lo/hikid exists */
            parent->eqkid = NULL;               /* set eqkid NULL */
            free (victim);                      /* free current victim */
            victim = parent;                    /* set parent = victim */
            parent = tst_stack_pop (stk);       /* get new parent */
            /* if both victim hi/lokid are present */
            if (victim->lokid && victim->hikid) {
                /* same checks and rotations as above */
                if (!victim->lokid->hikid) {
                    victim->lokid->hikid = victim->hikid;
                    if (!parent)
                        *root = victim->lokid;
                    else if (victim == parent->lokid)
                        parent->lokid = victim->lokid;
                    else if (victim == parent->hikid)
                        parent->hikid = victim->lokid;
                    else
                        parent->eqkid = victim->lokid;
                    free (victim);
                    victim = NULL;
                }
                else if (!victim->hikid->lokid) {
                    victim->hikid->lokid = victim->lokid;
                    if (!parent)
                        *root = victim->hikid;
                    else if (victim == parent->lokid)
                        parent->lokid = victim->hikid;
                    else if (victim == parent->hikid)
                        parent->hikid = victim->hikid;
                    else
                        parent->eqkid = victim->hikid;
                    free (victim);
                    victim = NULL;
                }
                else
                    return NULL;
            }
            /* if only lokid, rewire to parent */
            else if (victim->lokid) {
                if (parent) {   /* if parent exists, rewire */
                    if (victim == parent->lokid)
                        parent->lokid = victim->lokid;
                    else if (victim == parent->hikid)
                        parent->hikid = victim->lokid;
                    else
                        parent->eqkid = victim->lokid;
                }
                else            /* we are new root node, update root */
                    *root = victim->lokid;  /* make last node root */
                free (victim);
                victim = NULL;
            }
            /* if only hikid, rewire to parent */
            else if (victim->hikid) {
                if (parent) {   /* if parent exists, rewire */
                    if (victim == parent->lokid)
                        parent->lokid = victim->hikid;
                    else if (victim == parent->hikid)
                        parent->hikid = victim->hikid;
                    else
                        parent->eqkid = victim->hikid;
                }
                else            /* we are new root node, update root */
                    *root = victim->hikid;  /* make last node root */
                free (victim);
                victim = NULL;
            }
        }
    }

    return victim;  /* return NULL on successful free, *node otherwise */
}

/** tst_ins_del_cpy() insert or remove copy of 's' from ternary search tree.
 *  insert all nodes required for 's' in tree, with allocation for storage
 *  of 's' at eqkid node of leaf. increment refcnt, if 's' already exists
 *  (to be used for del). 0 for 'del' inserts, 1 for 'del' deletes. returns
 *  address of 's' in tree on successful insert (or on delete if refcnt non-
 *  zero on delete), NULL on allocation failure on insert, or on successful
 *  removal of 's' from tree.
 */
void *tst_ins_del_cpy (node_tst **root, const char *s, const int del)
{
    int diff;
    const char *p = s;
    tst_stack stk = { .data = {NULL}, .idx = 0 };
    node_tst *curr, **pcurr;

    if (!root || !s) return NULL;           /* validate parameters */
    if (strlen (s) + 1 > STKMAX / 2)        /* limit length to 1/2 STKMAX */
        return NULL;                        /* 128 char word lenght is plenty */

    pcurr = root;                           /* start at root */
    while ((curr = *pcurr)) {               /* iterate to insertion node  */
        diff = *p - curr->key;              /* get ASCII diff for >, <, = */
        if (diff == 0) {                    /* if char equal to node->key */
            if (*p++ == 0) {                /* check if word is duplicate */
                if (del) {                  /* delete instead of insert   */
                    (curr->refcnt)--;       /* decrement reference count  */
                    /* chk refcnt, del 's', return NULL on successful del */
                    return tst_del_word (root, curr, &stk);
                }
                else
                    curr->refcnt++;         /* increment refcnt if word exists */
                return (void *)curr->eqkid; /* pointer to word / NULL on del  */
            }
            pcurr = &(curr->eqkid);         /* get next eqkid pointer address */
        }
        else if (diff < 0) {                /* if char less than node->key */
            pcurr = &(curr->lokid);         /* get next lokid pointer address */
        }
        else {                              /* if char greater than node->key */
            pcurr = &(curr->hikid);         /* get next hikid pointer address */
        }
        if (del)
            tst_stack_push (&stk, curr);    /* push node on stack for del */
    }

    /* if not duplicate, insert remaining chars into tree rooted at curr */
    for (;;) {
        /* allocate memory for node, and fill. use calloc (or include
         * string.h and initialize w/memset) to avoid valgrind warning
         * "Conditional jump or move depends on uninitialised value(s)"
         */
        if (!(*pcurr = calloc (1, sizeof **pcurr))) {
            fprintf (stderr, "error: tst_insert(), memory exhausted.\n");
            return NULL;
        }
        curr = *pcurr;
        curr->key = *p;
        curr->refcnt = 1;
        curr->lokid = curr->hikid = curr->eqkid = NULL;

        if (!*root)         /* handle assignment to root if no root */
            *root = *pcurr;

        /* Place nodes until end of the string, at end of stign allocate
         * space for data, copy data as final eqkid, and return.
         */
        if (*p++ == 0) {
            const char *eqdata = strdup (s);
            if (!eqdata)
                return NULL;
            curr->eqkid = (node_tst *)eqdata;
            return (void*)eqdata;
        }
        pcurr = &(curr->eqkid);
    }
}

/** tst_ins_del_ref() insert or remove reference to 's' in ternary search tree.
 *  insert all nodes required for 's' in tree, insert 's' at eqkid node of leaf.
 *  0 for 'del' inserts, 1 for 'del' deletes. returns address of 's' in tree on
 *  successful insert or delete, NULL on successful removal of 's' from tree.
 */
void *tst_ins_del_ref (node_tst **root, char * const *s, const int del)
{
    int diff;
    const char *p = *s;
    tst_stack stk = { .data = {NULL}, .idx = 0 };
    node_tst *curr, **pcurr;

    if (!root || !*s) return NULL;          /* validate parameters */
    if (strlen (*s) + 1 > STKMAX / 2)       /* limit length to 1/2 STKMAX */
        return NULL;                        /* 128 char word lenght is plenty */

    pcurr = root;                           /* start at root */
    while ((curr = *pcurr)) {               /* iterate to insertion node  */
        diff = *p - curr->key;              /* get ASCII diff for >, <, = */
        if (diff == 0) {                    /* if char equal to node->key */
            if (*p++ == 0) {                /* check if word is duplicate */
                if (del) {                  /* delete instead of insert   */
                    (curr->refcnt)--;       /* decrement reference count  */
                    /* chk refcnt, del 's', return NULL on successful del */
                    return tst_del_ref (root, curr, &stk);
                }
                else
                    curr->refcnt++;         /* increment refcnt if word exists */
                return (void *)curr->eqkid; /* pointer to word / NULL on del  */
            }
            pcurr = &(curr->eqkid);         /* get next eqkid pointer address */
        }
        else if (diff < 0) {                /* if char less than node->key */
            pcurr = &(curr->lokid);         /* get next lokid pointer address */
        }
        else {                              /* if char greater than node->key */
            pcurr = &(curr->hikid);         /* get next hikid pointer address */
        }
        if (del)
            tst_stack_push (&stk, curr);    /* push node on stack for del */
    }

    /* if not duplicate, insert remaining chars into tree rooted at curr */
    for (;;) {
        /* allocate memory for node, and fill. use calloc (or include
         * string.h and initialize w/memset) to avoid valgrind warning
         * "Conditional jump or move depends on uninitialised value(s)"
         */
        if (!(*pcurr = calloc (1, sizeof **pcurr))) {
            fprintf (stderr, "error: tst_insert(), memory exhausted.\n");
            return NULL;
        }
        curr = *pcurr;
        curr->key = *p;
        curr->refcnt = 1;
        curr->lokid = curr->hikid = curr->eqkid = NULL;

        if (!*root)         /* handle assignment to root if no root */
            *root = *pcurr;

        /* Place nodes until end of the string, at end of stign allocate
         * space for data, copy data as final eqkid, and return.
         */
        if (*p++ == 0) {
            // printf ("adding '%s' at %p\n", *s, (void*)*s);
            curr->eqkid = (node_tst *)*s;
            return (void*)*s;
        }
        pcurr = &(curr->eqkid);
    }
}

/** tst_search(), non-recursive find of a string internary tree.
 *  returns pointer to 's' on success, NULL otherwise.
 */
void *tst_search (const node_tst *p, const char *s)
{
    const node_tst *curr = p;

    while (curr) {                          /* loop over each char in 's' */
        int diff = *s - curr->key;          /* calculate the difference */
        if (diff == 0) {                    /* handle the equal case */
            if (*s == 0)    /* if *s = curr->key = nul-char, 's' found */
                return (void *)curr->eqkid; /* return pointer to 's' */
            s++;
            curr = curr->eqkid;
        }
        else if (diff < 0)                  /* handle the less than case */
            curr = curr->lokid;
        else
            curr = curr->hikid;             /* handle the greater than case */
    }
    return NULL;
}

/** fill ptr array 'a' with strings matching prefix at node 'p'.
 *  the 'a' array will hold pointers to stored strings with prefix
 *  matching the string passed to tst_matching, ending in 'c', the
 *  nchr'th char in in each matched string.
 */
void tst_suggest (const node_tst *p, const char c, const size_t nchr,
                    char **a, int *n, const int max)
{
    if (!p || *n == max)
        return;
    tst_suggest (p->lokid, c, nchr, a, n, max);
    if (p->key)
        tst_suggest (p->eqkid, c, nchr, a, n, max);
    else if (*(((char*)p->eqkid) + nchr - 1) == c)
            a[(*n)++] = (char *)p->eqkid;
    tst_suggest (p->hikid, c, nchr, a, n, max);
}

/** tst_search_prefix fills ptr array 'a' with words prefixed with 's'.
 *  once the node containing the first prefix matching 's' is found
 *  tst_suggest is called to travers the ternary_tree beginning
 *  at the node filling 'a' with pointers to all words that contain
 *  the prefix upto 'max' words updating 'n' with the number of word
 *  in 'a'. a pointer to the first node is returned on success
 *  NULL otherwise.
 */
void *tst_search_prefix (const node_tst *root, const char *s,
                        char **a, int *n, const int max)
{
    const node_tst *curr = root;
    const char *start = s;

    if (!*s) return NULL;

    for (; *start; start++) {}              /* get length of s */
    const size_t nchr = start - s;

    start = s;
    *n = 0;

    /* Loop while we haven't hit a NULL node or returned */
    while (curr) {

        int diff = *s - curr->key;          /* calculate the difference */
        if (diff == 0) {                    /* handle the equal case */

            if ((size_t)(s - start) == nchr - 1) {
                tst_suggest (curr, curr->key, nchr, a, n, max);
                return (void*)curr;
            }
            if (*s == 0)
                return (void *)curr->eqkid;

            s++;
            curr = curr->eqkid;
        }
        else if (diff < 0)                  /* handle the less than case */
            curr = curr->lokid;
        else
            curr = curr->hikid;             /* handle the greater than case */
    }
    return NULL;
}

/** print_word(), function for tst_traverse_fn, print each word. */
void print_word (const void *node, void *data)
{
    printf ("%s\n", (char *)((node_tst *)node)->eqkid);
    if (data) {}
}

/** tst_traverse_fn(), traverse tree calling 'fn' on each word.
 *  prototype for 'fn' is void fn(const char *).
 */
void tst_traverse_fn (const node_tst *p, void(fn)(const void *, void *), void *data)
{
    if (!p)
        return;
    tst_traverse_fn (p->lokid, fn, data);
    if (p->key)
        tst_traverse_fn (p->eqkid, fn, data);
    else
        fn (p, data);
    tst_traverse_fn (p->hikid, fn, data);
}

/** free the ternary search tree rooted at p, data storage internal. */
void tst_free_all (node_tst *p)
{
    if (p) {
        tst_free_all (p->lokid);
        if (p->key)
            tst_free_all (p->eqkid);
        tst_free_all (p->hikid);
        if (!p->key)
            free (p->eqkid);
        free (p);
    }
}

/** free the ternary search tree rooted at p, data storage external. */
void tst_free (node_tst *p)
{
    if (p) {
        tst_free (p->lokid);
        if (p->key)
            tst_free (p->eqkid);
        tst_free (p->hikid);
        free (p);
    }
}

/** access functions tst_get_key(), tst_get_refcnt, & tst_get_string().
 *  provide access to struct members through opague pointers availale
 *  to program.
 */
char tst_get_key (const node_tst *node)
{
    return node->key;
}

unsigned tst_get_refcnt (const node_tst *node)
{
    return node->refcnt;
}

char *tst_get_string (const node_tst *node)
{
    if (node && !node->key)
        return (char*)node->eqkid;

    return NULL;
}
