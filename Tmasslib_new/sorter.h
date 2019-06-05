#ifndef SORTER_H_
#define SORTER_H_

#define sort_struct_by_member_incr(structname, member) static int \
sort_ ## structname ## _by_ ## member ## _incr(const void *ip1, const void *ip2)\
{\
    const structname *p1 = (const structname *)ip1;\
    const structname *p2 = (const structname *)ip2;\
    if (p1->member < p2->member)\
        return -1;\
    else if (p1->member > p2->member)\
        return 1;\
    else\
        return 0;\
}\

#define sort_struct_by_member_decr(structname, member) static int \
sort_ ## structname ## _by_ ## member ## _decr(const void *ip1, const void *ip2)\
{\
    const structname *p1 = (const structname *)ip1;\
    const structname *p2 = (const structname *)ip2;\
    if (p1->member < p2->member)\
        return 1;\
    else if (p1->member > p2->member)\
        return -1;\
    else\
        return 0;\
}\

#endif /* SORTER_H_ */
