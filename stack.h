

#if !defined(__STACK__)
#define __STACK__


#define stack_array_top_p(name) name ## __p__
#define stack_array_def(base_type, mag, name)  base_type name[mag]; base_type *stack_array_top_p(name)

#define stack_array_move(name, value)  (stack_array_top_p(name) = (value))
#define stack_array_init(name)         stack_array_move(name, (name))
#define stack_array_push(name, val)    (*stack_array_top_p(name)++ = (val))
#define stack_array_pop(name)          (*--stack_array_top_p(name))
#define stack_array_peek(name)         (*(stack_array_top_p(name) - 1))
#define stack_array_base(name)         (name)
#define stack_array_top(name)          stack_array_top_p
#define stack_array_is_empty(name)     ((name) >= stack_array_top_p(name))
#define stack_array_is_not_empty(name) ((name) < stack_array_top_p(name))

#endif