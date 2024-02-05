#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "../include/common.h"
#include "../include/list.h"
#include "../include/map.h"
#include "../include/set.h"
#include "../include/printing.h"
#include <string.h>
#include <ctype.h>


#define ERRMSG_SIZE 128
#define TOKEN_SIZE 32

 /*
 Copied from treeset.c. Uses in abstrax synxax tree building fucntions as their result. 
 */
struct treenode;
typedef struct treenode treenode_t;
struct treenode {
    treenode_t *left;
    treenode_t *right;
    treenode_t *next;
    void *elem;
};

/*
Basic struct in index.c
*/
typedef struct index {
	map_t* map; 
    set_t* set;
} index_t;

typedef struct query_result {
	char *path;    /* Document path */
	double score;  /* Document to query score */
}query_result_t;

/*
There were created 2 different structs: query_result и query_result_with_words. 
The second one contains struct query_result as well as the list of words it contains.
*/
typedef struct query_result_with_words {
    query_result_t* query_result;
    list_t* words;
} query_result_with_words_t;

 /*
 Copied from linkedlist.c
 */
struct listnode;
typedef struct listnode listnode_t;
struct listnode {
    listnode_t *next;
    listnode_t *prev;
    void *elem;
};

 /*
 Copied from linkedlist.c
 */
struct list {
    listnode_t *head;
    listnode_t *tail;
    int size;
    cmpfunc_t cmpfunc;
};
 /*
 Copied from assert_index.c
 */
typedef
struct document{
    list_t *words;
    char path[128];
}document_t;


/*
 * Creates a new, empty index.
 */
index_t *index_create(){
    index_t* index = (index_t*)malloc(sizeof(index_t));
    if (index == NULL){
        return NULL;
    }
    index->map = map_create(compare_strings, hash_string);
    index->set = set_create(compare_strings);
    if (index->map == NULL){
        free(index);
        return NULL;
    }
    return index;
}

/*
A wrapper function for set_destroy_and_destroy_items so it takes void and returns void
*/
void set_destroy_wrapper (void* set){
	set_destroy_and_destroy_items((set_t*)set, free);
}

/*
 * Destroys the given index. Subsequently accessing the index will
 * lead to undefined behavior.
 */
void index_destroy(index_t *index){
    map_destroy(index->map, free, set_destroy_wrapper);
    free(index);
}


// Declare the functions
treenode_t* query_func(listnode_t* first, listnode_t** last, char** errmsg);
treenode_t* andterm(listnode_t* first, listnode_t** last, char** errmsg);
treenode_t* orterm(listnode_t* first, listnode_t** last, char** errmsg);
treenode_t* term(listnode_t* first, listnode_t** last, char** errmsg);
void destroy_treenode(treenode_t* treenode);
treenode_t *newnode(void *elem);
int compare_documents(void* a, void* b);

/*
 * Adds the given path to the given index, and index the given
 * list of words under that path.
 * NOTE: It is the responsibility of index_addpath() to deallocate (free)
 * 'path' and the contents of the 'words' list.
 */

void index_addpath(index_t *index, char *path, list_t *words){
    list_t* words_copy = list_create(compare_strings);
    listnode_t* current = words->head;
    while (current != NULL){
        list_addlast(words_copy, current->elem);
        current = current->next;
    }
    list_iter_t* list_iter = list_createiter(words_copy);
	while (list_hasnext(list_iter)){
		void* item = list_next(list_iter);    
		
        if (map_haskey(index->map, item)){
            document_t* document = (document_t*)malloc(sizeof(document_t));
            if (document == NULL){
                //maybe need to free memory
                return;
            }
            strcpy(document->path, path);
            document->words = words_copy;
            set_add((set_t*)map_get(index->map, item), document);
        }
        else {
            char* item_copy = strdup((char*)item);
            if (item_copy != NULL){
                set_t* files = set_create(compare_documents);
                map_put(index->map, item_copy, files);
                document_t* document = (document_t*)malloc(sizeof(document_t));
                if (document == NULL){
                    return;
                }
                strcpy(document->path, path);
                document->words = words_copy;
                set_add(files, document);
            }
        }
	}
    set_add(index->set, path);
}



