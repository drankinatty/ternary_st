#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ternary_st.h"

/** constants insert, delete, max word(s) & stack nodes */
enum { INS, DEL, WRDMAX = 256, STKMAX = 512, LMAX = 1024 };
#define REF INS
#define CPY DEL

/** rand_int for use with shuffle */
static int rand_int (int n)
{
    int limit = RAND_MAX - RAND_MAX % n, rnd;

    rnd = rand();
    for (; rnd >= limit; )
        rnd = rand();

    return rnd % n;
}

/** shuffle an array of pointers */
void shuffle_ptrs (char **a, size_t n)
{
    char *tmp;
    size_t i;

    while (n-- > 1) {
        i = rand_int (n);
        tmp  = a[i];
        a[i] = a[n];
        a[n] = tmp;
    }
}

/** realloc 'ptr' of 'nelem' of 'psz' to 'nelem * 2' of 'psz'.
 *  returns pointer to reallocated block of memory with new
 *  memory initialized to 0/NULL. return must be assigned to
 *  original pointer in caller.
 */
void *xrealloc (void *ptr, size_t psz, size_t *nelem)
{
    void *memptr = realloc ((char *)ptr, *nelem * 2 * psz);
    if (!memptr) {
        fprintf (stderr, "realloc() error: virtual memory exhausted.\n");
        exit (EXIT_FAILURE);
    }
    /* zero new memory (optional) */
    memset ((char *)memptr + *nelem * psz, 0, *nelem * psz);
    *nelem *= 2;

    return memptr;
}

int main (int argc, char **argv) {

    char word[WRDMAX] = "",
        **words = NULL;
    node_tst *root = NULL, *node = NULL;
    size_t i, idx = 0, nptrs = WRDMAX;
    FILE *fp = argc > 1 ? fopen (argv[1], "r") : stdin;

    srand (time(NULL));

    if (!fp) {  /* validate file open for reading */
        fprintf (stderr, "error: file open failed '%s'.\n", argv[1]);
        return 1;
    }

    /* allocate nptrs pointers for word storage (for delete testing) */
    if (!(words = calloc (nptrs, sizeof *words))) {
        fprintf (stderr, "error: memory exhausted words ptrs.");
        return 1;
    }

    /* read words (1 per-line) from fp, insert in tree, add to words */
    while (fscanf (fp, "%s", word) == 1) {
        /* tree insert - allocated (char*)node returned, NULL on failure */
        char *p = word;
        if (!tst_ins_del (&root, &p, INS, CPY)) {
            fprintf (stderr, "error: memory exhausted, tst_insert.\n");
            return 1;
        }
        words[idx] = strdup (word);         /* copy to words */
        if (!words[idx]) {                  /* validate strdup allocation */
            fprintf (stderr, "error: memory exhausted, words[%zu]\n", idx);
            return 1;
        }
        if (++idx == nptrs)         /* realloc as required */
            words = xrealloc (words, sizeof *words, &nptrs);
    }
    if (fp != stdin) fclose (fp);   /* close file if not stdin */
    printf ("ternary_search_tree, loaded, %zu words.\n", idx);

    shuffle_ptrs (words, idx);      /* shuffle pointers in words */

    for (i = 0; i < idx; i++) {
        if (!tst_search (root, words[i])) {     /* search for word */
            fprintf (stderr, "tst_search - failed: %s\n", words[i]);
            break;
        }
        if ((node = tst_ins_del (&root, &words[i], DEL, CPY)) &&
                tst_get_refcnt (node) == 0) {  /* delete from tree */
            fprintf (stderr, "tst_insert_alloc (DEL) - failed: %s\n", words[i]);
            break;
        }
        if ((node = tst_search (root, words[i])) &&
                tst_get_refcnt (node) == 0) {   /* confirm delete */
            fprintf (stderr, "tst_search - %s found after DEL\n", words[i]);
            break;
        }
        for (size_t j = i + 1; j < idx; j++)    /* validate all others found */
            if (!tst_search (root, words[j])) {
                fprintf (stderr, "tst_search - ptr error: %s - not found\n",
                        words[j]);
                goto testdone;
            }
    }
    testdone:;

    if (i == idx)
        printf ("%zu successful deletions from search tree.\n", i);

    for (i = 0; i < idx; i++)
        free (words[i]);
    free (words);

    return 0;
}
