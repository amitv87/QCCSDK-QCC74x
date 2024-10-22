#include <stdint.h>
#include <stdbool.h>
#include "export/rwnx_config.h"
#include "ip/lmac/src/mm/mm.h"
#include "ip/lmac/src/mm/mm_task.h"
#include "ip/umac/src/rxu/rxu_task.h"
#include "ip/umac/src/scanu/scanu_task.h"
#include "ip/lmac/src/scan/scan_task.h"
#include "ip/umac/src/sm/sm_task.h"
#include "ip/umac/src/me/me_task.h"
#include "ip/umac/src/apm/apm_task.h"
#if NX_TWT
#include "ip/umac/src/twt/twt_task.h"
#endif
#if RW_MESH_EN
#include "modules/mesh/src/mesh_task.h"
#endif
#include "ip/umac/src/ftm/ftm_task.h"
#include "modules/dbg/src/dbg_task.h"

#define struct_func_alloc_def(name)                                             \
    void *name##_alloc()                                                        \
    {                                                                           \
        struct name *p = (struct name *)malloc(sizeof(struct name));            \
        return p;                                                               \
    }                                                                           \

#include "export/export_malloc_list.h"
#undef struct_func_alloc_def
