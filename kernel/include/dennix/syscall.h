/* Copyright (c) 2016, 2017, 2018, 2019, 2020, 2021, 2022 Dennis Wölfing
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

/* kernel/include/dennix/syscall.h
 * Syscall numbers.
 */

#ifndef _DENNIX_SYSCALL_H
#define _DENNIX_SYSCALL_H

#define SYSCALL_EXIT_THREAD 0
#define SYSCALL_WRITE 1
#define SYSCALL_READ 2
#define SYSCALL_MMAP 3
#define SYSCALL_MUNMAP 4
#define SYSCALL_OPENAT 5
#define SYSCALL_CLOSE 6
#define SYSCALL_REGFORK 7
#define SYSCALL_EXECVE 8
#define SYSCALL_WAITPID 9
#define SYSCALL_FSTATAT 10
#define SYSCALL_GETDENTS 11
#define SYSCALL_CLOCK_NANOSLEEP 12
#define SYSCALL_TCGETATTR 13
#define SYSCALL_TCSETATTR 14
#define SYSCALL_FCHDIRAT 15
#define SYSCALL_CONFSTR 16
#define SYSCALL_FSTAT 17
#define SYSCALL_MKDIRAT 18
#define SYSCALL_UNLINKAT 19
#define SYSCALL_RENAMEAT 20
#define SYSCALL_LINKAT 21
#define SYSCALL_SYMLINKAT 22
#define SYSCALL_GETPID 23
#define SYSCALL_KILL 24
#define SYSCALL_SIGACTION 25
#define SYSCALL_ABORT 26
#define SYSCALL_CLOCK_GETTIME 27
#define SYSCALL_DUP3 28
#define SYSCALL_ISATTY 29
#define SYSCALL_PIPE2 30
#define SYSCALL_LSEEK 31
#define SYSCALL_UMASK 32
#define SYSCALL_FCHMODAT 33
#define SYSCALL_FCNTL 34
#define SYSCALL_UTIMENSAT 35
#define SYSCALL_DEVCTL 36
#define SYSCALL_GETPGID 37
#define SYSCALL_SETPGID 38
#define SYSCALL_READLINKAT 39
#define SYSCALL_FTRUNCATE 40
#define SYSCALL_SIGPROCMASK 41
#define SYSCALL_ALARM 42
#define SYSCALL_FCHMOD 43
#define SYSCALL_FUTIMENS 44
#define SYSCALL_GETRUSAGENS 45
#define SYSCALL_GETENTROPY 46
#define SYSCALL_FCHDIR 47
#define SYSCALL_FCHOWNAT 48
#define SYSCALL_MEMINFO 49
#define SYSCALL_SIGTIMEDWAIT 50
#define SYSCALL_PPOLL 51
#define SYSCALL_SOCKET 52
#define SYSCALL_BIND 53
#define SYSCALL_LISTEN 54
#define SYSCALL_CONNECT 55
#define SYSCALL_ACCEPT4 56
#define SYSCALL_MOUNT 57
#define SYSCALL_UNMOUNT 58
#define SYSCALL_FPATHCONF 59
#define SYSCALL_FSSYNC 60
#define SYSCALL_FCHOWN 61
#define SYSCALL_SETSID 62

#define NUM_SYSCALLS 63

#endif
