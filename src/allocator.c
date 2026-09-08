#define HEAP_SIZE 1024

static unsigned char heap[HEAP_SIZE] = {0};

typedef struct Block {
    unsigned int size;
    int free;
    struct Block *next;
} Block;

static Block *head = 0;

void mem_init(void) {
    head = (Block *)heap;
    head->size = HEAP_SIZE - sizeof(Block);
    head->free = 1;
    head->next = 0;
}

void *mem_alloc(unsigned int size) {
    Block *cur = head;
    while (cur) {
        if (cur->free && cur->size >= size) {
            if (cur->size > size + sizeof(Block)) {
                Block *newb = (Block *)((unsigned char *)cur + sizeof(Block) + size);
                newb->size = cur->size - size - sizeof(Block);
                newb->free = 1;
                newb->next = cur->next;
                cur->next = newb;
                cur->size = size;
            }
            cur->free = 0;
            return (void *)((unsigned char *)cur + sizeof(Block));
        }
        cur = cur->next;
    }
    return 0;
}

void mem_free(void *ptr) {
    Block *cur = (Block *)((unsigned char *)ptr - sizeof(Block));
    cur->free = 1;

    cur = head;
    while (cur && cur->next) {
        if (cur->free && cur->next->free) {
            cur->size += sizeof(Block) + cur->next->size;
            cur->next = cur->next->next;
        } else {
            cur = cur->next;
        }
    }
}
