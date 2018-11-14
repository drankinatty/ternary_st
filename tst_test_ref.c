#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ternary_st.h"

/** constants insert, delete, max word(s) & stack nodes */
enum { INS, DEL, WRDMAX = 256, STKMAX = 512, LMAX = 1024 };
#define REF INS
#define CPY DEL

/** timing helper function */
double tvgetf (void)
{
    struct timespec ts;
    double sec;

    clock_gettime(CLOCK_REALTIME,&ts);
    sec = ts.tv_nsec;
    sec /= 1e9;
    sec += ts.tv_sec;

    return sec;
}

/** trim trailing '\r\n' using strcspn */
void rmcrlf (char *s)
{
    s[strcspn (s, "\r\n")] = 0;
}

/** simple realloc of ptr with psz elemets to nelem * 2 */
void *xrealloc2 (void *ptr, size_t psz, size_t *nelem)
{
    void *memptr = realloc ((char *)ptr, *nelem * 2 * psz);

    if (!memptr) {
        fprintf (stderr, "realloc() error: virtual memory exhausted.\n");
        exit (EXIT_FAILURE);
    }   /* zero new memory (optional) */
    memset ((char *)memptr + *nelem * psz, 0, *nelem * psz);
    *nelem *= 2;

    return memptr;
}

int main (int argc, char **argv) {

    char word[WRDMAX] = "",
        *sgl[LMAX] = {NULL},
        **words = NULL;
    node_tst *root = NULL, *res = NULL;
    int rtn = 0, idx = 0, sidx = 0;
    size_t nptr = WRDMAX;
    FILE *fp = argc > 1 ? fopen (argv[1], "r") : NULL;
    double t1, t2;

    if (!fp) {    /* prompt, open, validate file for reading */
        char fname[LMAX] = "";
        printf ("enter filename: ");
        if (!fgets (fname, LMAX, stdin)) {  /* get filename  */
            fprintf (stderr, "error: invalid input.\n");
            return 1;
        }
        rmcrlf (fname);                     /* trim newline  */
        if (!(fp = fopen (fname, "r"))) {   /* open/validate */
            fprintf (stderr, "error: file open failed '%s'.\n", argv[1]);
            return 1;
        }
    }

    if (!(words = calloc (nptr, sizeof (*words))))  /* allocate ptrs */
        return 1;

    t1 = tvgetf();
    while ((rtn = fscanf (fp, "%s", word)) != EOF) {
        words[idx] = strdup (word); /* allocate/copy to words[idx] */
        if (!words[idx]) {
            fprintf (stderr, "error: memory exhausted (strdup).\n");
            return 1;
        }
        if (++idx == (int)nptr)     /* realloc ptrs as required */
            words = xrealloc2 (words, sizeof *words, &nptr);
    }
    t2 = tvgetf();
    printf ("%d words loaded from file in %.6f sec\n", idx, t2-t1);
    fclose (fp);        /* close file */

    t1 = tvgetf();
    for (int i = 0; i < idx; i++) { /* insert reference to each string */
        if (!tst_ins_del (&root, &words[i], INS, REF)) {
            fprintf (stderr, "error: memory exhausted, tst_insert.\n");
            return 1;
        }
    }
    t2 = tvgetf();
    printf ("ternary_tree, loaded %d words in %.6f sec\n\n", idx, t2-t1);

    for (;;) {
        printf ("\n p  print words in tree\n"
                " a  add word to the tree\n"
                " f  find word in tree\n"
                " s  search words matching prefix (enter 3 chars)\n"
                " d  delete word from the tree\n"
                " q  quit, freeing all data\n\n"
                "choice: ");
        fgets (word, sizeof word, stdin);

        switch (*word) {
            case 'p' :  printf ("\nprinting all words in tree (if <= 100).\n\n");
                        if (idx > 100) {
                            fprintf (stderr, "no. of words exceeds 100 (%d), skipped.\n", idx);
                            break;
                        }
                        else
                            tst_traverse_fn (root, print_word, NULL);
                        break;
            case 'a' :  printf ("enter word to add: ");
                        if (!fgets (word, sizeof word, stdin)) {
                            fprintf (stderr, "error: insufficient input.\n");
                            break;
                        }
                        rmcrlf (word);
                        words[idx] = strdup (word);
                        t1 = tvgetf();
                        res = tst_ins_del (&root, &words[idx], INS, REF);
                        t2 = tvgetf();
                        if (res) {
                            idx++;
                            printf ("  %s - inserted in %.6f sec. (%d words in tree)\n",
                                    (char*)res, t2 - t1, idx);
                        }
                        else
                            printf ("  %s - already exists in list.\n", (char*)res);
                        break;
            case 'f' :  printf ("find word in tree: ");
                        if (!fgets (word, sizeof word, stdin)) {
                            fprintf (stderr, "error: insufficient input.\n");
                            break;
                        }
                        rmcrlf (word);
                        t1 = tvgetf();
                        res = tst_search (root, word);
                        t2 = tvgetf();
                        if (res)
                            printf ("  found %s in %.6f sec.\n", (char*)res, t2-t1);
                        else
                            printf ("  %s not found.\n", word);
                        break;
            case 's' :  printf ("find words matching prefix (3 chars): ");
                        if (!fgets (word, sizeof word, stdin)) {
                            fprintf (stderr, "error: insufficient input.\n");
                            break;
                        }
                        rmcrlf (word);
                        t1 = tvgetf();
                        res = tst_search_prefix (root, word, sgl, &sidx, LMAX);
                        t2 = tvgetf();
                        if (res) {
                            printf ("  %s - searched prefix in %.6f sec\n\n", word, t2 - t1);
                            for (int i = 0; i < sidx; i++)
                                printf ("suggest[%d] : %s\n", i, sgl[i]);
                        }
                        else
                            printf ("  %s - not found\n", word);
                        break;
            case 'd' :  printf ("enter word to del: ");
                        char *tmp;
                        if (!fgets (word, sizeof word, stdin)) {
                            fprintf (stderr, "error: insufficient input.\n");
                            break;
                        }
                        rmcrlf (word);
                        tmp = strdup (word);
                        printf ("  deleting %s\n", word);
                        t1 = tvgetf();
                        res = tst_ins_del (&root, &tmp, DEL, REF);
                        t2 = tvgetf();
                        if (res)
                            printf ("  delete failed.\n");
                        else {
                            printf ("  deleted %s in %.6f sec\n", word, t2 - t1);
                            idx--;
                        }
                        free (tmp);
                        break;
            case 'q' :  tst_free (root);
                        goto close_menu;
                        break;
            default  :  printf ("error: invalid selection.\n");
                        break;
        }
    }
    close_menu:;

    for (int i = 0; i < idx; i++)   /* free words & pointers */
        free (words[i]);
    free (words);

    return 0;
}
