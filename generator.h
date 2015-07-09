#ifndef CIAPOS_GENERATOR_H
#define CIAPOS_GENERATOR_H

#define begin_generator(state) \
    int *ciapos_generator_state_variable = &(state); \
    switch (state) { \
    case 0:
#define yield(x) \
        do { \
            *ciapos_generator_state_variable = __LINE__; \
            return (x); \
            case __LINE__:; \
        } while (0);
#define end_generator \
    }

#endif
