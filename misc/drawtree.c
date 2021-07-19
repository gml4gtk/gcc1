/* checks of GNU GPL splay tree thingies as used in gcc-2 */
/* Draws ascii picture of a tree.  Allows the user to splay */
/* any node, and then redraws the tree.                     */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>


#define TRUE 1
#define FALSE 0
int size;


/*
 * This version of top-down-simple splay maintains parent pointers.
 */
typedef struct tree_node {
    struct tree_node * left, * right, * parent;
    int key;
} Tree;

#define set_parent_left(t) {if((t)->left != NULL) (t)->left->parent = (t);}
#define set_parent_right(t) {if((t)->right != NULL) (t)->right->parent = (t);}

Tree * splay (int i, Tree * t) {
    Tree N, *l, *r, *y;
    if (t == NULL) return t;
    N.left = N.right = NULL;
    l = r = &N;

    for (;;) {
        if (i < t->key) {
            if (t->left == NULL) break;
            if (i < t->left->key) {
                y = t->left;                           /* rotate right */
                t->left = y->right;
                if (t->left != NULL) t->left->parent = t;
                y->right = t;
                t->parent = y;
                t = y;
                if (t->left == NULL) break;
            }
            r->left = t;                               /* link right */
            t->parent = r;
            r = t;
            t = t->left;
        } else if (i > t->key) {
            if (t->right == NULL) break;
            if (i > t->right->key) {
                y = t->right;                          /* rotate left */
                t->right = y->left;
                if (t->right != NULL) t->right->parent = t;
                y->left = t;
                t->parent = y;
                t = y;
                if (t->right == NULL) break;
            }
            l->right = t;                              /* link left */
            t->parent = l;
            l = t;
            t = t->right;
        } else {
            break;
        }
    }
    l->right = t->left;                                /* assemble */
    if (l->right != NULL) l->right->parent = l;
    r->left = t->right;
    if (r->left != NULL) r->left->parent = r;
    t->left = N.right;
    if (t->left != NULL) t->left->parent = t;
    t->right = N.left;
    if (t->right != NULL) t->right->parent = t;
    t->parent = NULL;
    return t;
}

Tree * move_to_root (int i, Tree * t) {
    Tree N, *l, *r, *y;
    if (t == NULL) return t;
    N.left = N.right = NULL;
    l = r = &N;

    for (;;) {
        if (i < t->key) {
            if (t->left == NULL) break;
            r->left = t;                               /* link right */
            t->parent = r;
            r = t;
            t = t->left;
        } else if (i > t->key) {
            if (t->right == NULL) break;
            l->right = t;                              /* link left */
            t->parent = l;
            l = t;
            t = t->right;
        } else {
            break;
        }
    }
    l->right = t->left;                                /* assemble */
    if (l->right != NULL) l->right->parent = l;
    r->left = t->right;
    if (r->left != NULL) r->left->parent = r;
    t->left = N.right;
    if (t->left != NULL) t->left->parent = t;
    t->right = N.left;
    if (t->right != NULL) t->right->parent = t;
    t->parent = NULL;
    return t;
}

int check_tree(Tree * t) {
    if (t == NULL) return TRUE;
    if (t->left != NULL &&(t->left->parent != t || !check_tree(t->left))) {
        return FALSE;
    }
    if (t->right != NULL && (t->right->parent != t || !check_tree(t->right))) {
        return FALSE;
    }
    return TRUE;
}

#define INFINITY (1<<20)
#define LABLEN 20

int min (int X, int Y)  {return ((X) < (Y)) ? (X) : (Y);}
int max (int X, int Y)  {return ((X) > (Y)) ? (X) : (Y);}

typedef struct pnode_struct Pnode;

struct pnode_struct {
    Pnode * left, * right;
    int edge_length; /* length of the edge from this node to its children */
                     /* number of "\" or "/".  so it's at least 1         */
                     /* unless both children are null.  Then it's 0       */

    int height;      /* The number of rows required to print this tree */
    int lablen;
    int parent_dir;       /* -1=I am left, 0=I am root, 1=right       */
                          /* this is used to decide how to break ties */
                          /* when the label is of even length         */
    char label[LABLEN+1];
};

