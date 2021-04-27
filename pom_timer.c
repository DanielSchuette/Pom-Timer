/* A minimalistic, terminal-based Pomodoro Timer.
 *
 * Pom_Timer Copyright (C) 2021 Daniel Schuette
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "pom_timer.h"

int main(int argc, char **argv)
{
    setup_term();
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
        fprintf(stderr, "Failed to catch signal.\n");

    /* parse arguments and print them out */
    consume_args(argc, argv);
    print_info();

    /* loop infinitely, changing state from work to break to work */
    while (true) {
        if (is_work(&timer)) {
            print_and_sleep(&timer, 1, "Time to Work ");
            inc(&timer);
        }
        else {
            take_break(&timer);
        }
    }

    return 0;
}

void print_and_sleep(ptimer *t, int s, const char *msg)
{
    fprintf(stderr, "%s[%s%2dm:%2ds%s, done %s%d%s time%s]\r",
            msg, ANSI_GREEN, t->mins, t->secs, ANSI_RESET, ANSI_YELLOW,
            t->num_work, ANSI_RESET, t->num_work==1 ? "" : "s");
    sleep(s);
}

void inc(ptimer *t)
{
    if (t->secs == 59) {
        t->mins++;
        t->secs = 0;
    } else {
        t->secs++;
    }
}

void take_break(ptimer *t)
{
    /* reset timer and terminal for break */
    t->secs = 0; t->mins = 0; t->num_work++;
    clear_line();

    while (is_break(t)) {
        fprintf(stderr, "Take a break [%s%2dm:%2ds%s]\r",
                ANSI_GREEN, t->mins, t->secs, ANSI_RESET);
        sleep(1);
        inc(t);
    }

    /* reset timer and terminal for main loop use */
    t->secs = 0; t->mins = 0; t->num_break++;
    clear_line();
}

bool is_work(ptimer *t)
{
    if (t->mins == config.work_time)
        return false;
    return true;
}

bool is_break(ptimer *t)
{
    if (t->mins == config.break_time)
        return false;
    return true;
}

void clear_line(void)
{
    char *empty;
    int spaces = 80; /* better use terminal width here */

    empty = (char *)malloc(spaces);
    for (int i = 0; i < spaces; empty[i++] = ' ')
        ;
    /* If we run over the line ending and end up in the next line, this will
     * break.
     */
    fprintf(stderr, "%s\r", empty);

    free(empty);
}

void consume_args(int argc, char **argv)
{
    while (--argc > 0 && ++argv) {
        /* interpret flags */
        if (!strcmp(*argv, "--help") || !strcmp(*argv, "-h")) {
            fprintf(stderr, "%s", help_msg);
            exit(0);
        }

        /* interpret options & their arguments */
        if (!strcmp(*argv, "--work") || !strcmp(*argv, "-w")) {
            get_option(&argc, &argv, "--work");
            continue;
        }
        if (!strcmp(*argv, "--break") || !strcmp(*argv, "-b")) {
            get_option(&argc, &argv, "--break");
            continue;
        }
        if (!strcmp(*argv, "--log-file") || !strcmp(*argv, "-f")) {
            get_option(&argc, &argv, "--log-file");
            continue;
        }
        bad_option(0, *argv, MODE_CONTINUE);
    }
}

void get_option(int *argc, char ***argv, const char *opt_name)
{
    int ntime = 0;

    if (*argc > 1) { /* check if there are still values */
        /* modify argc & argv and parse the parameter */
        (*argv)++;
        (*argc)--;

        if (!strcmp("--work", opt_name) || !strcmp("--break", opt_name)) {
            ntime = atoi(**argv);
            if (ntime < 1)
                bad_option(ntime, opt_name, MODE_FAIL);
        }
        if (!strcmp("--log-file", opt_name)) {
            config.save_path = (char *)malloc(BUFSIZ*sizeof(char));
            strncpy(config.save_path, **argv, BUFSIZ);
        }

        /* conditionally add parsed parameter to configs */
        if (!strcmp("--work", opt_name))
            config.work_time = ntime;
        if (!strcmp("--break", opt_name))
            config.break_time = ntime;
    } else {
        fprintf(stderr, "%swarning%s: Need value after %s.\n",
                ANSI_YELLOW, ANSI_RESET, opt_name);
    }
}

void bad_option(int val, const char *option, int mode)
{
    if (mode == MODE_FAIL) {
        fprintf(stderr, "%serror%s: Provided bad value %d to %s "
                "(must be int > 0).\n", ANSI_RED, ANSI_RESET, val, option);
        exit(1);
    } else if (mode == MODE_CONTINUE) {
        fprintf(stderr, "%swarning%s: Provided bad option %s.\n",
                ANSI_YELLOW, ANSI_RESET, option);
    }
}

void sigint_handler(int signum)
{
    (void)signum; /* gcc doesn't want us to omit the param name */
    if (config.save_path)
        save_stats(config.save_path);
    fprintf(stderr, "\n\n%sDone%s.\n", ANSI_GREEN, ANSI_RESET);

    free(config.save_path);
    exit(0);
}

void save_stats(const char *path)
{
    FILE *file;
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    int elapsed;

    /* open the log file, calculate the time and save it */
    file = fopen(path, "a");
    elapsed = (timer.num_work*config.work_time*60)+   /* completed work secs */
              (timer.num_break*config.break_time*60)+ /* completed break secs */
              (timer.mins*60+timer.secs);             /* secs on clock */
    fprintf(file, "[%2d/%02d/%d %2dh:%2dm]\t%dhrs\t%dmins (%dsecs)\n",
            tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, elapsed/3600, elapsed/60, elapsed);
    fclose(file);
}

void setup_term(void)
{
    struct termios t;

    /* disable ECHOCTL, not std-c though so compile conditionally */
    tcgetattr(0, &t);
#ifdef ECHOCTL
    t.c_lflag &= ~ECHOCTL;
#endif
    tcsetattr(0, TCSANOW, &t);
}

void print_info(void)
{
    fprintf(stderr, "Work time: %s%dmin%s%s, "
            "Break time: %s%dmin%s%s.\n",
            ANSI_GREEN, config.work_time,
            config.work_time==0 ? "" : "s", ANSI_RESET,
            ANSI_RED, config.break_time,
            config.break_time==0 ? "" : "s", ANSI_RESET);
    if (config.save_path)
        fprintf(stderr, "Saving logs to `%s%s%s'.\n",
                ANSI_BLUE, config.save_path, ANSI_RESET);
    else
        fprintf(stderr, "%sNot%s saving logs.\n",
                ANSI_BLUE, ANSI_RESET);
    fprintf(stderr, "Exit with %sctrl+c%s.\n\n",
            ANSI_RED, ANSI_RESET);
}
