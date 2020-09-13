#ifndef __ISH_USER_FS_H
#define __ISH_USER_FS_H

#define STDIN_FD 0
#define STDOUT_FD 1
#define STDERR_FD 2
ssize_t host_write(int fd, const void *data, size_t len);
ssize_t host_read(int fd, void *data, size_t len);
ssize_t host_pwrite(int fd, void *data, size_t len, off_t offset);
ssize_t host_pread(int fd, void *data, size_t len, off_t offset);
int fd_set_nonblock(int fd);
int host_open(const char *path, int flags);

void termio_make_raw(int fd);

int fd_add_irq(int fd, int types, int irq);

#endif