int allocs_in_use;
void my_free(void * p) {
    allocs_in_use--;
    free(p);
}
void * my_alloc(int size) {
    void * p = malloc(size);
    allocs_in_use++;
    if (p == NULL) {
        fprintf(stderr, "Ran out of space.  Requested size=%d.\n", size);
        exit(1);
    }
    return p;
}

/* Free all the nodes of the given tree */
void free_ptree(Pnode *pn) {
    if (pn == NULL) return;
    free_ptree(pn->left);
    free_ptree(pn->right);
    my_free(pn);
}

Pnode * build_ptree_rec(Tree * t) {
    Pnode * pn;
    if (t == NULL) return NULL;
    pn = my_alloc(sizeof(Pnode));
    pn->left = build_ptree_rec(t->left);
    pn->right = build_ptree_rec(t->right);
    if (pn->left != NULL) pn->left->parent_dir = -1;
    if (pn->right != NULL) pn->right->parent_dir = 1;

    sprintf(pn->label, "%d", t->key);
    pn->lablen = strlen(pn->label);
    return pn;
}

/*
 * Copy the tree from the original structure into the Pnode structure
 * fill in the parent_dir, label, and labelen fields, but not the
 * edge_length or height fields.
 */
Pnode * build_ptree(Tree * t) {
    Pnode *pn;
    if (t == NULL) return NULL;
    pn = build_ptree_rec(t);
    pn->parent_dir = 0;
    return pn;
}

/*
 * The lprofile array is description of the left profile of a tree.
 * Assuming the root is located at (0,0), lprofile[i] is the leftmost
 * point used on row i of the tree.  rprofile is similarly defined.
 */
#define MAX_HEIGHT 1000
int lprofile[MAX_HEIGHT];
int rprofile[MAX_HEIGHT];

/*
 * The following function fills in the lprofile array for the given tree.
 * It assumes that the center of the label of the root of this tree
 * is located at a position (x,y).  It assumes that the edge_length
 * fields have been computed for this tree.
 */
void compute_lprofile(Pnode *pn, int x, int y) {
    int i, isleft;
    if (pn == NULL) return;
    isleft = (pn->parent_dir == -1);
    lprofile[y] = min(lprofile[y], x-((pn->lablen-isleft)/2));
    if (pn->left != NULL) {
        for (i=1; i <= pn->edge_length && y+i < MAX_HEIGHT; i++) {
            lprofile[y+i] = min(lprofile[y+i], x-i);
        }
    }
    compute_lprofile(pn->left, x-pn->edge_length-1, y+pn->edge_length+1);
    compute_lprofile(pn->right, x+pn->edge_length+1, y+pn->edge_length+1);
}
void compute_rprofile(Pnode *pn, int x, int y) {
    int i, notleft;
    if (pn == NULL) return;
    notleft = (pn->parent_dir != -1);
    rprofile[y] = max(rprofile[y], x+((pn->lablen-notleft)/2));
    if (pn->right != NULL) {
        for (i=1; i <= pn->edge_length && y+i < MAX_HEIGHT; i++) {
            rprofile[y+i] = max(rprofile[y+i], x+i);
        }
    }
    compute_rprofile(pn->left, x-pn->edge_length-1, y+pn->edge_length+1);
    compute_rprofile(pn->right, x+pn->edge_length+1, y+pn->edge_length+1);
}

/*
 * This function fills in the edge_length and height fields of the
 * specified tree.
 */
