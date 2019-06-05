#ifndef _RANLUX_H
#define _RANLUX_H

#define RANDOM_BUFFER_SIZE 10000

extern float hs_ranlux_common_buffer[RANDOM_BUFFER_SIZE];
extern int hs_ranlux_common_buffer_index;

#define next_uniform_random(varname) do {\
    if (hs_ranlux_common_buffer_index >= RANDOM_BUFFER_SIZE)\
    {\
        hs_ranlux_common_buffer_index = RANDOM_BUFFER_SIZE;\
        ranlux_(hs_ranlux_common_buffer, &hs_ranlux_common_buffer_index);\
        hs_ranlux_common_buffer_index = 0;\
    }\
    varname = hs_ranlux_common_buffer[hs_ranlux_common_buffer_index++];\
} while(varname <= 0.f || varname >= 1.f);

#endif /* _RANLUX_H */
