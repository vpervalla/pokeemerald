// Runs a GBA ROM headless in libmgba and feeds it a list of commands from stdin, one per line:
//   wait N          run N frames with no key held
//   press KEY [N]   hold KEY (A B SELECT START RIGHT LEFT UP DOWN R L) for N frames (default 6), then release for 6
//   shot FILE       write the screen to FILE (PPM)
//   keys MASK N     hold the keys in MASK (bit 0 A, 1 B, 2 SELECT, 3 START, 4 RIGHT, 5 LEFT, 6 UP, 7 DOWN, 8 R, 9 L) for N frames
//   peek ADDR LEN   print LEN bytes from the bus at ADDR (hex) as hex, on one line
//   savestate FILE / loadstate FILE
//   sync            print "ok" (every reply is flushed, so a script can drive this through a pipe)
// Build: cc -O2 -o mgba_driver mgba_driver.c -lmgba
// Usage: mgba_driver ROM SAVE < commands    (SAVE is the cartridge's flash: created if missing, written on exit)
#include <mgba/core/core.h>
#include <mgba/core/config.h>
#include <mgba/core/log.h>
#include <mgba-util/vfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

static void QuietLog(struct mLogger *l, int c, enum mLogLevel lv, const char *f, va_list a) { (void)l; (void)c; (void)lv; (void)f; (void)a; }

static const char *const kKeys[] = {"A", "B", "SELECT", "START", "RIGHT", "LEFT", "UP", "DOWN", "R", "L"};

int main(int argc, char **argv)
{
    static struct mLogger logger = {.log = QuietLog};
    struct mCore *core;
    color_t *video;
    unsigned w, h;
    char line[512];

    if (argc < 3)
        return 1;
    mLogSetDefaultLogger(&logger);
    core = mCoreFind(argv[1]);
    if (!core || !core->init(core))
        return 2;
    mCoreInitConfig(core, NULL);
    core->desiredVideoDimensions(core, &w, &h);
    video = calloc(w * h, sizeof(*video));
    core->setVideoBuffer(core, video, w);
    if (!mCoreLoadFile(core, argv[1]))
        return 3;
    core->loadSave(core, VFileOpen(argv[2], O_CREAT | O_RDWR));
    core->reset(core);

    while (fgets(line, sizeof(line), stdin))
    {
        char cmd[32], arg[400];
        int n = 0, i, k;
        arg[0] = 0;
        if (sscanf(line, "%31s %399s %d", cmd, arg, &n) < 1 || cmd[0] == '#')
            continue;
        if (!strcmp(cmd, "wait"))
        {
            for (i = 0, n = atoi(arg); i < n; i++)
                core->runFrame(core);
        }
        else if (!strcmp(cmd, "press"))
        {
            for (k = 0; k < 10 && strcmp(kKeys[k], arg); k++)
                ;
            if (k == 10)
                return 4;
            core->setKeys(core, 1 << k);
            for (i = 0; i < (n ? n : 6); i++)
                core->runFrame(core);
            core->setKeys(core, 0);
            for (i = 0; i < 6; i++)
                core->runFrame(core);
        }
        else if (!strcmp(cmd, "keys"))
        {
            core->setKeys(core, strtoul(arg, NULL, 0));
            for (i = 0; i < n; i++)
                core->runFrame(core);
            core->setKeys(core, 0);
        }
        else if (!strcmp(cmd, "peek"))
        {
            uint32_t addr = strtoul(arg, NULL, 16);
            for (i = 0; i < n; i++)
                printf("%02x", core->busRead8(core, addr + i));
            printf("\n");
        }
        else if (!strcmp(cmd, "savestate") || !strcmp(cmd, "loadstate"))
        {
            size_t size = core->stateSize(core);
            void *state = malloc(size);
            FILE *f = fopen(arg, cmd[0] == 's' ? "wb" : "rb");
            if (cmd[0] == 's')
                core->saveState(core, state), fwrite(state, 1, size, f);
            else if (fread(state, 1, size, f) == size)
                core->loadState(core, state);
            fclose(f);
            free(state);
        }
        else if (!strcmp(cmd, "sync"))
        {
            printf("ok\n");
        }
        else if (!strcmp(cmd, "shot"))
        {
            FILE *f = fopen(arg, "wb");
            fprintf(f, "P6 %u %u 255\n", w, h);
            for (i = 0; i < (int)(w * h); i++)
            {
                uint32_t c = video[i];
                fputc(c & 0xFF, f), fputc((c >> 8) & 0xFF, f), fputc((c >> 16) & 0xFF, f);
            }
            fclose(f);
        }
        fflush(stdout);
    }
    core->deinit(core);
    return 0;
}
