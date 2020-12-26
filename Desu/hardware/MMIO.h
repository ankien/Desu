/// LCD ///
#define DISPCNT_MODE (systemMemory->dispcnt & 0x7)
#define DISPCNT_CGB (systemMemory->dispcnt & 0x8)
#define DISPCNT_DISPLAY_FRAME_SELECT (systemMemory->dispcnt & 0x10)
#define DISPCNT_HBLANK_INTERVAL_FREE (systemMemory->dispcnt & 0x20)
#define DISPCNT_OBJ_VRAM_MAPPING (systemMemory->dispcnt & 0x40)
#define DISPCNT_FORCED_BLANK (systemMemory->dispcnt & 0x80)
#define DISPCNT_BG0 (systemMemory->dispcnt & 0x100)
#define DISPCNT_BG1 (systemMemory->dispcnt & 0x200)
#define DISPCNT_BG2 (systemMemory->dispcnt & 0x400)
#define DISPCNT_BG3 (systemMemory->dispcnt & 0x800)
#define DISPCNT_OBJ (systemMemory->dispcnt & 0x1000)
#define DISPCNT_WIN0 (systemMemory->dispcnt & 0x2000)
#define DISPCNT_WIN1 (systemMemory->dispcnt & 0x4000)

#define DISPSTAT_VBLANK_FLAG (systemMemory->dispstat & 0x1)
#define DISPSTAT_HBLANK_FLAG (systemMemory->dispstat & 0x2)
#define DISPSTAT_VCOUNTER_FLAG (systemMemory->dispstat & 0x4)
#define DISPSTAT_VBLANK_IRQ (systemMemory->dispstat & 0x8)
#define DISPSTAT_HBLANK_IRQ (systemMemory->dispstat & 0x10)
#define DISPSTAT_VCOUNT_IRQ (systemMemory->dispstat & 0x20)
#define DISPSTAT_VCOUNT_SETTING ((systemMemory->dispstat & 0xFF00) >> 8)

#define VCOUNT (systemMemory->vcount & 0xFF)

#define BG0CNT_BG_PRIORITY (systemMemory->bgcnt0 & 0x3)
#define BG0CNT_CHARACTER_BASE_BLOCK ((systemMemory->bgcnt0 & 0xC) >> 2)
#define BG0CNT_MOSAIC (systemMemory->bgcnt0 & 0x40)
#define BG0CNT_COLOR_PALETTES (systemMemory->bgcnt0 & 0x80)
#define BG0CNT_SCREEN_BASE_BLOCK ((systemMemory->bgcnt0 & 0x1F00) >> 8)
#define BG0CNT_DISPLAY_AREA_OVERFLOW (systemMemory->bgcnt0 & 0x2000)
#define BG0CNT_SCREEN_SIZE ((systemMemory->bgcnt0 & 0xC000) >> 14)

#define BG1CNT_BG_PRIORITY (systemMemory->bgcnt1 & 0x3)
#define BG1CNT_CHARACTER_BASE_BLOCK ((systemMemory->bgcnt1 & 0xC) >> 2)
#define BG1CNT_MOSAIC (systemMemory->bgcnt1 & 0x40)
#define BG1CNT_COLOR_PALETTES (systemMemory->bgcnt1 & 0x80)
#define BG1CNT_SCREEN_BASE_BLOCK ((systemMemory->bgcnt1 & 0x1F00) >> 8)
#define BG1CNT_DISPLAY_AREA_OVERFLOW (systemMemory->bgcnt1 & 0x2000)
#define BG1CNT_SCREEN_SIZE ((systemMemory->bgcnt1 & 0xC000) >> 14)

#define BG2CNT_BG_PRIORITY (systemMemory->bgcnt2 & 0x3)
#define BG2CNT_CHARACTER_BASE_BLOCK ((systemMemory->bgcnt2 & 0xC) >> 2)
#define BG2CNT_MOSAIC (systemMemory->bgcnt2 & 0x40)
#define BG2CNT_COLOR_PALETTES (systemMemory->bgcnt2 & 0x80)
#define BG2CNT_SCREEN_BASE_BLOCK ((systemMemory->bgcnt2 & 0x1F00) >> 8)
#define BG2CNT_DISPLAY_AREA_OVERFLOW (systemMemory->bgcnt2 & 0x2000)
#define BG2CNT_SCREEN_SIZE ((systemMemory->bgcnt2 & 0xC000) >> 14)

