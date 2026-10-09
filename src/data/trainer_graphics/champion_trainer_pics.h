// The champions of other regions (and Alain), for the Mega Evolution tournament. Drawn by
// tools/mega_island/champions (export.py writes the pics and palettes).

static const u32 gTrainerFrontPic_ChampionCynthia[] = INCGFX_U32("graphics/trainers/front_pics/champion_cynthia.png", ".4bpp.lz");
static const u32 gTrainerPalette_ChampionCynthia[] = INCGFX_U32("graphics/trainers/palettes/champion_cynthia.pal", ".gbapal.lz");
static const u32 gTrainerFrontPic_ChampionAlder[] = INCGFX_U32("graphics/trainers/front_pics/champion_alder.png", ".4bpp.lz");
static const u32 gTrainerPalette_ChampionAlder[] = INCGFX_U32("graphics/trainers/palettes/champion_alder.pal", ".gbapal.lz");
static const u32 gTrainerFrontPic_ChampionDiantha[] = INCGFX_U32("graphics/trainers/front_pics/champion_diantha.png", ".4bpp.lz");
static const u32 gTrainerPalette_ChampionDiantha[] = INCGFX_U32("graphics/trainers/palettes/champion_diantha.pal", ".gbapal.lz");
static const u32 gTrainerFrontPic_ChampionLeon[] = INCGFX_U32("graphics/trainers/front_pics/champion_leon.png", ".4bpp.lz");
static const u32 gTrainerPalette_ChampionLeon[] = INCGFX_U32("graphics/trainers/palettes/champion_leon.pal", ".gbapal.lz");
static const u32 gTrainerFrontPic_ChampionAlain[] = INCGFX_U32("graphics/trainers/front_pics/champion_alain.png", ".4bpp.lz");
static const u32 gTrainerPalette_ChampionAlain[] = INCGFX_U32("graphics/trainers/palettes/champion_alain.pal", ".gbapal.lz");

#define CHAMPION_TRAINER_FRONT_PIC_COORDS \
    [TRAINER_PIC_CHAMPION_CYNTHIA] = {.size = 8, .y_offset = 1}, \
    [TRAINER_PIC_CHAMPION_ALDER] = {.size = 8, .y_offset = 1}, \
    [TRAINER_PIC_CHAMPION_DIANTHA] = {.size = 8, .y_offset = 1}, \
    [TRAINER_PIC_CHAMPION_LEON] = {.size = 8, .y_offset = 1}, \
    [TRAINER_PIC_ALAIN] = {.size = 8, .y_offset = 1}

#define CHAMPION_TRAINER_FRONT_PICS \
    [TRAINER_PIC_CHAMPION_CYNTHIA] = {gTrainerFrontPic_ChampionCynthia, TRAINER_PIC_SIZE, TRAINER_PIC_CHAMPION_CYNTHIA}, \
    [TRAINER_PIC_CHAMPION_ALDER] = {gTrainerFrontPic_ChampionAlder, TRAINER_PIC_SIZE, TRAINER_PIC_CHAMPION_ALDER}, \
    [TRAINER_PIC_CHAMPION_DIANTHA] = {gTrainerFrontPic_ChampionDiantha, TRAINER_PIC_SIZE, TRAINER_PIC_CHAMPION_DIANTHA}, \
    [TRAINER_PIC_CHAMPION_LEON] = {gTrainerFrontPic_ChampionLeon, TRAINER_PIC_SIZE, TRAINER_PIC_CHAMPION_LEON}, \
    [TRAINER_PIC_ALAIN] = {gTrainerFrontPic_ChampionAlain, TRAINER_PIC_SIZE, TRAINER_PIC_ALAIN}

#define CHAMPION_TRAINER_FRONT_PALETTES \
    [TRAINER_PIC_CHAMPION_CYNTHIA] = {gTrainerPalette_ChampionCynthia, TRAINER_PIC_CHAMPION_CYNTHIA}, \
    [TRAINER_PIC_CHAMPION_ALDER] = {gTrainerPalette_ChampionAlder, TRAINER_PIC_CHAMPION_ALDER}, \
    [TRAINER_PIC_CHAMPION_DIANTHA] = {gTrainerPalette_ChampionDiantha, TRAINER_PIC_CHAMPION_DIANTHA}, \
    [TRAINER_PIC_CHAMPION_LEON] = {gTrainerPalette_ChampionLeon, TRAINER_PIC_CHAMPION_LEON}, \
    [TRAINER_PIC_ALAIN] = {gTrainerPalette_ChampionAlain, TRAINER_PIC_ALAIN}

#define CHAMPION_TRAINER_FRONT_ANIMS \
    [TRAINER_PIC_CHAMPION_CYNTHIA] = sAnims_Hiker, \
    [TRAINER_PIC_CHAMPION_ALDER] = sAnims_Hiker, \
    [TRAINER_PIC_CHAMPION_DIANTHA] = sAnims_Hiker, \
    [TRAINER_PIC_CHAMPION_LEON] = sAnims_Hiker, \
    [TRAINER_PIC_ALAIN] = sAnims_Hiker
