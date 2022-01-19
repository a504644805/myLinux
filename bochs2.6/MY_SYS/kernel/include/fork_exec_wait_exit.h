#ifndef FORK_EXEC_WAIT_EXIT_H
#define FORK_EXEC_WAIT_EXIT_H

int sys_fork();
void sys_execv(const char *path, char *const argv[]);

#endif