void compute_edge_lengths(Pnode *pn) {
    int h, hmin, i, delta;
    if (pn == NULL) return;
    compute_edge_lengths(pn->left);
    compute_edge_lengths(pn->right);

    /* first fill in the edge_length of pn */
    if (pn->right == NULL && pn->left == NULL) {
        pn->edge_length = 0;
    } else {
        if (pn->left != NULL) {
            for (i=0; i<pn->left->height && i < MAX_HEIGHT; i++) {
                rprofile[i] = -INFINITY;
            }
            compute_rprofile(pn->left, 0, 0);
            hmin = pn->left->height;
        } else {
            hmin = 0;
        }
        if (pn->right != NULL) {
            for (i=0; i<pn->right->height && i < MAX_HEIGHT; i++) {
                lprofile[i] = INFINITY;
            }
            compute_lprofile(pn->right, 0, 0);
            hmin = min(pn->right->height, hmin);
        } else {
            hmin = 0;
        }
        delta = 4;
        for (i=0; i<hmin; i++) {
            delta = max(delta, 2 + 1 + rprofile[i] - lprofile[i]);
       /* the "2" guarantees a gap of 2 between different parts of the tree */
        }
        /* If the node has two children of height 1, then we allow the
           two leaves to be within 1, instead of 2 */
        if (((pn->left != NULL && pn->left->height == 1) ||
            (pn->right != NULL && pn->right->height == 1))&&delta>4) delta--;
        pn->edge_length = ((delta+1)/2) - 1;
    }

    /* now fill in the height of pn */
    h = 1;
    if (pn->left != NULL) {
        h = max(pn->left->height + pn->edge_length + 1, h);
    }
    if (pn->right != NULL) {
        h = max(pn->right->height + pn->edge_length + 1, h);
    }
    pn->height = h;
}

int print_next;  /* used by print_level.  If you call "printf()" at   */
                 /* any point, this is the x coordinate of the next   */
                 /* char printed.                                     */

/*
 * This function prints the given level of the given tree, assuming
 * that the node pn has the given x cordinate.
 */
void print_level(Pnode *pn, int x, int level) {
    int i, isleft;
    if (pn == NULL) return;
    isleft = (pn->parent_dir == -1);
    if (level == 0) {
        for (i=0; i<(x-print_next-((pn->lablen-isleft)/2)); i++) {
            printf(" ");
        }
        print_next += i;
        printf("%s", pn->label);
        print_next += pn->lablen;
    } else if (pn->edge_length >= level) {
        if (pn->left != NULL) {
            for (i=0; i<(x-print_next-(level)); i++) {
                printf(" ");
            }
            print_next += i;
            printf("/");
            print_next++;
        }
        if (pn->right != NULL) {
            for (i=0; i<(x-print_next+(level)); i++) {
                printf(" ");
            }
            print_next += i;
            printf("\\");
            print_next++;
        }
    } else {
        print_level(pn->left, x-pn->edge_length-1, level-pn->edge_length-1);
        print_level(pn->right, x+pn->edge_length+1, level-pn->edge_length-1);
    }
}

/*
 * This pretty-prints the given tree, left-justified.
 * The tree is drawn in such a way that both of the edges down from
 * a node are the same length.  This length is the minimum such that
 * the two subtrees are separated by at least two blanks.
 */
void pretty_print_tree(Tree * t) {
    Pnode *proot;
    int xmin, i;
    if (t == NULL) return;
    proot = build_ptree(t);
    compute_edge_lengths(proot);
    for (i=0; i<proot->height && i < MAX_HEIGHT; i++) {
        lprofile[i] = INFINITY;
    }
    compute_lprofile(proot, 0, 0);
    xmin = 0;
    for (i = 0; i < proot->height && i < MAX_HEIGHT; i++) {
        xmin = min(xmin, lprofile[i]);
    }
    for (i = 0; i < proot->height; i++) {
        print_next = 0;
        print_level(proot, -xmin, i);
        printf("\n");
    }
    if (proot->height >= MAX_HEIGHT) {
        printf("(This tree is taller than %d, and may be drawn incorrectly.)\n"
, MAX_HEIGHT);
    }
    free_ptree(proot);
}

Tree * insert(int i, Tree * t) {
/* Insert i into the tree t, unless it's already there.    */
/* Return a pointer to the resulting tree.                 */
    Tree * new;

    new = (Tree *) my_alloc (sizeof (Tree));
    new->key = i;
    if (t == NULL) {
        new->left = new->right = new->parent = NULL;
        size = 1;
        return new;
    }
    t = splay(i,t);
    if (i < t->key) {
        new->left = t->left;
        new->right = t;
        t->left = NULL;
        set_parent_left(new);
        set_parent_right(new);
        size ++;
        return new;
    } else if (i > t->key) {
        new->right = t->right;
        new->left = t;
        t->right = NULL;
        set_parent_left(new);
        set_parent_right(new);
        size++;
        return new;
    } else { /* We get here if it's already in the tree */
             /* Don't add it again                      */
        my_free(new);
        return t;
    }
}

