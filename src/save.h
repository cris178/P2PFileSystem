//This file will be where we save what happens in the file system.
//Similar to log.h

#ifndef _SAVE_H_
#define _SAVE_H_
#include <stdio.h>

//  macro to save fields in structs.
#define save_struct(st, field, format, typecast) \
    save_msg("    " #field " = " #format "\n", typecast st->field)

FILE *save_open(void);
void save_msg(const char *format, ...);
void save_conn(struct fuse_conn_info *conn);
int save_error(char *func);
void save_fi(struct fuse_file_info *fi);
void save_fuse_context(struct fuse_context *context);
void save_retstat(char *func, int retstat);
void save_stat(struct stat *si);
void save_statvfs(struct statvfs *sv);
int save_syscall(char *func, int retstat, int min_ret);
void save_utime(struct utimbuf *buf);

#endif
