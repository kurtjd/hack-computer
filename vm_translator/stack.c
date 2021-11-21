#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "stack.h"

void stack_free(Stack *stack)
{
    free(stack->data);
    stack->data = NULL;
}

bool stack_init(Stack *stack)
{
    stack->data = malloc(STACK_SIZE);
    if (stack->data == NULL)
    {
        fprintf(stderr, "Unable to allocate stack memory.\n");
        return false;
    }

    stack->sp = 0;
    return true;
}

bool stack_push(Stack *stack, int16_t value)
{
    stack->data = realloc(stack->data, STACK_SIZE + 1);
    if (stack->data == NULL)
    {
        fprintf(stderr, "Unable to reallocate stack memory for push.\n");
        return false;
    }

    *(stack->data + stack->sp++) = value;
    return true;
}

int16_t stack_pop(Stack *stack, bool *success)
{
    stack->data = realloc(stack->data, STACK_SIZE - 1);
    if (stack->data == NULL)
    {
        fprintf(stderr, "Unable to reallocate stack memory for pop.\n");
        if (success != NULL)
        {
            *success = false;
        }

        return 0;
    }

    if (success != NULL)
    {
        *success = true;
    }

    return *(stack->data + --stack->sp);
}
