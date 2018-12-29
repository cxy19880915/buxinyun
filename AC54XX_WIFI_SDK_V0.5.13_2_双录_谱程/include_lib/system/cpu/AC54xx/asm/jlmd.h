
#ifndef __JLMD_H__
#define __JLMD_H__


#include "typedef.h"


#define     JLMD_EN         (0)
#define     JLMD_INT_EN     (1)
#define     JLMD_START      (2)
#define     JLMD_AXI_IDLE   (3)
#define     JLMD_STATUS     (4)
#define     JLMD_ERR        (10)
#define     JLMD_INT_PND    (11)
#define     JLMD_INT_CLR    (12)

#define     JLMD_EN_MASK         BIT(0)
#define     JLMD_INT_EN_MASK     BIT(1)
#define     JLMD_START_MASK      BIT(2)
#define     JLMD_AXI_IDLE_MASK   BIT(3)
#define     JLMD_STATUS_MASK     BIT(4)
#define     JLMD_ERR_MASK        BIT(10)
#define     JLMD_INT_PND_MASK    BIT(11)
#define     JLMD_INT_CLR_MASK    BIT(12)



#endif // __JLMD_H__
