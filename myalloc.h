#ifndef MYALLOC_H
#define MYALLOC_H

#include <stdio.h>
#include <stdlib.h>

#define MEM_CAP 100
#define MEM_CHUNK_CAP 100
#define INFO_DUMP chunk_list_dump(alloced_chunks);chunk_list_dump(free_chunks);
#define BAR "\033[37m|"
#define ALLOCED_COLOR "\033[31m"
#define FREE_COLOR "\033[32m"

typedef struct {
	void* start;
	size_t size;
} Chunk;

typedef struct {
	size_t count;
	Chunk chunks[MEM_CHUNK_CAP];
} Chunk_List;

int MYALLOC_DEBUG = 0;
char memory[MEM_CAP];
void* mem_start = (void*)memory;
char chunk_line[MEM_CAP];
Chunk_List free_chunks = {1, {(void*)memory, sizeof(memory)}};
Chunk_List alloced_chunks = {0};

void print_chunk_line(void* recent) {
	Chunk cur;
	size_t cur_index;
	int going = 1;
	int runup = 0;
	for (size_t i=0; i<free_chunks.count; i++) {
		cur = free_chunks.chunks[i];
		cur_index = (cur.start - mem_start);
		for (size_t off=0; off<cur.size; off++) {
			chunk_line[cur_index+off] = off == 0 ? cur.start == recent ? 'n' : 's' : 'f';
		}
	}
	for (size_t i=0; i<alloced_chunks.count; i++) {
		cur = alloced_chunks.chunks[i];
		cur_index = (cur.start - mem_start);
		for (size_t off=0; off<cur.size; off++) {
			chunk_line[cur_index+off] = off == 0 ? cur.start == recent ? 'o' : 'p' : 'a';
		}
	}
	char c = 0;
	for (size_t i=0; i<sizeof(chunk_line); i++) {
		c = chunk_line[i];
		if ('f' == c) {printf(FREE_COLOR);}
		else if ('a' == c) {printf(ALLOCED_COLOR);}
		else if ('s' == c) {printf(BAR FREE_COLOR); runup += 1;}
		else if ('p' == c) {printf(BAR ALLOCED_COLOR); runup += 1;}
		else if ('n' == c) {printf(ALLOCED_COLOR); going = 0; runup += 1;}
		else if ('o' == c) {printf(FREE_COLOR); going = 0; runup += 1;}
		else {printf("\033[37mError: chunk_line[%zu] is not in free or alloced chunks!!!\n", i);exit(42);}
		printf("%c", c);
		// printf("%c", c == 'n' ? 'a' : c == 'o' ? 'f' : c == 's' ? 'f' : c == 'p' ? 'a' : c);
		chunk_line[i] = 0;
		runup += going;
	}
	printf("\033[37m|\n");
	for (int i=0; i<runup; i++) {printf(" ");}
	printf("^\n");
}

void chunk_list_dump(Chunk_List list) {
	printf("Chunks(%zu):\n", list.count);
	for (size_t i=0; i<list.count; i++) {
		Chunk c = list.chunks[i];
		printf("start: %p, size: %zu\n", c.start, c.size);
	}
}

void chunk_list_insert(Chunk_List* list, void* ptr, size_t size) {
	if (list->count >= MEM_CHUNK_CAP) {printf("Too many chunks!!!\n"); exit(42);}
	Chunk new = {ptr, size};
	if (0 == list->count) {list->chunks[list->count++] = new; return;}
	list->chunks[list->count++] = new;
	for (size_t i=list->count-1; i>0; i--) {
		if (list->chunks[i].start < list->chunks[i-1].start) {
			Chunk tmp = list->chunks[i];
			list->chunks[i] = list->chunks[i-1];
			list->chunks[i-1] = tmp;
		}
		else {break;}
	}
}

void chunk_list_remove(Chunk_List* list, size_t index) {
	if (index >= list->count) {printf("Invalid removal index!!!\n"); exit(42);}
	for (size_t i=index; i<list->count; i++) {
		list->chunks[i] = list->chunks[i+1];
	}
	list->count--;
}

int chunk_list_find(Chunk_List list, void* ptr) {
	for (size_t i=0; i<list.count; i++) {
		if (list.chunks[i].start == ptr) {return i;}
	}
	return -1;
}

void* memalloc(size_t size) {
	if (0 == size) {return NULL;}
	for (size_t i=0; i<free_chunks.count; i++) {
		Chunk opt = free_chunks.chunks[i];
		if (opt.size >= size) {
			chunk_list_remove(&free_chunks, i);
			chunk_list_insert(&alloced_chunks, opt.start, size);
			if (opt.size != size) {
				chunk_list_insert(&free_chunks, opt.start + size, opt.size - size);
			}
			print_chunk_line(opt.start);
			return opt.start;
		}
	}
	return NULL;
}

void memfree(void* ptr) {
	if (NULL == ptr) {return;}
	int id = chunk_list_find(alloced_chunks, ptr);
	if (-1 != id) {
		Chunk opt = alloced_chunks.chunks[id];
		chunk_list_insert(&free_chunks, opt.start, opt.size);
		chunk_list_remove(&alloced_chunks, id);
		return;
	}
	printf("Invalid ptr for free: %p\n", ptr);
	exit(42);
}

#endif
