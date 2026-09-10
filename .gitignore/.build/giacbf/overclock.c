//---
// gint:cpg:overclock - Clock speed control
//
// Most of the data in this file has been reused from Sentaro21's Ftune and
// Ptune utilities, which have long been the standard for overclocking CASIO
// calculators.
// See: http://pm.matrix.jp/ftune2e.html
//
// SlyVTT also contributed early testing on both the fx-CG 10/20 and fx-CG 50.
//---

extern int calculator;
#define TODO_is_a_cg50 (calculator==1)

#include "overclock.h"
#include "mpu_cpg_bsc.h"
#include <stddef.h>

#define CPG SH7305_CPG
#define BSC SH7305_BSC

//---
// Low-level clock speed access
//---

#define SDMR3_CL2 ((volatile uint8_t *)0xFEC15040)
#define SDMR3_CL3 ((volatile uint8_t *)0xFEC15060)

void cpg_get_overclock_setting(struct cpg_overclock_setting *s)
{
    s->FLLFRQ = CPG.FLLFRQ.lword;
    s->FRQCR = CPG.FRQCR.lword;

    s->CS0BCR = BSC.CS0BCR.lword;
    s->CS0WCR = BSC.CS0WCR.lword;
    s->CS2BCR = BSC.CS2BCR.lword;
    s->CS2WCR = BSC.CS2WCR.lword;
    s->CS3BCR = BSC.CS3BCR.lword;
    s->CS3WCR = BSC.CS3WCR.lword;
    s->CS5aBCR = BSC.CS5ABCR.lword;
    s->CS5aWCR = BSC.CS5AWCR.lword;
}

void cpg_set_overclock_setting(struct cpg_overclock_setting const *s)
{
    BSC.CS0WCR.WR = 11; /* 18 cycles */

    CPG.FLLFRQ.lword = s->FLLFRQ;
    CPG.FRQCR.lword = s->FRQCR;
    CPG.FRQCR.KICK = 1;
    while(CPG.LSTATS != 0) {}

    BSC.CS0BCR.lword = s->CS0BCR;
    BSC.CS0WCR.lword = s->CS0WCR;
    BSC.CS2BCR.lword = s->CS2BCR;
    BSC.CS2WCR.lword = s->CS2WCR;
    BSC.CS3BCR.lword = s->CS3BCR;
    BSC.CS3WCR.lword = s->CS3WCR;

    if(BSC.CS3WCR.A3CL == 1)
        *SDMR3_CL2 = 0;
    else
        *SDMR3_CL3 = 0;

    BSC.CS5ABCR.lword = s->CS5aBCR;
    BSC.CS5AWCR.lword = s->CS5aWCR;
}

//---
// Predefined clock speeds
//---

#define PLL_32x  0b011111
#define PLL_26x  0b011001
#define PLL_16x  0b001111
#define DIV_2    0
#define DIV_4    1
#define DIV_8    2
#define DIV_16   3
#define DIV_32   4

static struct cpg_overclock_setting settings_cg50[5] = {
    /* CLOCK_SPEED_F1 */
    { .FLLFRQ  = 0x00004000 + 900,
      .FRQCR   = 0x0F011112,
      .CS0BCR  = 0x36DA0400,
      .CS2BCR  = 0x36DA3400,
      .CS3BCR  = 0x36DB4400,
      .CS5aBCR = 0x17DF0400,
      .CS0WCR  = 0x000003C0,
      .CS2WCR  = 0x000003C0,
      .CS3WCR  = 0x000024D1,
      .CS5aWCR = 0x000203C1 },
    /* CLOCK_SPEED_F2 */
    { .FLLFRQ  = 0x00004000 + 900,
      .FRQCR   = (PLL_16x<<24)+(DIV_4<<20)+(DIV_8<<12)+(DIV_8<<8)+DIV_8,
      .CS0BCR  = 0x24920400,
      .CS2BCR  = 0x24923400,
      .CS3BCR  = 0x24924400,
      .CS5aBCR = 0x17DF0400,
      .CS0WCR  = 0x00000340,
      .CS2WCR  = 0x000003C0,
      .CS3WCR  = 0x000024D1,
      .CS5aWCR = 0x000203C1 },
    /* CLOCK_SPEED_F3 */
    { .FLLFRQ  = 0x00004000 + 900,
      .FRQCR  = (PLL_26x<<24)+(DIV_4<<20)+(DIV_8<<12)+(DIV_8<<8)+DIV_8,
      .CS0BCR  = 0x24920400,
      .CS2BCR  = 0x24923400,
      .CS3BCR  = 0x24924400,
      .CS5aBCR = 0x17DF0400,
      .CS0WCR  = 0x00000240,
      .CS2WCR  = 0x000003C0,
      .CS3WCR  = 0x000024D1,
      .CS5aWCR = 0x000203C1 },
    /* CLOCK_SPEED_F4 */
    { .FLLFRQ  = 0x00004000 + 900,
      .FRQCR  = (PLL_32x<<24)+(DIV_2<<20)+(DIV_4<<12)+(DIV_8<<8)+DIV_16,
      .CS0BCR  = 0x24920400,
      .CS2BCR  = 0x24923400,
      .CS3BCR  = 0x24924400,
      .CS5aBCR = 0x17DF0400,
      .CS0WCR  = 0x000002C0,
      .CS2WCR  = 0x000003C0,
      .CS3WCR  = 0x000024D1,
      .CS5aWCR = 0x000203C1 },
    /* CLOCK_SPEED_F5 */
    { .FLLFRQ  = 0x00004000 + 900,
      .FRQCR  = (PLL_26x<<24)+(DIV_2<<20)+(DIV_4<<12)+(DIV_4<<8)+DIV_8,
      .CS0BCR  = 0x24920400,
      .CS2BCR  = 0x24923400,
      .CS3BCR  = 0x24924400,
      .CS5aBCR = 0x17DF0400,
      .CS0WCR  = 0x00000440,
      .CS2WCR  = 0x000003C0,
      .CS3WCR  = 0x000024D1,
      .CS5aWCR = 0x000203C1 },
};

