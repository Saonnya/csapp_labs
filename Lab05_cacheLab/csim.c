#define _GNU_SOURCE
#include "cachelab.h"
#include "stdlib.h"
#include <stdio.h>
#include "getopt.h"
#include <string.h>
#define addrLen 8

// ====== @struct definition ======
typedef struct _Node {
    int isValid;
    unsigned tag;
    struct _Node* next;
    struct _Node* prev;
}Node;  // one Node stands for one cache line

typedef struct _CacheLines {
    Node* head;
    Node* tail;
    int size;
}CacheLines;  // one set consists of few cache lines

// ====== @static vars ======
static int V;  // -v, verbose flag, default 0
static int S;  // s, number of set index bits
static int E;  // E, number of lines per set
static int B;  // B, number of block bits
static CacheLines* pCache;  // pointer to all sets
static int hits = 0;     // count hits
static int misses = 0;   // count misses
static int evictions = 0;// count evictions

// ====== @function definition ======
void print_help() {
    printf("[HELP INFO] Call this cache simulator as following: \n");
    printf("./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
    printf("\t-h: Optional help flag that prints usage info\n");
    printf("\t-v: Optional verbose flag that displays trace info\n");
    printf("\t-s <s>: Number of set index bits (S = 2^s is the number of sets)\n");
    printf("\t-E <E>: Associativity (number of lines per set)\n");
    printf("\t-b <b>: Number of block bits (B = 2^b is the block size)\n");
}


void put_node_to_front(Node* pNode, CacheLines* pSet) {
    /* add Node to front of Set */
    pNode->next = pSet->head->next;
    pSet->head->next->prev = pNode;
    pSet->head->next = pNode;
    pNode->prev = pSet->head;
}


void update_cache(unsigned addr) {
    /* load or store updates here */
    // step1: get its number of target set/tag
    int tarSet = (addr >> B) & ((unsigned)(-1) >> (32 - S));
    unsigned tarTag = addr >> (S + B);

    CacheLines* pSet = &pCache[tarSet];
    
    // step2: search target tag in this cache set LRU
    Node* curNode = pSet->head->next;
    while(curNode != pSet->tail) {
        if(curNode->tag == tarTag) {
            break;
        }
        curNode = curNode->next;
    }

    if(curNode->tag == tarTag && curNode->isValid == 1) {
        /*/  H I T  /*/
        hits++;
        if(V == 1)  printf(" hit");
        // move curNode to front
        if(curNode != pSet->head->next) {
            curNode->prev->next = curNode->next;
            curNode->next->prev = curNode->prev;
            put_node_to_front(curNode, pSet);
        }
    } else {
        /*/ M I S S /*/
        misses++;
        if(V == 1)  printf(" miss");
        // creat new Node and add to front
        Node* newNode = malloc(sizeof(Node));
        newNode->tag = tarTag;
        newNode->isValid = 1;
        put_node_to_front(newNode, pSet);
        // evict LRU Node if the set is full
        if(pSet->size == E) {
            /*/ EVICTION /*/
            evictions++;
            if(V == 1)  printf(" eviction");
            pSet->tail->prev->prev->next = pSet->tail;
            pSet->tail->prev = pSet->tail->prev->prev;
        } else {
            pSet->size++;
        }
    }
}


void parseOption(int argc, char** argv, char* filename) {
    /* 解析输入参数, 并修改相应的变量, 在main中调用 */
    int option;
    while((option = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch(option) {
            case 'h':
                print_help();
                exit(0);
            case 'v':
                V = 1;
                break;
            case 's':
                S = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                B = atoi(optarg);
                break;
            case 't':
                strcpy(filename, optarg);
                break;
        }
    }
}


void cacheSimulator(char* filename) {
    /* 初始化并模拟调用cache的不同功能 */
    // step1: new cache line space for eeach set
    int numSet = 1 << S;
    pCache = malloc(numSet * sizeof(CacheLines));  // not freed
    for(int i = 0; i < numSet; i++) {
        pCache[i].head = malloc(sizeof(Node));
        pCache[i].tail = malloc(sizeof(Node));
        pCache[i].head->next = pCache[i].tail;
        pCache[i].tail->prev = pCache[i].head;
        pCache[i].size = 0;
    }

    // step2: read operation and do
    FILE *fp = fopen(filename, "r");
    if(fp == NULL) {
        printf("[ERR] Wrong path of file!");
        exit(-1);
    }
    char opt;
    unsigned addr;
    int size;
    // memory access won't crosses block, so size can be ignored
    while(fscanf(fp, " %c %x,%d", &opt, &addr, &size) > 0) {
        switch (opt) {
        case 'L':
            if(V == 1)  printf("%c %x,%d", opt, addr, size);
            update_cache(addr);
            if(V == 1)  printf("\n");
            break;
        case 'M':
            if(V == 1)  printf("%c %x,%d", opt, addr, size);
            update_cache(addr);
            if(V == 1)  printf("\n");
            // no break, will fallthrough and update twice
        case 'S':
            if(V == 1)  printf("%c %x,%d", opt, addr, size);
            update_cache(addr);
            if(V == 1)  printf("\n");
            break;
        default:
            break;  // ignore I -- instruction load
        }
    }
    fclose(fp);
}

// ====== main ======
int main(int argc, char** argv) {
    // step1: parse option
    char* filename = malloc(64 * sizeof(char));
    parseOption(argc, argv, filename);

    // step2: read all requirements and analysize
    cacheSimulator(filename);

    // template print
    printSummary(hits, misses, evictions);
    free(filename);
    filename = NULL;
    return 0;
}
