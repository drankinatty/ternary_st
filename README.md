**Ternary Search Tree for Prefix-Search**

Copyright David C. Rankin, J.D.,P.E. 2017

Licensed Under GPLv2 available here

https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html

A *ternary search tree (tst)* is a binary search tree (bst) with an additional node pointer.

There is a relative dearth of information available about ternary trees, and specifically, proper node-rotation when a node is deleted from the tree. The only examples for a node-delete found are simple deletes that leave the tree dirty by leaving a node, with or without siblings, but having no valid middle (or equal) pointer. The following code, written as part of a text-editor word-completion implementation, provides a proper delete with rotation of the low or high node in place of the deleted node.

*Ternary Search Tree Intro*

With a binary search tree each node contains a *left* and *right* node-pointer so a binary choice controls traversal. Either a *greater than* or *less than* choice. As the result of a comparison between the node-data and a reference either the left or right node-pointer is used to further descend.

A ternary search tree adds a third (or middle) node. While you can still use the `left-middle-right` notation, ternary trees use a `low-equal-high` pointer verbiage. With each node holding a *key ID*, the node pointers for this code uses a `lokid`, `eqkid`, and `hikid` pointer naming convention. If the difference between a reference and the node key is negative (lower than), the `lokid` node is traversed, if they are equal, the `eqkid` node is followed or `hikid` is followed. The addition of the `equal` pointer and storage of character data as the `node->key` makes the ternary search tree optimal for string and prefix searching.

*Tree-node Used In This Code*

In addition to the `node->key`, the node used in this example adds a *reference count* (a `node->refcnt`) to the node data to track the number of occurrences for each word it holds. So for example, if using the tree to track the words in an editor buffer (where there may be multiple occurrences of 'the' or other common words), the node holding 'the' is not deleted, until no other occurrences remain (i.e. the `node->refcnt` is zero).

Each individual node has the following form:

                              o
                              |-key
                              |-refcnt
                  ------------+------------
                  |lokid      |eqkid      |hikid
                  o           o           o


The string data (a pointer to a word, or a copy of the word itself) is stored as a pointer in an additional/special node following the node containing the last character (node->key) in the search path. The pointer to the data is cast to type (node *) and stored as the `node->eqkid` pointer. Further, since this is the *node after the last character*, similar to the end of a string, its key is the *nul-character* (decimal `0`). The 'key's for each of the nodes that make up the search path of a word, are the letters in the word with the final node having a key `0` with either a pointer to the string (if stored in an external data structure) or an allocated copy of the string itself if the string is to be stored in the tree. (as in holding the words for an edit buffer, where the location/address for the string changes with each keypress). In either case, the traversal to the final node will have a form similar to the following for the word "cat":

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
                              |-t            for words that "cat" or "ca" is
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

The ternary tree has the same O(n) efficiency for insert and search as does a bst. A dirty delete from the tree is O(n). This delete with rotation is only slightly less efficient due to the proper deletion of the chain of all unique nodes in the search path and proper rotation. Lookup times associated with loading the entire `/usr/share/dict/words` file and searching range between `0.000002 - 0.000014` sec. However, the *prefix search* ability offered by the ternary search tree sets it apart from virtually all other data structures. While Tri/Radix trees can perform as well, their memory requirements to cover all ASCII characters are often 20 times that of a ternary tree.

*Example words File Provided (or use dict/words)*

