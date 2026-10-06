#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba/core/config.h>
#include <mgba-util/vfs.h>
#include <mgba/core/log.h>
static void nolog(struct mLogger* l, int c, enum mLogLevel lv, const char* f, va_list a) {}
static struct mLogger silent = { .log = nolog };
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

static struct mCore* core;
static color_t* buf;
static struct VFile* savevf;

int emu_init(const char* rom, const char* sav) {
    mLogSetDefaultLogger(&silent);
    core = mCoreFind(rom);
    if (!core) return -1;
    core->init(core);
    mCoreInitConfig(core, NULL);
    unsigned w, h;
    core->desiredVideoDimensions(core, &w, &h);
    buf = calloc(w * h, sizeof(color_t));
    core->setVideoBuffer(core, buf, w);
    if (!mCoreLoadFile(core, rom)) return -2;
    savevf = VFileOpen(sav, O_CREAT | O_RDWR);
    if (!savevf) return -3;
    core->loadSave(core, savevf);
    core->reset(core);
    return (int)(w * 1000 + h);
}
void emu_run(int keys, int frames) {
    core->setKeys(core, keys);
    for (int i = 0; i < frames; i++) core->runFrame(core);
}
color_t* emu_frame(void) { return buf; }
int emu_save_state(const char* path) {
    struct VFile* vf = VFileOpen(path, O_CREAT | O_TRUNC | O_RDWR);
    if (!vf) return 0;
    bool ok = mCoreSaveStateNamed(core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
    vf->close(vf);
    return ok;
}
int emu_load_state(const char* path) {
    struct VFile* vf = VFileOpen(path, O_RDONLY);
    if (!vf) return 0;
    bool ok = mCoreLoadStateNamed(core, vf, SAVESTATE_SAVEDATA | SAVESTATE_RTC);
    vf->close(vf);
    return ok;
}
int emu_read8(unsigned addr) { return core->busRead8(core, addr); }
void emu_read(unsigned addr, unsigned char* out, int n) { for (int i = 0; i < n; i++) out[i] = core->busRead8(core, addr + i); }
void emu_write8(unsigned addr, int v) { core->busWrite8(core, addr, v); }
void emu_close(void) { core->deinit(core); }