/*
Helper function. Basically destroys a treenode
*/
void destroy_treenode(treenode_t* treenode){
    if (treenode == NULL){
        return;
    }
    destroy_treenode(treenode->left);
    destroy_treenode(treenode->right);
    free(treenode);
}

/* 
Parses the tokens in the query list and constructs an expression tree using query_func, 
based on its syntax
*/
treenode_t* parse_query(list_t* query_list, char** errmsg){
    listnode_t* last = NULL;
    treenode_t* result = query_func(query_list->head, &last, errmsg);
    if (result == NULL){
        return NULL;
    }
    if (last != NULL){
        *errmsg = (char*)malloc(sizeof(char)*ERRMSG_SIZE);
        strcpy(*errmsg, "Unexpected token in the end of query");
        destroy_treenode(result);
        return NULL;
    }
    return result;
}

/* Query_func processes a query language and constructs an Abstract Syntax Tree for the input query by parsing tokens. 
It parsing a query that combines sub-queries with an "ANDNOT" operator. 
It constructs an AST, where each node represents a query term or an operator, and returns the root of the constructed tree.
*/
treenode_t* query_func(listnode_t* first, listnode_t** last, char** errmsg){
    treenode_t* and_result = andterm(first, last, errmsg);
    if (and_result == NULL){
        return NULL;
    }
    if (*last == NULL ||
        strcmp((char*)((*last)->elem), "ANDNOT")){
        return and_result;
    }

    treenode_t* right = query_func((*last)->next, last, errmsg);
    if (right == NULL){
        destroy_treenode(and_result);
        return NULL;
    }
    char* token = (char*)malloc(sizeof(char)*TOKEN_SIZE);
    strcpy(token, "ANDNOT");
    treenode_t* result = newnode(token);
    result->left = and_result;
    result->right = right;
    return result;
}
/*
Andterm function is parsing a query that combines sub-queries with "AND" operator. 
It constructs an AST, where each node represents a query term or an operator, and returns the root of the constructed tree. 
This function appears to work in tandem with the orterm, which handles the "OR" operator.
*/

treenode_t* andterm(listnode_t* first, listnode_t** last, char** errmsg){
    treenode_t* or_result = orterm(first, last, errmsg);
    if (or_result == NULL){
        return NULL;
    }
    if (*last == NULL || 
        strcmp((char*)((*last)->elem), "AND")){
        return or_result;
    }

    treenode_t* right = andterm((*last)->next, last, errmsg);
    if (right == NULL){
        destroy_treenode(or_result);
        return NULL;
    }
    char* token = (char*)malloc(sizeof(char)*TOKEN_SIZE);
    strcpy(token, "AND");
    treenode_t* result = newnode(token);
    result->left = or_result;
    result->right = right;
    return result;
}

/*  Andterm function is parsing a query that combines sub-queries with an "OR" operator. 
It constructs an AST, where each node represents a query term or an operator, and returns the root of the constructed tree. 
This function appears to work in tandem with the term.
*/

treenode_t* orterm(listnode_t* first, listnode_t** last, char** errmsg){
    treenode_t* term_result = term(first, last, errmsg);
    if (term_result == NULL){
        return NULL;
    }
    if (*last == NULL ||
        strcmp((char*)((*last)->elem), "OR")){
        return term_result;
    }
    treenode_t* right = orterm((*last)->next, last, errmsg);
    if (right == NULL){
        destroy_treenode(term_result);
        return NULL;
    }
    char* token = (char*)malloc(sizeof(char)*TOKEN_SIZE);
    strcpy(token, "OR");
    treenode_t* result = newnode(token);
    result->left = term_result;
    result->right = right;
    return result;    
}
// This helper function is taken from indexer.c. Uses in term.
static int is_reserved_word(char *word)
{
    if(strcmp(word, "ANDNOT") == 0)
        return 1;
    else if(strcmp(word, "AND") == 0)
        return 1;
    else if(strcmp(word, "OR") == 0)
        return 1;
    else if(strcmp(word, "(") == 0)
        return 1;
    else if(strcmp(word, ")") == 0)
        return 1;
    else
        return 0;
}

/*
Term function processes a single query term from tokens, creates a leaf node in the AST with the term, and returns that node. 
This function works in tandem with query_func and construct the AST by combining the leaf nodes created by term.
*/

