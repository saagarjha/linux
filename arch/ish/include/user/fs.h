#ifndef __ISH_USER_FS_H
#define __ISH_USER_FS_H

#define STDIN_FD 0
#define STDOUT_FD 1
#define STDERR_FD 2
ssize_t host_write(int fd, const void *data, size_t len);
ssize_t host_read(int fd, void *data, size_t len);
int fd_set_nonblock(int fd);

void termio_make_raw(int fd);

#define LISTEN_READ 1
typedef void (*fd_callback_t)(int fd, int types, void *data);
int fd_add_listener(int fd, int types, fd_callback_t callback, void *data);

#endif
