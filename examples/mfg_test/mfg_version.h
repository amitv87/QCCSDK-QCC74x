#ifndef _MFG_VERSION_H_
#define _MFG_VERSION_H_

#define QCC74x_MFG_VER "2.50"



typedef struct
{
    uint8_t anti_rollback;
    uint8_t x;
    uint8_t y;
    uint8_t z;
    char *name;
    char *build_time;
    char * commit_id;
    uint32_t rsvd0;
    uint32_t rsvd1;
}qcc74xverinf_t;

#endif /*_MFG_VERSION_H_*/
