#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_FRAMES 10000
#define MIN_ENTITIES 100
#define MAX_ENTITIES 10000
#define NUM_RUNS 5

typedef struct {
    char *buffer;
    size_t buffer_size;
    size_t current_offset;
    char on_stack[524288];
} StackAllocator;

void init_allocator(StackAllocator *alloc) {
    alloc->buffer = alloc->on_stack;
    alloc->buffer_size = sizeof(alloc->on_stack);
    alloc->current_offset = 0;
}

void *stack_alloc(StackAllocator *alloc, size_t size) {
    size = (size + 7) & ~7;  // 8-byte alignment
    if (alloc->current_offset + size > alloc->buffer_size) {
        return NULL;
    }
    void *ptr = alloc->buffer + alloc->current_offset;
    alloc->current_offset += size;
    return ptr;
}

typedef struct {
    float x, y;
    char name[32];
} Entity;

double run_stack_benchmark(int num_frames, int num_entities) {
    clock_t start = clock();
    StackAllocator frame_allocator;
    init_allocator(&frame_allocator);

    for (int frame = 0; frame < num_frames; frame++) {
        Entity *player = stack_alloc(&frame_allocator, sizeof(Entity));
        if (!player) {
            fprintf(stderr, "Stack allocation failed for player\n");
            return -1;
        }

        player->x = 100.0f;
        player->y = 200.0f;
        strcpy(player->name, "Player1");

        Entity *enemies =
            stack_alloc(&frame_allocator, num_entities * sizeof(Entity));
        if (!enemies) {
            fprintf(stderr, "Stack allocation failed for enemies\n");
            return -1;
        }

        for (int i = 0; i < num_entities; ++i) {
            enemies[i].x = i * 50.0f;
            enemies[i].y = i * 30.0f;
            sprintf(enemies[i].name, "Enemy%d", i);
        }

        frame_allocator.current_offset = 0;  // Reset allocator for next frame
    }

    clock_t end = clock();
    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

double run_heap_benchmark(int num_frames, int num_entities) {
    clock_t start = clock();

    for (int frame = 0; frame < num_frames; frame++) {
        Entity *player = (Entity *)malloc(sizeof(Entity));
        if (!player) {
            fprintf(stderr, "Heap allocation failed for player\n");
            return -1;
        }

        player->x = 100.0f;
        player->y = 200.0f;
        strcpy(player->name, "Player1");

        Entity *enemies = (Entity *)malloc(num_entities * sizeof(Entity));
        if (!enemies) {
            fprintf(stderr, "Heap allocation failed for enemies\n");
            free(player);
            return -1;
        }

        for (int i = 0; i < num_entities; ++i) {
            enemies[i].x = i * 50.0f;
            enemies[i].y = i * 30.0f;
            sprintf(enemies[i].name, "Enemy%d", i);
        }

        free(enemies);
        free(player);
    }

    clock_t end = clock();
    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

int main() {
    printf("Running benchmarks...\n");
    printf("Sizeof Entity -> %zu\n", sizeof(Entity));

    double stack_times[NUM_RUNS];
    double heap_times[NUM_RUNS];

    srand(time(NULL));

    for (int i = 0; i < NUM_RUNS; i++) {
        int num_entities =
            MIN_ENTITIES + (rand() % (MAX_ENTITIES - MIN_ENTITIES));
        printf("Configuration: %d frames, %d entities per frame\n", NUM_FRAMES,
               num_entities);

        stack_times[i] = run_stack_benchmark(NUM_FRAMES, num_entities);
        heap_times[i] = run_heap_benchmark(NUM_FRAMES, num_entities);

        printf("Run %d:\n", i + 1);
        printf("  Stack Allocator: %.4f seconds\n", stack_times[i]);
        printf("  Heap Allocator:  %.4f seconds\n\n", heap_times[i]);
    }

    double stack_avg = 0, heap_avg = 0;
    for (int i = 0; i < NUM_RUNS; i++) {
        stack_avg += stack_times[i];
        heap_avg += heap_times[i];
    }
    stack_avg /= NUM_RUNS;
    heap_avg /= NUM_RUNS;

    printf("Average Results:\n");
    printf("  Stack Allocator: %.4f seconds\n", stack_avg);
    printf("  Heap Allocator:  %.4f seconds\n", heap_avg);
    return 0;
}