static struct cpg_overclock_setting settings_cg20[5] = {
    /* CLOCK_SPEED_F1 */
    { .FLLFRQ  = 0x00004000 + 900,
      .FRQCR   = 0x0F102203,
      .CS0BCR  = 0x24920400,
      .CS2BCR  = 0x24923400,
      .CS3BCR  = 0x24924400,
      .CS5aBCR = 0x15140400,
      .CS0WCR  = 0x000001C0,
      .CS2WCR  = 0x00000140,
      .CS3WCR  = 0x000024D0,
      .CS5aWCR = 0x00010240 },
    /* CLOCK_SPEED_F2 */
    { .FLLFRQ  = 0x00004000 + 900,
      .FRQCR   = (PLL_32x<<24)+(DIV_8<<20)+(DIV_16<<12)+(DIV_16<<8)+DIV_32,
      .CS0BCR  = 0x04900400,
      .CS2BCR  = 0x04903400,
      .CS3BCR  = 0x24924400,
      .CS5aBCR = 0x15140400,
      .CS0WCR  = 0x00000140,
      .CS2WCR  = 0x000100C0,
      .CS3WCR  = 0x000024D0,
      .CS5aWCR = 0x00010240 },
    /* CLOCK_SPEED_F3 */
    { .FLLFRQ  = 0x00004000 + 900,
      .FRQCR   = (PLL_32x<<24)+(DIV_4<<20)+(DIV_8<<12)+(DIV_8<<8)+DIV_32,
      .CS0BCR  = 0x24900400,
      .CS2BCR  = 0x04903400,
      .CS3BCR  = 0x24924400,
      .CS5aBCR = 0x15140400,
      .CS0WCR  = 0x000002C0,
      .CS2WCR  = 0x000201C0,
      .CS3WCR  = 0x000024D0,
      .CS5aWCR = 0x00010240 },
    /* CLOCK_SPEED_F4 */
    { .FLLFRQ  = 0x00004000 + 900,
      .FRQCR   = (PLL_32x<<24)+(DIV_4<<20)+(DIV_4<<12)+(DIV_4<<8)+DIV_32,
      .CS0BCR  = 0x44900400,
      .CS2BCR  = 0x04903400,
      .CS3BCR  = 0x24924400,
      .CS5aBCR = 0x15140400,
      .CS0WCR  = 0x00000440,
      .CS2WCR  = 0x00040340,
      .CS3WCR  = 0x000024D0,
      .CS5aWCR = 0x00010240 },
    /* CLOCK_SPEED_F5 */
    { .FLLFRQ  = 0x00004000 + 900,
      .FRQCR   = (PLL_26x<<24)+(DIV_2<<20)+(DIV_4<<12)+(DIV_4<<8)+DIV_16,
      .CS0BCR  = 0x34900400,
      .CS2BCR  = 0x04903400,
      .CS3BCR  = 0x24924400,
      .CS5aBCR = 0x15140400,
      .CS0WCR  = 0x000003C0,
      .CS2WCR  = 0x000402C0,
      .CS3WCR  = 0x000024D0,
      .CS5aWCR = 0x00010240 },
};

static struct cpg_overclock_setting *get_settings(void)
{
    if(TODO_is_a_cg50)
        return settings_cg50;
    // if(TODO_is_a_cg10_or_cg20)
        return settings_cg20;
    return NULL;
}

int clock_get_speed(void)
{
    struct cpg_overclock_setting *settings = get_settings();
    if(!settings)
        return CLOCK_SPEED_UNKNOWN;

    for(int i = 0; i < 5; i++) {
        struct cpg_overclock_setting *s = &settings[i];

        if(CPG.FLLFRQ.lword == s->FLLFRQ
            && CPG.FRQCR.lword == s->FRQCR
            && BSC.CS0BCR.lword == s->CS0BCR
            && BSC.CS2BCR.lword == s->CS2BCR
            && BSC.CS3BCR.lword == s->CS3BCR
            && BSC.CS5ABCR.lword == s->CS5aBCR
            && BSC.CS0WCR.lword == s->CS0WCR
            && BSC.CS2WCR.lword == s->CS2WCR
            && BSC.CS3WCR.lword == s->CS3WCR
            && BSC.CS5AWCR.lword == s->CS5aWCR)
            return CLOCK_SPEED_F1 + i;
    }

    return CLOCK_SPEED_UNKNOWN;
}

void clock_set_speed(int level)
{
    if(level < CLOCK_SPEED_F1 || level > CLOCK_SPEED_F5)
        return;
    if(clock_get_speed() == level)
        return;

    struct cpg_overclock_setting *settings = get_settings();
    if(!settings)
        return;

    struct cpg_overclock_setting *s = &settings[level - CLOCK_SPEED_F1];

    /* Block interrupts */
    uint32_t SR, SR2;
    __asm__("stc sr, %0": "=r"(SR));
    SR2 = SR | (1 << 28);
    __asm__("ldc %0, sr":: "r"(SR2));

    /* Disable interrupts during the change */    /* Set the clock settings */
    cpg_set_overclock_setting(s);

    /* Restore interrupts */
    __asm__("ldc %0, sr":: "r"(SR));
}
