#include <stdio.h>
#include <string.h>

typedef struct {
    char *buffer;
    size_t buffer_size;
    size_t current_offset;
    char on_stack[8192];
} StackAllocator;

void init_allocator(StackAllocator *alloc) {
    alloc->buffer = alloc->on_stack;
    alloc->buffer_size = sizeof(alloc->on_stack);
    alloc->current_offset = 0;
}

void *stack_alloc(StackAllocator *alloc, size_t size) {
    size = (size + 7) & ~7;

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

int main() {
    StackAllocator frame_allocator;
    init_allocator(&frame_allocator);

    for (int frame = 0; frame < 3; frame++) {
        printf("\nFrame %d:\n", frame);

        Entity *player = stack_alloc(&frame_allocator, sizeof(Entity));
        if (!player) {
            fprintf(stderr, "DO NOT TRY TO OVERFLOW THE BUFFER KID\n");
            return 1;
        }
        player->x = 100.0f;
        player->y = 200.0f;
        strcpy(player->name, "Player1");
        printf("Created player at %.1f, %.1f\n", player->x, player->y);

        Entity *enemies = stack_alloc(&frame_allocator, 69 * sizeof(Entity));
        if (!enemies) {
            fprintf(stderr, "DO NOT TRY TO OVERFLOW THE BUFFER KID\n");
            return 1;
        }
        for (int i = 0; i < 69; ++i) {
            enemies[i].x = i * 50.0f;
            enemies[i].y = i * 30.0f;
            sprintf(enemies[i].name, "Enemy%d", i);
            printf("Created %d enemy at %.1f, %.1f\n", i + 1, enemies[i].x,
                   enemies[i].y);
        }

        frame_allocator.current_offset = 0;
        printf("Memory reset for next frame\n");
    }

    return 0;
}