The `/usr/share/dict/words` file (or `/var/lib/dict/words` on some systems) holds roughly 305000 words (including plural possessives), and roughly 249092 words without. You can load the full words file for testing, or I have also provided the first 1000 words from the file in `words1000.txt` if you are not on a Linux/Unix machine. There is also a nice [english-words](https://github.com/dwyl/english-words/blob/master/words_alpha.txt) list here on github with `370099` words (up to 31-chars, e.g. `"dichlorodiphenyltrichloroethane"`) that can be used with the test programs provided along with this code. (note the `words_alpha.txt` file has DOS line endings). The test routines handle either DOS of POSIX line endings.

*Ternary Search Tree Source Files*

The files containing the ternary tree implementation are:

    ternary_st.h
    ternary_st.c

*Interactive Example Programs*

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

*Tree Validation Program*

The final file `tst_validate.c` is a short torture test fully exercising and validating tree integrity. After the tree is filled, the array of pointer to words used to fill the tree are shuffled into a random order and `delete` is called on each word in the shuffled list. A further inner loop than validates that every remaining word and every pointer in the tree each time a deletion takes place. It is a validation routine, so the only output provided is the following on success, on error, a dump of the deletion order file and current list state is made. The successful output is similar to:

    $ tst_validate dat/dictwords_1000.txt
    ternary_search_tree, loaded, 1000 words.
    1000 successful deletions from search tree.

*Compilation*

Compilation with full error checking and optimization is suggested, e.g. and a Makefile is provided that will build the `ternary_st.o` object file and then compile all test programs placing the executables in a `./bin` subdirectory. You can individually compile any of the test programs similar to the following:

    $ gcc -Wall -Wextra -pedantic -Wshadow -finline-functions -std=c11 -Ofast \
      ternary_st.c -o bin/ternary_st_val ternary_st_val.c

*Prefix Searching*

The benefit of a ternary tree for prefix searching of text lies in its ability to quickly traverse a tree of any size finding the node containing the last character in the wanted prefix. An in-order traversal of that node identifies all strings in the tree containing the prefix. For example, in the `words1000.txt` file, you can locate all words beginning with `abr` near instantaneously, e.g.

    $ ./bin/tst_test_cpy words1000.txt
    ternary_tree, loaded 1000 words in 0.001965 sec

     p  print words in tree
     a  add word to the tree
     f  find word in tree
     s  search words matching prefix (enter 3 chars)
     d  delete word from the tree
     q  quit, freeing all data

    choice: s
    find words matching prefix (3 chars): abr
      abr - searched prefix in 0.000008 sec

    suggest[0] : abrade
    suggest[1] : abrasion
    suggest[2] : abrasive
    suggest[3] : abreact
    suggest[4] : abreast
    suggest[5] : abridge
    suggest[6] : abridgment
    suggest[7] : abroad
    suggest[8] : abrogate
    suggest[9] : abrupt

The search and prefix search times remain fast as the size of the tree increases. For example, adding all 249092 word from `/usr/share/dict/words` (not containing apostrophes), while there are eight times as many words with the `abr` prefix, the time to acquire an array of pointers to all prefixed words increases only slightly, e.g.

    $ ./bin/tst_test_cpy dat/words
    ternary_tree, loaded 249092 words in 0.186725 sec

     p  print words in tree
     a  add word to the tree
     f  find word in tree
     s  search words matching prefix (enter 3 chars)
     d  delete word from the tree
     q  quit, freeing all data

    choice: s
    find words matching prefix (3 chars): abr
      abr - searched prefix in 0.000051 sec

    suggest[0] : abrégé
    suggest[1] : abr
    suggest[2] : abracadabra
    suggest[3] : abrachia
    suggest[4] : abradable
    suggest[5] : abradant
    suggest[6] : abrade
    <snip>
    suggest[67] : abrupter
    suggest[68] : abruptest
    suggest[69] : abruption
    suggest[70] : abruptly
    suggest[71] : abruptness
    suggest[72] : abruptnesses

**Changes**

With the addition of a `Makefile` the source tree has been reorganized to separate the source and include files into separate directories as well as moving the example file to the `dat` subdirectory. The source tree layout is now:

    ├── Makefile
    ├── Makefile.lib
    ├── README.md
    ├── dat
    │   └── words1000.txt
    ├── include
    │   └── ternary_st.h
    └── src
        ├── ternary_st.c
        ├── tst_test_cpy.c
        ├── tst_test_ref.c
        └── tst_validate.c

`strdup` has been removed and replaced with standard `strlen, malloc, memcpy` to address the removal of `strdup` from the C99 standard. While many compilers provide `strdup` as an extension, as was originally provided by `-std=gnu11`, the code is now C11 standard compliant and backward compatible with prior versions.

An additional makefile `Makefile.lib` was added which builds a Linux shared-object library from the `ternary_st` source. The original makefile builds the .so by default by calling `Makefile.lib` within the `all:` rule. You can build-only the shared-object library by invoking make as `make libternary_st` or by simply passing the `-f` option to call make on the library makefile with `make -f Makefile.lib`.

Adding `install` (and running with elevated privileges) will install the shared-object library in `$(PREFIX)/$(INSTDIR)` (`/usr/local/lib64` by default). The configuration file containing the path is installed in `/etc/ld.so.conf.d/ternary_st.conf` and `ldconfig` is run to update the linker shared-object library cache.

If you find any problems, let me know (or better yet, create a pull-request and provide a patch).
