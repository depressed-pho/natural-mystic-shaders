#ifndef _UNIFORM_INTER_FRAME_CONSTANTS_H
#define _UNIFORM_INTER_FRAME_CONSTANTS_H

#include "natural-mystic-precision.h"
#include "uniformMacro.h"

#ifdef MCPE_PLATFORM_NX
uniform InterFrameConstants {
#endif
// BEGIN_UNIFORM_BLOCK(InterFrameConstants) - unfortunately this macro does not work on old Amazon platforms so using above 3 lines instead
// in secs. This is reset every 1 hour. so the range is [0, 3600]
// make sure your shader handles the case when it transitions from 3600 to 0
UNIFORM prec_hm float TOTAL_REAL_WORLD_TIME;
UNIFORM MAT4 CUBE_MAP_ROTATION;
END_UNIFORM_BLOCK

#endif
