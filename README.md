Pom-Timer
---------

`pom_timer` is a minimalistic Pomodoro Timer that you can use from the command line.
If you hate using bloated web interfaces and don't bother to write something similar
to this by yourself, go ahead and download the source files. It is just a few lines
of C code, really. We don't even have a Makefile! Then, simply compile using your
favorite C compiler, e.g.

```bash
gcc pom_timer.c -o pom_timer
```

You can get usage information via

```bash
pom_timer --help
```

To keep things simple, you can only configure work and break times as well as the
path to a log file (not required, though). Defaults are 25 minutes of work and a 5
minute break time afterwards (the "classic" Pomodoro Technique, I guess).

License
-------
Pom-Timer is licensed under the [GPLv3](https://www.gnu.org/licenses/gpl-3.0.en.html).