Tree * delete(int i, Tree * t) {
/* Deletes i from the tree if it's there.               */
/* Return a pointer to the resulting tree.              */
    Tree * x;
    if (t==NULL) return NULL;
    t = splay(i,t);
    if (i == t->key) {               /* found it */
        if (t->left == NULL) {
            x = t->right;
        } else {
            x = splay(i, t->left);
            x->right = t->right;
            set_parent_right(x);
        }
        size--;
        my_free(t);
        return x;
    }
    return t;                         /* It wasn't there */
}

int main(int argc, char *argv[]) {
    Tree * root;
    char line[100];
    int i, N;
    root = NULL;              /* the empty tree */
    size = 0;
    while(TRUE) {
        printf("Enter the number of nodes in the tree [1, 200]: ");
        N = -1;
        if (fgets(line, sizeof(line), stdin) == NULL) exit(1);
        sscanf(line,"%d", &N);
        if ((N<1) || (N > 200)) {
            printf("Choose a number between 1 and 200.\n");
            continue;
        }
        break;
    }

    for (i = 0; i < N; i++) {
        root = insert(i, root);
        if (!check_tree(root)) printf("error\n");;
    }
    pretty_print_tree(root);
    for (;;) {
        printf("Select a node to splay: ");
        if (fgets(line, sizeof(line), stdin) == NULL) break;
        if ((sscanf(line, "%d", &i) == 1) && (i >= 0 && i < N)) {
            /*  root = move_to_root(i, root);  */
            root = splay(i, root);
        } else {
            if (strncasecmp(line, "quit", strlen("quit")) == 0) {
                break;
            } else {
                printf("Choose a number in [%d, %d] or type \"quit\".\n", 0, N-1);
                continue;
            }
        }
        pretty_print_tree(root);
    }
/*
    for (i = 0; i < N; i++) {
        root = delete((541*i) & (N-1), root);
        if (!check_tree(root)) printf("error\n");;
    }
    printf("size = %d\n", size);
*/
}

/* next is for documentation only - the original */
#if 0
/*
           An implementation of top-down splaying with sizes
             D. Sleator <[17]sleator@cs.cmu.edu>, January 1994.

  This extends top-down-splay.c to maintain a size field in each node.
  This is the number of nodes in the subtree rooted there.  This makes
  it possible to efficiently compute the rank of a key.  (The rank is
  the number of nodes to the left of the given key.)  It it also
  possible to quickly find the node of a given rank.  Both of these
  operations are illustrated in the code below.  The remainder of this
  introduction is taken from top-down-splay.c.

  "Splay trees", or "self-adjusting search trees" are a simple and
  efficient data structure for storing an ordered set.  The data
  structure consists of a binary tree, with no additional fields.  It
  allows searching, insertion, deletion, deletemin, deletemax,
  splitting, joining, and many other operations, all with amortized
  logarithmic performance.  Since the trees adapt to the sequence of
  requests, their performance on real access patterns is typically even
  better.  Splay trees are described in a number of texts and papers
  [1,2,3,4].

  The code here is adapted from simple top-down splay, at the bottom of
  page 669 of [2].  It can be obtained via anonymous ftp from
  spade.pc.cs.cmu.edu in directory /usr/sleator/public.

  The chief modification here is that the splay operation works even if the
  item being splayed is not in the tree, and even if the tree root of the
  tree is NULL.  So the line:

                              t = splay(i, t);

  causes it to search for item with key i in the tree rooted at t.  If it's
  there, it is splayed to the root.  If it isn't there, then the node put
  at the root is the last one before NULL that would have been reached in a
  normal binary search for i.  (It's a neighbor of i in the tree.)  This
  allows many other operations to be easily implemented, as shown below.

  [1] "Data Structures and Their Algorithms", Lewis and Denenberg,
       Harper Collins, 1991, pp 243-251.
  [2] "Self-adjusting Binary Search Trees" Sleator and Tarjan,
       JACM Volume 32, No 3, July 1985, pp 652-686.
  [3] "Data Structure and Algorithm Analysis", Mark Weiss,
       Benjamin Cummins, 1992, pp 119-130.
  [4] "Data Structures, Algorithms, and Performance", Derick Wood,
       Addison-Wesley, 1993, pp 367-375
*/

