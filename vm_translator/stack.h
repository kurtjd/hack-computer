#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define STACK_SIZE (sizeof *(stack->data))

typedef struct Stack
{
    int16_t *data;
    int sp;
} Stack;

// Free the allocated memory for the stack
void stack_free(Stack *stack);

// Allocate initial memory for stack
bool stack_init(Stack *stack);

/* Add a value to top of stack
 *
 * Return false if unable to reallocate memory
 */
bool stack_push(Stack *stack, int16_t value);

/* Retrieve a value from top of stack
 *
 * Since every possible return value is valid, the success of the function
 * cannot be returned. Pass a reference to a bool which will store the success,
 * or pass NULL if wish to discard.
 */
int16_t stack_pop(Stack *stack, bool *success);
