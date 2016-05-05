#include "pdp8_defs.h"
#include "pidp8_gpio.h"

#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <unistd.h>

t_stat execute_cmd(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char *cbuf;
    vasprintf(&cbuf, fmt, ap);
    va_end(ap);

    if (!cbuf) {
        sim_printf("ERROR: vasprintf failed\n");
        return SCPE_MEM;
    }

    sim_printf("Executing command: %s\n", cbuf);

    char gbuf[64];
    char *cptr = get_glyph (cbuf, gbuf, 0);             /* get command glyph */
    sim_switches = 0;                                   /* init switches */
    t_stat stat;
    CTAB *cmdp;
    if ((cmdp = find_cmd (gbuf)))                       /* lookup command */
        stat = cmdp->action (cmdp->arg, cptr);          /* if found, exec */
    else
        stat = SCPE_UNK;

    if (stat >= SCPE_BASE)                              /* error? */
        sim_printf ("%s\n", sim_error_text (stat));

    free(cbuf);

    return stat;
}

char *mountedFiles[8] = {0};

static void mountUSBStickFile(int devNo)
{
    const char *devcodes[] = {
        "ptr", // PTR paper tape reader
        "ptp", // High speed paper tape punch
        "dt0", // TC08 DECtape (#8 is first!)
        "dt1",
        "rx0", // RX8E (8/e peripheral!)
        "rx1",
        "rl0"  // RL8A
    };
    const char *devcode = devcodes[devNo - 1];

    // take first 2 digits of devcode as extension
    char extension[4];
    sprintf(extension, ".%.2s", devcode);
    char *mountFile = NULL;

    int i;
    for (i=0; i<8; i++) {
        char mountPoint[PATH_MAX];
        sprintf(mountPoint, "/media/usb%d", i);

        DIR *dp;
        struct dirent *dent;
        dp = opendir(mountPoint);

        if (!dp) {
            sim_printf("Couldn't open dir %s\n", mountPoint);
            continue;
        }

        while ((dent = readdir(dp))) {
            char *dot = strrchr(dent->d_name, '.');
            if (!dot || strcmp(dot, extension))
                continue;

            char filename[PATH_MAX];
            sprintf(filename, "%s/%s", mountPoint, dent->d_name);
            if (access(filename, R_OK))
                goto next_file;

            int j;
            for (j=0; j<8; j++) {
                if (mountedFiles[j] && !strcmp(filename, mountedFiles[i]))
                    goto next_file;
            }
            // found one; stop looking
            mountFile = filename;
            goto mount_file;
next_file: ;
        }
        closedir(dp);
    }

    if (!mountFile) {
        sim_printf("No file found to mount on %s\n", devcode);
        return;
    }

mount_file:
    if (mountedFiles[devNo])
        free(mountedFiles[devNo]);

    t_stat result = execute_cmd("ATTACH %s %s", devcode, mountFile);
    if (result == SCPE_OK)
        mountedFiles[devNo] = strdup(mountFile);
    else
        sim_printf("Failed to attach file %s\n", mountFile);
}

int pidp8_handle_sing_step(void) {
    // this bit of code detects SING_STEP as the special features switch.
    if (!switches_event.SING_STEP)
        return 0;
    switches_event.SING_STEP = 0;

    // Scan for shutdown command (Sing_Step + Sing_inst + Start)

    if (switches.SING_INST && switches.START)
    {
        sim_printf("Shutdown request\n");
        execute_cmd("! shutdown -h -t 1 now");
        return SCPE_EXIT;
    }

    // Scan for host reboot command (Sing_Step + Sing_Inst + Cont)

    if (switches.SING_INST && switches.CONT)
    {
        sim_printf("Reboot request\n");
        execute_cmd("! reboot");
        return SCPE_EXIT;
    }

    // Scan for mount command (Sing_Step + Sing_Inst + Load Add)

    if (switches.SING_INST && switches.LOAD_ADD)
    {
        execute_cmd("! /opt/pidp8/bin/automount");
        return 0;
    }

    // Scan for unmount command (Sing_Step + Sing_Inst + Deposit)

    if (switches.SING_INST && switches.DEP)
    {
        execute_cmd("! /opt/pidp8/bin/unmount");
        return 0;
    }

    // Scan DF to see if any devices need to be mounted (DF=0 --> nothing to mount)

    int device = switches.DF;
    if (device)
        mountUSBStickFile(device);

    // Scan IF to see if we need to reboot with a new bootscript

    device = switches.IF;
    if (device!=0)
    {
        execute_cmd("DO /opt/pidp8/bootscripts/%d.script", device);
        return STOP_HALT;
    }

}