treenode_t* term(listnode_t* first, listnode_t** last, char** errmsg) {
    if (first == NULL) {
        *errmsg = (char*)malloc(sizeof(char) * ERRMSG_SIZE);
        strcpy(*errmsg, "Unexpected end of input (term)");
        return NULL;
    }
    if (strcmp("(", (char*)first->elem) == 0){
        treenode_t *result = query_func(first->next, last, errmsg);
        if (result == NULL){
            return NULL;
        }
        if (*last != NULL && strcmp((*last)->elem, ")") == 0){
            *last = (*last)->next;
            return result;
        }
        destroy_treenode(result);
        *errmsg = (char*)malloc(sizeof(char) * ERRMSG_SIZE);
        strcpy(*errmsg, "Missing ) bracket");
        return NULL;
    }
    if (is_reserved_word((char*)first->elem)){
        *errmsg = (char*)malloc(sizeof(char) * ERRMSG_SIZE);
        strcpy(*errmsg, "Unexpected reserved word");
        return NULL;
    }
    *last = first->next;
    treenode_t *result = newnode(first->elem);
    result->left = NULL;
    result->right = NULL;
    return result;
}

/*
This function is used to calculate a syntax tree from a given expression tree, and then evaluate the resulting syntax tree using an index of documents.
The function takes in two parameters: a pointer to the root of the expression tree node, and a pointer to an index object that contains an inverted index of documents. 
The function returns a set_t pointer that represents the set of documents that satisfy the query.
It recursively creates a syntax tree by traversing the expression tree in a post-order traversal, starting from the left subtree, then the right subtree, and finally the root node.
*/
set_t* calculate_syntax_tree (treenode_t* node, index_t* index){
    if (node == NULL){
        return NULL;
    }
    set_t* left_set = calculate_syntax_tree(node->left, index);
    set_t* right_set = calculate_syntax_tree(node->right, index);
    char *word = (char*)node->elem;
    if (left_set == NULL && right_set == NULL){
        if (!is_reserved_word(word)&& map_haskey(index->map, word)){
            return (set_t*)map_get(index->map, word);
        }
        return NULL;
    }
    if (left_set == NULL){
        if (strcmp(node->elem,"OR") == 0){
        // set of paths of "web-pages"
            return right_set;
        }
        return NULL;
    }
    if (right_set == NULL){
        if (strcmp(node->elem,"OR") == 0 ||
            strcmp(node->elem,"ANDNOT") == 0){
        // set of paths of "web-pages"
            return left_set;
        }
        return NULL;
    }
    if(strcmp(word, "ANDNOT") == 0){
        //returns a set of paths which is a difference of results from left and right part of user's request
        return set_difference(left_set, right_set);
    }
    else if(strcmp(word, "AND") == 0){
        return set_intersection(left_set, right_set);
    }
    return set_union(left_set, right_set);
}

/*
Comparation functions
*/
int compare_query_results(void* a, void* b){
    query_result_t* qr_a = (query_result_t*)a;
    query_result_t* qr_b = (query_result_t*)b;
    return qr_b->score - qr_a->score;
}

int compare_documents(void* a, void* b){
    document_t* qr_a = (document_t*)a;
    document_t* qr_b = (document_t*)b;
    return compare_strings(qr_a->path, qr_b->path);
}

int compare_query_results_with_words (void* a, void* b){
    query_result_with_words_t* qr_a = (query_result_with_words_t*)a;
    query_result_with_words_t* qr_b = (query_result_with_words_t*)b;
    double diff = qr_b->query_result->score - qr_a->query_result->score;
    if(diff < 0)
        return -1;
    else if(diff == 0)
        return 0;
    return 1;
}


/*
Term Frequency (TF): This measures the frequency of a term in a document. It is calculated as the number of times a term appears in a document 
divided by the total number of terms in the document. 
The idea behind this is that the more frequently a term appears in a document, the more important it is to that document.
*/
double tf (char* word, query_result_with_words_t* doc){
    list_t* word_list = doc->words;
    double numitems = word_list->size;
    listnode_t* node = word_list->head;
    int count_word = 0;
    while (node != NULL){
        if (strcmp(word, (char*)node->elem)== 0){
            count_word ++;
        }
        node = node->next;
    }
    return count_word/numitems;
}

