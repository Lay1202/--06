#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_DATA   100
#define NUM_SEARCH 50
#define MAX_VALUE  1000
#define DEFAULT_SEED 20240607u

typedef struct Node {
    int key;
    int height;
    struct Node* left;
    struct Node* right;
} Node;

static Node* new_node(int key)
{
    Node* n = (Node*)malloc(sizeof(Node));
    if (n == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    n->key = key;
    n->height = 1;
    n->left = n->right = NULL;
    return n;
}

static void free_tree(Node* root)
{
    if (root == NULL) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

static int tree_height(const Node* root)
{
    if (root == NULL) return 0;
    int l = tree_height(root->left);
    int r = tree_height(root->right);
    return (l > r ? l : r) + 1;
}

static int sequential_search(const int* arr, int len, int key, long* cmp)
{
    for (int i = 0; i < len; i++) {
        (*cmp)++;
        if (arr[i] == key) return 1;
    }
    return 0;
}

static int array_insert(int* arr, int* len, int key, long* cmp)
{
    if (sequential_search(arr, *len, key, cmp)) return 0;
    arr[*len] = key;
    (*len)++;
    return 1;
}

static int bst_insert(Node** root, int key, long* cmp)
{
    Node** link = root;
    while (*link != NULL) {
        (*cmp)++;
        if (key == (*link)->key) return 0;
        if (key < (*link)->key) link = &(*link)->left;
        else                    link = &(*link)->right;
    }
    *link = new_node(key);
    return 1;
}

static int tree_search(const Node* root, int key, long* cmp)
{
    const Node* cur = root;
    while (cur != NULL) {
        (*cmp)++;
        if (key == cur->key) return 1;
        cur = (key < cur->key) ? cur->left : cur->right;
    }
    return 0;
}

static long rot_LL = 0, rot_RR = 0, rot_LR = 0, rot_RL = 0;

static int node_height(const Node* n) { return n ? n->height : 0; }

static void update_height(Node* n)
{
    int l = node_height(n->left);
    int r = node_height(n->right);
    n->height = (l > r ? l : r) + 1;
}

static int balance_factor(const Node* n)
{
    return n ? node_height(n->left) - node_height(n->right) : 0;
}

static Node* rotate_right(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    x->right = y;
    update_height(y);
    update_height(x);
    return x;
}

static Node* rotate_left(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    y->left = x;
    update_height(x);
    update_height(y);
    return y;
}

static Node* avl_insert(Node* node, int key, long* cmp, int* inserted)
{
    if (node == NULL) {
        *inserted = 1;
        return new_node(key);
    }

    (*cmp)++;
    if (key == node->key) {
        *inserted = 0;
        return node;
    }
    if (key < node->key) node->left = avl_insert(node->left, key, cmp, inserted);
    else                 node->right = avl_insert(node->right, key, cmp, inserted);

    if (!*inserted) return node;

    update_height(node);
    int bf = balance_factor(node);

    if (bf > 1) {
        if (balance_factor(node->left) >= 0) {
            rot_LL++;
            return rotate_right(node);
        }
        else {
            rot_LR++;
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }
    }
    if (bf < -1) {
        if (balance_factor(node->right) <= 0) {
            rot_RR++;
            return rotate_left(node);
        }
        else {
            rot_RL++;
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }
    }
    return node;
}

static int inorder(const Node* n, int* out, int idx)
{
    if (n == NULL) return idx;
    idx = inorder(n->left, out, idx);
    out[idx++] = n->key;
    return inorder(n->right, out, idx);
}

static int is_bst(const Node* n, int lo, int hi)
{
    if (n == NULL) return 1;
    if (n->key <= lo || n->key >= hi) return 0;
    return is_bst(n->left, lo, n->key) && is_bst(n->right, n->key, hi);
}

static int is_balanced(const Node* n)
{
    if (n == NULL) return 1;
    int diff = tree_height(n->left) - tree_height(n->right);
    if (diff < -1 || diff > 1) return 0;
    return is_balanced(n->left) && is_balanced(n->right);
}

static int cmp_int(const void* a, const void* b)
{
    int x = *(const int*)a, y = *(const int*)b;
    return (x > y) - (x < y);
}

static const char* commas(long v, char* buf)
{
    char tmp[32];
    int len = 0;
    int out = 0;
    if (v == 0) tmp[len++] = '0';
    while (v > 0) {
        tmp[len++] = (char)('0' + v % 10);
        v /= 10;
    }
    for (int i = len - 1; i >= 0; i--) {
        buf[out++] = tmp[i];
        if (i > 0 && i % 3 == 0) buf[out++] = ',';
    }
    buf[out] = '\0';
    return buf;
}

static void print_line_num(const char* label, long v)
{
    char b[32];
    printf("%-20s: %s\n", label, commas(v, b));
}

int main(int argc, char* argv[])
{
    unsigned int seed = DEFAULT_SEED;
    if (argc >= 2) seed = (unsigned int)strtoul(argv[1], NULL, 10);
    srand(seed);

    int data[NUM_DATA];
    int keys[NUM_SEARCH];

    int arr[NUM_DATA];
    int arr_len = 0;
    Node* bst = NULL;
    Node* avl = NULL;

    long arr_cmp = 0, bst_cmp = 0, avl_cmp = 0;
    int stored = 0, duplicates = 0;

    for (int i = 0; i < NUM_DATA; i++)
        data[i] = rand() % (MAX_VALUE + 1);

    printf("Seed : %u\n\n", seed);
    printf("Generated %d integers\n", NUM_DATA);
    for (int i = 0; i < NUM_DATA; i++) {
        printf("%4d", data[i]);
        if ((i + 1) % 10 == 0) printf("\n");
    }
    printf("\n");

    for (int i = 0; i < NUM_DATA; i++) {
        int ins_arr = array_insert(arr, &arr_len, data[i], &arr_cmp);
        int ins_bst = bst_insert(&bst, data[i], &bst_cmp);
        int ins_avl = 0;
        avl = avl_insert(avl, data[i], &avl_cmp, &ins_avl);

        if (ins_arr != ins_bst || ins_bst != ins_avl) {
            fprintf(stderr, "Error: insertion results differ between structures (value %d)\n", data[i]);
            return EXIT_FAILURE;
        }
        if (ins_arr) stored++;
        else         duplicates++;
    }

    int bst_h = tree_height(bst);
    int avl_h = tree_height(avl);

    printf("Stored values      : %d\n", stored);
    printf("Duplicates skipped : %d\n\n", duplicates);

    printf("Construction\n");
    print_line_num("Array comparisons", arr_cmp);
    print_line_num("BST comparisons", bst_cmp);
    print_line_num("AVL comparisons", avl_cmp);

    printf("\nStructure\n");
    printf("%-14s: %d\n", "Array length", arr_len);
    printf("%-14s: %d\n", "BST height", bst_h);
    printf("%-14s: %d\n", "AVL height", avl_h);

    printf("\nAVL rotations (reference)\n");
    printf("LL : %ld, RR : %ld, LR : %ld, RL : %ld (total %ld)\n",
        rot_LL, rot_RR, rot_LR, rot_RL, rot_LL + rot_RR + rot_LR + rot_RL);

    {
        int sorted_arr[NUM_DATA], in_bst[NUM_DATA], in_avl[NUM_DATA];
        memcpy(sorted_arr, arr, sizeof(int) * arr_len);
        qsort(sorted_arr, arr_len, sizeof(int), cmp_int);
        int nb = inorder(bst, in_bst, 0);
        int na = inorder(avl, in_avl, 0);

        int ok = (nb == arr_len && na == arr_len &&
            memcmp(sorted_arr, in_bst, sizeof(int) * arr_len) == 0 &&
            memcmp(sorted_arr, in_avl, sizeof(int) * arr_len) == 0 &&
            is_bst(bst, -1, MAX_VALUE + 1) &&
            is_bst(avl, -1, MAX_VALUE + 1) &&
            is_balanced(avl));
        printf("\nVerification (same value set / BST property / AVL balance) : %s\n",
            ok ? "OK" : "FAILED");
        if (!ok) return EXIT_FAILURE;
    }

    for (int i = 0; i < NUM_SEARCH; i++)
        keys[i] = rand() % (MAX_VALUE + 1);

    printf("\nSearches : %d\n", NUM_SEARCH);
    printf("Search keys\n");
    for (int i = 0; i < NUM_SEARCH; i++) {
        printf("%4d", keys[i]);
        if ((i + 1) % 10 == 0) printf("\n");
    }
    printf("\n");

    long seq_total = 0, bst_total = 0, avl_total = 0;
    int found_cnt = 0;

    for (int i = 0; i < NUM_SEARCH; i++) {
        long c_seq = 0, c_bst = 0, c_avl = 0;
        int r_seq = sequential_search(arr, arr_len, keys[i], &c_seq);
        int r_bst = tree_search(bst, keys[i], &c_bst);
        int r_avl = tree_search(avl, keys[i], &c_avl);

        if (r_seq != r_bst || r_bst != r_avl) {
            fprintf(stderr, "Error: search results differ between structures (key %d)\n", keys[i]);
            return EXIT_FAILURE;
        }
        if (r_seq) found_cnt++;

        seq_total += c_seq;
        bst_total += c_bst;
        avl_total += c_avl;

        printf("[%2d] Search Key : %d\n\n", i + 1, keys[i]);
        printf("Sequential Search\n");
        printf("Result      : %s\n", r_seq ? "Found" : "Not Found");
        printf("Comparisons : %ld\n\n", c_seq);
        printf("BST Search\n");
        printf("Result      : %s\n", r_bst ? "Found" : "Not Found");
        printf("Comparisons : %ld\n\n", c_bst);
        printf("AVL Search\n");
        printf("Result      : %s\n", r_avl ? "Found" : "Not Found");
        printf("Comparisons : %ld\n\n", c_avl);
        printf("----------------------------------------\n\n");
    }

    printf("Search summary (found %d / not found %d)\n\n",
        found_cnt, NUM_SEARCH - found_cnt);

    char b[32];
    printf("Sequential Search\n");
    printf("Total comparisons   : %s\n", commas(seq_total, b));
    printf("Average comparisons : %.2f\n\n", (double)seq_total / NUM_SEARCH);

    printf("BST Search\n");
    printf("Total comparisons   : %s\n", commas(bst_total, b));
    printf("Average comparisons : %.2f\n\n", (double)bst_total / NUM_SEARCH);

    printf("AVL Search\n");
    printf("Total comparisons   : %s\n", commas(avl_total, b));
    printf("Average comparisons : %.2f\n", (double)avl_total / NUM_SEARCH);

    free_tree(bst);
    free_tree(avl);
    return 0;
}