#include <stdio.h>

typedef struct tree_node Tree;
struct tree_node {
    Tree * left, * right;
    int key;
    int size;   /* maintained to be the number of nodes rooted here */
};

#define compare(i,j) ((i)-(j))
/* This is the comparison.                                       */
/* Returns <0 if i<j, =0 if i=j, and >0 if i>j                   */

#define node_size(x) (((x)==NULL) ? 0 : ((x)->size))
/* This macro returns the size of a node.  Unlike "x->size",     */
/* it works even if x=NULL.  The test could be avoided by using  */
/* a special version of NULL which was a real node with size 0.  */

Tree * splay (int i, Tree *t)
/* Splay using the key i (which may or may not be in the tree.) */
/* The starting root is t, and the tree used is defined by rat  */
/* size fields are maintained */
{
    Tree N, *l, *r, *y;
    int comp, root_size, l_size, r_size;

    if (t == NULL) return t;
    N.left = N.right = NULL;
    l = r = &N;
    root_size = node_size(t);
    l_size = r_size = 0;

    for (;;) {
        comp = compare(i, t->key);
        if (comp < 0) {
            if (t->left == NULL) break;
            if (compare(i, t->left->key) < 0) {
                y = t->left;                           /* rotate right */
                t->left = y->right;
                y->right = t;
                t->size = node_size(t->left) + node_size(t->right) + 1;
                t = y;
                if (t->left == NULL) break;
            }
            r->left = t;                               /* link right */
            r = t;
            t = t->left;
            r_size += 1+node_size(r->right);
        } else if (comp > 0) {
            if (t->right == NULL) break;
            if (compare(i, t->right->key) > 0) {
                y = t->right;                          /* rotate left */
                t->right = y->left;
                y->left = t;
                t->size = node_size(t->left) + node_size(t->right) + 1;
                t = y;
                if (t->right == NULL) break;
            }
            l->right = t;                              /* link left */
            l = t;
            t = t->right;
            l_size += 1+node_size(l->left);
        } else {
            break;
        }
    }
    l_size += node_size(t->left);  /* Now l_size and r_size are the sizes of */
    r_size += node_size(t->right); /* the left and right trees we just built.*/
    t->size = l_size + r_size + 1;

    l->right = r->left = NULL;

    /* The following two loops correct the size fields of the right path  */
    /* from the left child of the root and the right path from the left   */
    /* child of the root.                                                 */
    for (y = N.right; y != NULL; y = y->right) {
        y->size = l_size;
        l_size -= 1+node_size(y->left);
    }
    for (y = N.left; y != NULL; y = y->left) {
        y->size = r_size;
        r_size -= 1+node_size(y->right);
    }

    l->right = t->left;                                /* assemble */
    r->left = t->right;
    t->left = N.right;
    t->right = N.left;

    return t;
}

Tree * insert(int i, Tree * t) {
/* Insert key i into the tree t, if it is not already there. */
/* Return a pointer to the resulting tree.                   */
    Tree * new;

    if (t != NULL) {
        t = splay(i,t);
        if (compare(i, t->key)==0) {
            return t;  /* it's already there */
        }
    }
    new = (Tree *) malloc (sizeof (Tree));
    if (new == NULL) {printf("Ran out of space\n"); exit(1);}
    if (t == NULL) {
        new->left = new->right = NULL;
    } else if (compare(i, t->key) < 0) {
        new->left = t->left;
        new->right = t;
        t->left = NULL;
        t->size = 1+node_size(t->right);
    } else {
        new->right = t->right;
        new->left = t;
        t->right = NULL;
        t->size = 1+node_size(t->left);
    }
    new->key = i;
    new->size = 1 + node_size(new->left) + node_size(new->right);
    return new;
}

