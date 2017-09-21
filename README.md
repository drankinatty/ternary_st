#ternary_st
Ternary Search Tree for Prefix-Search

Copyright David C. Rankin, J.D.,P.E. 2017

Licensed Under GPLv2 availabe here

https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html

A *ternary search tree (tst)* is a binary search tree (bst) with an additional node pointer.

Given the durth of information available about ternary trees, and specifially, proper node-rotation when a node is deleted from the tree. Having found no examples and the only examples for delete being simple deletes leaving the ternary tree dirty by leaving a node, with siblings, but no valid middle (or equal) pointer. To resolve the lack of a valid delete, the following code was written as part of a text-editor word-completion implementation.

With a binary search tree each node contains a *left* and *right* node-pointer so a binary choice controls traversal of the tree. Either a *greater than* or *less than* choice. As the result of a comparison between the node-data and a reference either the left or right node-pointer is used to further descend.

A ternary search tree adds a middle node. While you can still use the `left-middle-right` notation, ternary trees use a `low-equal-high` pointer verbiage. With each node holding a *key ID*, the node pointers for the code here uses a `lokid`, `eqkid`, and `hikid` naming convention to descend though the node based on a difference between the node key value and a reference value. If the difference is negative (lower than), the `lokid` node is traversed, if they are equal, the `eqkid` node is followed or `hikid` is followed.

In addition to the `node->key`, the node used in this example code adds a *reference count* (a `node->refcnt`) to track the number of occurrances for each word it holds. So for example, if using the tree to track the words in an editor buffer (where there may be multiple occurrences of 'the' or other common words), the node holding 'the' is not deleted, upon delete, until no other occurrences remain (i.e. the `node->refcnt` is zero).

Each individual node has the following form:

                              o
                              |-key
                              |-refcnt
                  ------------+------------
                  |lokid      |eqkid      |hikid
                  o           o           o


The data (a pointer to a word, or a copy of the word itself) is stored in an additional node after the node holding the last character (node->key) in the word cast to type (node *) and stored as the `node->eqkid`. Further, since this is the *node after the last character*, it's key is the *nul-character* (decimal `0`) just as you would have ending a string. Thus, the 'key's for the nodes from where it is attached at its root are the letters in the word with the final node having a key `0` with either a pointer to the string (if stored in an external data structure) or an allocated copy of the string itself if the string is to be stored in the tree. (as in holding the words for an edit buffer, where the location/address for the string changes with each keypress). In either case, the traversal down nodes to the final node will have a form similar to the following for the word "cat":

                              o
                              |-c
                              |-0
                  ------------+------------
                  |lokid      |eqkid      |hikid
                  o           o           o
                              |-a
                              |-0
                          ----+----
                          |   |   |    note: any of the lokid or hikid nodes
                              o              can also have pointers to nodes
                              |-t            for words that "cat" of "ca" is
                              |-0            a partial prefix to.
                          ----+----
                          |   |   |
                              o
                              |-0
                              |-1    <== the refcnt is only relevant to the final node
                          ----+----
                          |   |   |
                        NULL  o  NULL
                            "cat"

The ternary tree has the same O(n) efficiency for insert and search as does a bst. The delete is only slightly worse due to the proper deletion of the chain of unique nodes that make a word and proper rotation to eliminate the final node containing siblings. Lookup times associated with loading the entire `/usr/share/dict/words` file and searching range between `0.000002 - 0.000014` sec. However, the *prefix search* ability offered by the ternary search tree sets it apart from virtually all other data stuctures. While Tri/Radix trees can perform as well, their memory requirements are often 20 times more than a ternary tree.

The `/usr/share/dict/words` file (or `/var/lib/dict/words` on some systems) holds roughly 305000 words. You can load this file for testing, or I have also provided the first 1000 words from the file in `words1000.txt` if you are not on a Linux/Unix machine.

The files containing the ternary tree implementation are:

    ternary_st.h
    ternary_st.c

The following two test files provide a short menu driven application:

    tst_test_cpy.c   /* uses tst_ins_del_cpy() to store an allocated copy of word */
    tst_test_ref.c   /* uses tst_ins_del_ref() to store reference to word stored elsewhere */

The test application provides the following operations on the tree:

        p  print words in tree
        a  add word to the tree
        f  find word in tree
        s  search words matching prefix (enter 3 chars)
        d  delete word from the tree
        q  quit, freeing all data

(note: code skips printing tree with more than 100 words)

The final file `tst_validate.c` is a short torture test fully exercising and validating tree integrity. After the tree is filled, the array of pointer to words used to fill the tree are shuffled into a random order and `delete` is called on each word in the shuffled list. A further inner loop than validates that every remaining word and every pointer in the tree each time a deletion takes place. It is a validation routine, so the only output avoided is the following on success, on error, a dump of the deltetion order file and current list state is made. The successul output is similar to:

    $ tst_validate dat/dictwords_1000.txt
    ternary_search_tree, loaded, 1000 words.
    1000 successful deletions from search tree.

Compilation with full error checking and optimization is suggested, e.g.

    $ gcc -Wall -Wextra -pedantic -Wshadow -finline-functions -std=gnu11 -Ofast \
      ternary_st.c -o bin/ternary_st_val ternary_st_val.c

If you find any problems, let me know (or better yet provide a patch).
