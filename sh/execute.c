/* Copyright (c) 2018, 2019 Dennis Wölfing
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* sh/execute.c
 * Shell command execution.
 */

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/wait.h>

#include "builtins.h"
#include "execute.h"
#include "expand.h"
#include "sh.h"

static volatile sig_atomic_t pipelineReady;

static void sigusr1Handler(int signum) {
    (void) signum;
    pipelineReady = 1;
}

static int executePipeline(struct Pipeline* pipeline);
static int executeSimpleCommand(struct SimpleCommand* simpleCommand,
        bool subshell);
static noreturn void executeUtility(char** arguments,
        struct Redirection* redirections, size_t numRedirections);
static int forkAndExecuteUtility(char** arguments,
        struct Redirection* redirections, size_t numRedirections);
static const char* getExecutablePath(const char* command);
static bool performRedirections(struct Redirection* redirections,
        size_t numRedirections);
static void resetSignals(void);
static int waitForCommand(pid_t pid);

int execute(struct CompleteCommand* command) {
    return executePipeline(&command->pipeline);
}

static int executePipeline(struct Pipeline* pipeline) {
    if (pipeline->numCommands <= 1) {
        return executeSimpleCommand(&pipeline->commands[0], false);
    }

    int inputFd;
    pid_t pgid = -1;

    for (size_t i = 0; i < pipeline->numCommands; i++) {
        bool firstInPipeline = i == 0;
        bool lastInPipeline = i == pipeline->numCommands - 1;

        int pipeFds[2];
        if (!lastInPipeline) {
            if (pipe(pipeFds) < 0) err(1, "pipe");
        }

        pid_t pid = fork();

        if (pid < 0) {
            err(1, "fork");
        } else if (pid == 0) {
            if (!lastInPipeline) {
                close(pipeFds[0]);
            }

            if (!firstInPipeline) {
                if (!moveFd(inputFd, 0)) {
                    warn("cannot move file descriptor");
                    _Exit(126);
                }
            }

            if (!lastInPipeline) {
                if (!moveFd(pipeFds[1], 1)) {
                    warn("cannot move file descriptor");
                    _Exit(126);
                }
            }

            if (firstInPipeline) {
                signal(SIGUSR1, sigusr1Handler);
            }
            setpgid(0, pgid == -1 ? 0 : pgid);

            if (firstInPipeline) {
                if (inputIsTerminal) {
                    tcsetpgrp(0, getpgid(0));
                }

                while (!pipelineReady) {
                    // Wait for all processes in the pipeline to start.
                    sched_yield();
                }
            }

            resetSignals();
            executeSimpleCommand(&pipeline->commands[i], true);
        } else {
            if (!lastInPipeline) {
                close(pipeFds[1]);
                if (!firstInPipeline) {
                    close(inputFd);
                    setpgid(pid, pgid);
                } else {
                    pgid = pid;
                    while (getpgid(pid) != pgid) {
                        sched_yield();
                    }
                }

                inputFd = pipeFds[0];
            } else {
                assert(inputFd != 0);
                close(inputFd);

                setpgid(pid, pgid);
                // Inform the first process in the pipeline that all processes
                // have started.
                kill(pgid, SIGUSR1);

                int exitStatus = waitForCommand(pid);

                for (size_t j = 0; j < pipeline->numCommands - 1; j++) {
                    // Wait for all other commands of the pipeline.
                    int status;
                    wait(&status);
                }
                if (pipeline->bang) return !exitStatus;
                return exitStatus;
            }
        }
    }

    assert(false); // This should be unreachable.
}