Tree * delete(int i, Tree *t) {
/* Deletes i from the tree if it's there.               */
/* Return a pointer to the resulting tree.              */
    Tree * x;
    int tsize;

    if (t==NULL) return NULL;
    tsize = t->size;
    t = splay(i,t);
    if (compare(i, t->key) == 0) {               /* found it */
        if (t->left == NULL) {
            x = t->right;
        } else {
            x = splay(i, t->left);
            x->right = t->right;
        }
        free(t);
        if (x != NULL) {
            x->size = tsize-1;
        }
        return x;
    } else {
        return t;                         /* It wasn't there */
    }
}

Tree *find_rank(int r, Tree *t) {
/* Returns a pointer to the node in the tree with the given rank.  */
/* Returns NULL if there is no such node.                          */
/* Does not change the tree.  To guarantee logarithmic behavior,   */
/* the node found here should be splayed to the root.              */
    int lsize;
    if ((r < 0) || (r >= node_size(t))) return NULL;
    for (;;) {
        lsize = node_size(t->left);
        if (r < lsize) {
            t = t->left;
        } else if (r > lsize) {
            r = r - lsize -1;
            t = t->right;
        } else {
            return t;
        }
    }
}

void printtree(Tree * t, int d) {
    int i;
    if (t == NULL) return;
    printtree(t->right, d+1);
    for (i=0; i<d; i++) printf("  ");
    printf("%d(%d)\n", t->key, t->size);
    printtree(t->left, d+1);
}

void main() {
/* A sample use of these functions.  Start with the empty tree,         */
/* insert some stuff into it, and then delete it                        */
    Tree * root, *t;
    int i;
    root = NULL;              /* the empty tree */
    for (i = 0; i < 100; i++) {
        root = insert((541*i) & (1023), root);
    }

    printtree(root, 0);

    for (i = -1; i<=100; i++) {
        t = find_rank(i, root);
        if (t == NULL) {
            printf("could not find a node of rank %d.\n", i);
        } else {
            printf("%d is of rank %d\n", t->key, i);
        }
    }

    printtree(root, 0);

    for (i=0; i<1000; i=i+20) {
        root = splay(i, root);
        printf("There are %d elements to the left of %d in the set.\n",
               node_size(root->left), i);
    }

    for (i = 0; i < 100; i++) {
        root = delete((541*i) & (1023), root);
    }
}

--------------4F1EDD890ABC2CF8BE49B65B
Content-Type: text/plain; charset=us-ascii;
 name="top-down-splay.c"
Content-Transfer-Encoding: 7bit
Content-Disposition: inline;
 filename="top-down-splay.c"

/*
                An implementation of top-down splaying
                    D. Sleator <[18]sleator@cs.cmu.edu>
                             March 1992

  "Splay trees", or "self-adjusting search trees" are a simple and
  efficient data structure for storing an ordered set.  The data
  structure consists of a binary tree, without parent pointers, and no
  additional fields.  It allows searching, insertion, deletion,
  deletemin, deletemax, splitting, joining, and many other operations,
  all with amortized logarithmic performance.  Since the trees adapt to
  the sequence of requests, their performance on real access patterns is
  typically even better.  Splay trees are described in a number of texts
  and papers [1,2,3,4,5].

  The code here is adapted from simple top-down splay, at the bottom of
  page 669 of [3].  It can be obtained via anonymous ftp from
  spade.pc.cs.cmu.edu in directory /usr/sleator/public.

  The chief modification here is that the splay operation works even if the
  item being splayed is not in the tree, and even if the tree root of the
  tree is NULL.  So the line:

                              t = splay(i, t);

  causes it to search for item with key i in the tree rooted at t.  If it's
  there, it is splayed to the root.  If it isn't there, then the node put
  at the root is the last one before NULL that would have been reached in a
  normal binary search for i.  (It's a neighbor of i in the tree.)  This
  allows many other operations to be easily implemented, as shown below.

  [1] "Fundamentals of data structures in C", Horowitz, Sahni,
       and Anderson-Freed, Computer Science Press, pp 542-547.
  [2] "Data Structures and Their Algorithms", Lewis and Denenberg,
       Harper Collins, 1991, pp 243-251.
  [3] "Self-adjusting Binary Search Trees" Sleator and Tarjan,
       JACM Volume 32, No 3, July 1985, pp 652-686.
  [4] "Data Structure and Algorithm Analysis", Mark Weiss,
       Benjamin Cummins, 1992, pp 119-130.
  [5] "Data Structures, Algorithms, and Performance", Derick Wood,
       Addison-Wesley, 1993, pp 367-375.
*/