/* Count the combination of term frequency tf in the document and inverse document frequency idf to get the weight of each term in each document
tf_idf = tf*idf
In other words:
1. The weight reaches its maximum value if the term occurs many times in a small number of documents.
2. The weight is reduced if the term occurs only a few times in any document or occurs in many documents.
3. Weight is of minimal importance if the term appears in almost all documents
*/

void calculate_scores (index_t* index, list_t* query_results, list_t* query){
    listnode_t* word = query->head; 
    double* tfs = (double*)malloc(sizeof(double) * query_results->size);
    if(tfs == NULL)
        return;
    while (word != NULL){
        if (!is_reserved_word((char*)word->elem)){
            int doc_num = 0;
            double doc_count = 0;
            listnode_t* q_result_node = query_results->head;
            while (q_result_node != NULL){
                double tf_word = tf ((char*)word->elem, (query_result_with_words_t*)q_result_node->elem);
                if (tf_word > 0){
                    doc_count ++;
                }
                tfs[doc_num] = tf_word;
                q_result_node = q_result_node->next;
                ++doc_num;
            }

    double idf_word = log(set_size(index->set) / (double)query_results->size);
    doc_num = 0;            
    q_result_node = query_results->head;
    while (q_result_node != NULL){
        ((query_result_with_words_t*)q_result_node->elem)->query_result->score += tfs[doc_num] * idf_word;
        q_result_node = q_result_node->next;
        ++doc_num;
        }
    }
    word = word->next;
}
    free(tfs);
}

/*
Helper function, uses in index_query. It creates a new list of query_result_t structures without the field "words", 
by iterating over a list of query_result_with_words_t structures and extracting only the query_result_t field.
It takes a pointer to the original list list as input and returns a pointer to a new list list_without_words that only contains the query_result_t structures. 
It does so by iterating over each element of the original list, casting it to a query_result_with_words_t pointer, accessing its query_result field, 
and adding it to the new list using list_addlast.
*/
list_t* build_list_without_words (list_t* list){
    list_t* list_without_words = list_create(compare_query_results);
    if (list_without_words == NULL){
        return NULL;
    }
    listnode_t* current = list->head;
    while (current != NULL){
        list_addlast(list_without_words, ((query_result_with_words_t*)current->elem)->query_result);
        current = current->next;
    }
    list_destroy_and_destroy_elems(list, free);
    return list_without_words;
}

/*
 * Performs the given query on the given index.  If the query
 * succeeds, the return value will be a list of paths (query_result_t). 
 * If there is an error (e.g. a syntax error in the query), an error 
 * message is assigned to the given errmsg pointer and the return value
 * will be NULL.
 */
list_t *index_query(index_t *index, list_t *query, char **errmsg){
    //builds a syntax tree based on LL query using parse_query
    treenode_t* root = parse_query(query,errmsg);
    //creates an abstract syntax tree representing the structure of the query
    set_t* paths = calculate_syntax_tree(root, index);
    destroy_treenode(root);
    if (paths == NULL){
        return list_create(compare_query_results);
    }
    set_iter_t* set_iter = set_createiter(paths);
    list_t* list = list_create(compare_query_results_with_words);
    while (set_hasnext(set_iter)){
        document_t* doc = (document_t*)set_next(set_iter);
        query_result_t* query_result = (query_result_t*)malloc(sizeof(query_result_t));
        if (query_result == NULL){
            *errmsg = (char*)malloc(sizeof(char) * ERRMSG_SIZE);
            strcpy(*errmsg, "Memory allocation error in index_query");
            list_destroy(list);
            return NULL;
        }
        query_result_with_words_t* query_result_with_words = (query_result_with_words_t*)malloc(sizeof(query_result_with_words_t));
        if (query_result_with_words == NULL){
            *errmsg = (char*)malloc(sizeof(char) * ERRMSG_SIZE);
            strcpy(*errmsg, "Memory allocation error in index_query - query_results_with_words");
            list_destroy(list);
            return NULL;
        }
        query_result->path = doc->path;
        query_result->score = 0;
        query_result_with_words->query_result = query_result;
        query_result_with_words->words = doc->words;
        list_addlast(list, query_result_with_words);
    }
    //calculates the set of documents that match the query based on the logic of the syntax tree
    calculate_scores(index, list, query);
    list_sort(list);
    return build_list_without_words(list);
}