static int executeSimpleCommand(struct SimpleCommand* simpleCommand,
        bool subshell) {
    int argc = simpleCommand->numWords;

    char** arguments = malloc((argc + 1) * sizeof(char*));
    if (!arguments) err(1, "malloc");
    for (int i = 0; i < argc; i++) {
        arguments[i] = expandWord(simpleCommand->words[i]);
    }

    arguments[argc] = NULL;

    size_t numRedirections = simpleCommand->numRedirections;
    struct Redirection* redirections = malloc(numRedirections *
            sizeof(struct Redirection));
    if (!redirections) err(1, "malloc");
    for (size_t i = 0; i < numRedirections; i++) {
        redirections[i] = simpleCommand->redirections[i];
        redirections[i].filename = expandWord(redirections[i].filename);
    }

    const char* command = arguments[0];
    if (!command) {
        assert(numRedirections > 0);
    // Special built-ins
    } else if (strcmp(command, "exit") == 0) {
        exit(0);
    // Regular built-ins
    } else {
        for (struct builtin* builtin = builtins; builtin->name; builtin++) {
            if (strcmp(command, builtin->name) == 0) {
                int result = builtin->func(argc, arguments);
                free(arguments);
                free(redirections);
                if (subshell) {
                    exit(result);
                }
                return result;
            }
        }
    }

    if (subshell) {
        executeUtility(arguments, redirections, numRedirections);
    } else {
        int result = forkAndExecuteUtility(arguments, redirections,
                numRedirections);
        free(arguments);
        free(redirections);
        return result;
    }
}

static noreturn void executeUtility(char** arguments,
        struct Redirection* redirections, size_t numRedirections) {
    const char* command = arguments[0];
    if (!performRedirections(redirections, numRedirections)) {
        _Exit(126);
    }

    if (!command) _Exit(0);

    if (!strchr(command, '/')) {
        command = getExecutablePath(command);
    }

    if (command) {
        execv(command, arguments);
        warn("execv: '%s'", command);
        _Exit(126);
    } else {
        warnx("'%s': Command not found", arguments[0]);
        _Exit(127);
    }
}

static int forkAndExecuteUtility(char** arguments,
        struct Redirection* redirections, size_t numRedirections) {
    pid_t pid = fork();

    if (pid < 0) {
        err(1, "fork");
    } else if (pid == 0) {
        setpgid(0, 0);
        if (inputIsTerminal) {
            tcsetpgrp(0, getpgid(0));
        }

        resetSignals();
        executeUtility(arguments, redirections, numRedirections);
    } else {
        return waitForCommand(pid);
    }
}

static const char* getExecutablePath(const char* command) {
    size_t commandLength = strlen(command);
    const char* path = getenv("PATH");

    while (*path) {
        size_t length = strcspn(path, ":");
        char* buffer = malloc(commandLength + length + 2);
        if (!buffer) {
            warn("malloc");
            _Exit(126);
        }

        memcpy(buffer, path, length);
        buffer[length] = '/';
        memcpy(buffer + length + 1, command, commandLength);
        buffer[commandLength + length + 1] = '\0';

        if (access(buffer, X_OK) == 0) {
            return buffer;
        }

        free(buffer);
        path += length + 1;
    }

    return NULL;
}

static bool performRedirections(struct Redirection* redirections,
        size_t numRedirections) {
    for (size_t i = 0; i < numRedirections; i++) {
        struct Redirection redirection = redirections[i];
        int fd;

        if (redirection.filenameIsFd) {
            char* tail;
            fd = strtol(redirection.filename, &tail, 10);
            if (*tail) {
                errno = EBADF;
                warn("'%s'", redirection.filename);
                return false;
            }
        } else {
            fd = open(redirection.filename, redirection.flags, 0666);
            if (fd < 0) {
                warn("open: '%s'", redirection.filename);
                return false;
            }
        }

        if (dup2(fd, redirection.fd) < 0) {
            warn("dup2: '%s'", redirection.filename);
            return false;
        }
        if (!redirection.filenameIsFd && fd != redirection.fd) {
            close(fd);
        }
    }

    return true;
}

static void resetSignals(void) {
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
}

static int waitForCommand(pid_t pid) {
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        err(1, "waitpid");
    }

    if (inputIsTerminal) {
        tcsetpgrp(0, getpgid(0));
    }

    if (WIFSIGNALED(status)) {
        if (inputIsTerminal) {
            tcsetattr(0, TCSAFLUSH, &termios);
        }

        int signum = WTERMSIG(status);
        if (signum == SIGINT) {
            fputc('\n', stderr);
        } else {
            fprintf(stderr, "%s\n", strsignal(signum));
        }
        return 128 + signum;
    }

    return WEXITSTATUS(status);
}