#include <stdio.h>

int size;  /* number of nodes in the tree */
           /* Not actually needed for any of the operations */

typedef struct tree_node Tree;
struct tree_node {
    Tree * left, * right;
    int item;
};

Tree * splay (int i, Tree * t) {
/* Simple top down splay, not requiring i to be in the tree t.  */
/* What it does is described above.                             */
    Tree N, *l, *r, *y;
    if (t == NULL) return t;
    N.left = N.right = NULL;
    l = r = &N;

    for (;;) {
        if (i < t->item) {
            if (t->left == NULL) break;
            if (i < t->left->item) {
                y = t->left;                           /* rotate right */
                t->left = y->right;
                y->right = t;
                t = y;
                if (t->left == NULL) break;
            }
            r->left = t;                               /* link right */
            r = t;
            t = t->left;
        } else if (i > t->item) {
            if (t->right == NULL) break;
            if (i > t->right->item) {
                y = t->right;                          /* rotate left */
                t->right = y->left;
                y->left = t;
                t = y;
                if (t->right == NULL) break;
            }
            l->right = t;                              /* link left */
            l = t;
            t = t->right;
        } else {
            break;
        }
    }
    l->right = t->left;                                /* assemble */
    r->left = t->right;
    t->left = N.right;
    t->right = N.left;
    return t;
}

/* Here is how sedgewick would have written this.                    */
/* It does the same thing.                                           */
Tree * sedgewickized_splay (int i, Tree * t) {
    Tree N, *l, *r, *y;
    if (t == NULL) return t;
    N.left = N.right = NULL;
    l = r = &N;

    for (;;) {
        if (i < t->item) {
            if (t->left != NULL && i < t->left->item) {
                y = t->left; t->left = y->right; y->right = t; t = y;
            }
            if (t->left == NULL) break;
            r->left = t; r = t; t = t->left;
        } else if (i > t->item) {
            if (t->right != NULL && i > t->right->item) {
                y = t->right; t->right = y->left; y->left = t; t = y;
            }
            if (t->right == NULL) break;
            l->right = t; l = t; t = t->right;
        } else break;
    }
    l->right=t->left; r->left=t->right; t->left=N.right; t->right=N.left;
    return t;
}

Tree * insert(int i, Tree * t) {
/* Insert i into the tree t, unless it's already there.    */
/* Return a pointer to the resulting tree.                 */
    Tree * new;

    new = (Tree *) malloc (sizeof (Tree));
    if (new == NULL) {
        printf("Ran out of space\n");
        exit(1);
    }
    new->item = i;
    if (t == NULL) {
        new->left = new->right = NULL;
        size = 1;
        return new;
    }
    t = splay(i,t);
    if (i < t->item) {
        new->left = t->left;
        new->right = t;
        t->left = NULL;
        size ++;
        return new;
    } else if (i > t->item) {
        new->right = t->right;
        new->left = t;
        t->right = NULL;
        size++;
        return new;
    } else { /* We get here if it's already in the tree */
             /* Don't add it again                      */
        free(new);
        return t;
    }
}

Tree * delete(int i, Tree * t) {
/* Deletes i from the tree if it's there.               */
/* Return a pointer to the resulting tree.              */
    Tree * x;
    if (t==NULL) return NULL;
    t = splay(i,t);
    if (i == t->item) {               /* found it */
        if (t->left == NULL) {
            x = t->right;
        } else {
            x = splay(i, t->left);
            x->right = t->right;
        }
        size--;
        free(t);
        return x;
    }
    return t;                         /* It wasn't there */
}

int main(int argc, char *argv[]) {
/* A sample use of these functions.  Start with the empty tree,         */
/* insert some stuff into it, and then delete it                        */
    Tree * root;
    int i;
    root = NULL;              /* the empty tree */
    size = 0;
    for (i = 0; i < 1024; i++) {
        root = insert((541*i) & (1023), root);
    }
    for (i = 0; i < 1024; i++) {
        root = delete((541*i) & (1023), root);
    }
    printf("size = %d\n", size);
}

#endif