#define BG3CNT_BG_PRIORITY (systemMemory->bgcnt3 & 0x3)
#define BG3CNT_CHARACTER_BASE_BLOCK ((systemMemory->bgcnt3 & 0xC) >> 2)
#define BG3CNT_MOSAIC (systemMemory->bgcnt3 & 0x40)
#define BG3CNT_COLOR_PALETTES (systemMemory->bgcnt3 & 0x80)
#define BG3CNT_SCREEN_BASE_BLOCK ((systemMemory->bgcnt3 & 0x1F00) >> 8)
#define BG3CNT_DISPLAY_AREA_OVERFLOW (systemMemory->bgcnt3 & 0x2000)
#define BG3CNT_SCREEN_SIZE ((systemMemory->bgcnt3 & 0xC000) >> 14)

#define BG0HOFS (systemMemory->bghofs0 & 0x1FF)
#define BG0VOFS (systemMemory->bgvofs0 & 0x1FF)
#define BG1HOFS (systemMemory->bghofs1 & 0x1FF)
#define BG1VOFS (systemMemory->bgvofs1 & 0x1FF)
#define BG2HOFS (systemMemory->bghofs2 & 0x1FF)
#define BG2VOFS (systemMemory->bgvofs2 & 0x1FF)
#define BG3HOFS (systemMemory->bghofs3 & 0x1FF)
#define BG3VOFS (systemMemory->bgvofs3 & 0x1FF)

#define WIN0H_X2 (systemMemory->winh0 & 0xFF)
#define WIN0H_X1 ((systemMemory->winh0 & 0xFF00) >> 8)
#define WIN1H_X2 (systemMemory->winh1 & 0xFF)
#define WIN1H_X1 ((systemMemory->winh1 & 0xFF00) >> 8)
#define WIN0V_Y2 (systemMemory->winv0 & 0xFF)
#define WIN0V_Y1 ((systemMemory->winv0 & 0xFF00) >> 8)
#define WIN1V_Y2 (systemMemory->winv1 & 0xFF)
#define WIN1V_Y1 ((systemMemory->winv1 & 0xFF00) >> 8)

#define WININ_WIN0_BG_ENABLE_BITS (systemMemory->winin & 0xF)
#define WININ_WIN0_OBJ_ENABLE_BIT (systemMemory->winin & 0x10)
#define WININ_WIN0_COLOR_SPECIAL_EFFECT (systemMemory->winin & 0x20)
#define WININ_WIN1_BG_ENABLE_BITS ((systemMemory->winin & 0xF00) >> 8)
#define WININ_WIN1_OBJ_ENABLE_BIT (systemMemory->winin & 0x1000)
#define WININ_WIN1_COLOR_SPECIAL_EFFECT (systemMemory->winin & 0x2000)

#define MOSAIC_BG_HSIZE (systemMemory->mosaic & 0xF)
#define MOSAIC_BG_VSIZE ((systemMemory->mosaic & 0xF0) >> 4)
#define MOSAIC_OBJ_HSIZE ((systemMemory->mosaic & 0xF00) >> 8)
#define MOSAIC_OBJ_HSIZE ((systemMemory->mosaic & 0xF000) >> 12)

#define BLDCNT_BG0_TP1 (systemMemory->bldcnt & 0x1)
#define BLDCNT_BG1_TP1 (systemMemory->bldcnt & 0x2)
#define BLDCNT_BG2_TP1 (systemMemory->bldcnt & 0x4)
#define BLDCNT_BG3_TP1 (systemMemory->bldcnt & 0x8)
#define BLDCNT_OBJ_TP1 (systemMemory->bldcnt & 0x10)
#define BLDCNT_BD_TP1 (systemMemory->bldcnt & 0x20)
#define BLDCNT_COLOR_SP ((systemMemory->bldcnt & 0xC0) >> 6)
#define BLDCNT_BG0_TP2 (systemMemory->bldcnt & 0x100)
#define BLDCNT_BG1_TP2 (systemMemory->bldcnt & 0x200)
#define BLDCNT_BG2_TP2 (systemMemory->bldcnt & 0x400)
#define BLDCNT_BG3_TP2 (systemMemory->bldcnt & 0x800)
#define BLDCNT_OBJ_TP2 (systemMemory->bldcnt & 0x1000)
#define BLDCNT_BD_TP2 (systemMemory->bldcnt & 0x2000)

#define BLDALPHA_EVA (systemMemory->bldalpha & 0x1F)
#define BLDALPHA_EVB ((systemMemory->bldalpha & 0x1F00) >> 8)

#define BLDY_EVY (systemMemory->bldy & 0x1F)