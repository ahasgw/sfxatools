/***********************************************************************
 * $Id: mmfile.c,v 1.1.1.1 2005/02/02 10:39:30 aki Exp $
 *
 * Memory mapped file
 * Copyright (C) 2005 RIKEN. All rights reserved.
 * Written by Aki Hasegawa <aki@gsc.riken.jp>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 ***********************************************************************/

#ifndef CONFIG_H_INCLUDED
# define CONFIG_H_INCLUDED 1
# include <config.h>
#endif

#include "mmfile.h"

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <errno.h>
#include <string.h>

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif
#if HAVE_SYS_FILE_H
# include <sys/file.h>
#endif
#if HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif

#include <stdint.h>

/*======================================================================
 * public function definitions
 *======================================================================*/

/*----------------------------------------------------------------------
 * ctor & dtor
 *----------------------------------------------------------------------*/

/* allocate mmfile_t object */
mmfile_t *mmfile_new(const char *path)
{
    mmfile_t *mf = (mmfile_t*)malloc(sizeof(mmfile_t));
    if (mf != NULL && (mmfile_init(mf, path) != 0))
	free(mf), mf = NULL;
    return mf;
}

/* initialize struct mmfile_t */
int mmfile_init(mmfile_t *mf, const char *path)
{
    mf->ptr = NULL;
    mf->sz = -1;
    mf->fd = -1;
    if ((mf->path = strdup(path)) == NULL)
	return errno;
    return 0;
}

/* free */
void mmfile_free(mmfile_t *mf)
{
    free(mf->path), mf->path = NULL;
    mf->ptr = NULL;
    mf->sz = -1;
    mf->fd = -1;
}

/* delete mmfile_t object */
void mmfile_delete(mmfile_t *mf)
{
    mmfile_free(mf);
    free(mf);
}

/*----------------------------------------------------------------------
 * accessor
 *----------------------------------------------------------------------*/

/* get file path */
const char *mmfile_path(const mmfile_t *mf)
{
    return mf->path;
}

/* get mmapped pointer */
void *mmfile_ptr(const mmfile_t *mf)
{
    return mf->ptr;
}

/* get mmapped length */
off_t mmfile_len(const mmfile_t *mf)
{
    return mf->sz;
}

/* get file descriptor */
int mmfile_fileno(const mmfile_t *mf)
{
    return mf->fd;
}

/* truncate a file to a specified length */
int mmfile_truncate(mmfile_t *mf, off_t len)
{
    if (ftruncate(mf->fd, len) != 0)
	return errno;
    return 0;
}

/*----------------------------------------------------------------------
 * mapper
 *----------------------------------------------------------------------*/

/* map shared read only */
int mmfile_map_shared_rd(mmfile_t *mf)
{
    int fd;
    void *ptr;
    struct stat st;

    if ((fd = open(mf->path, O_RDONLY)) == -1)
	return errno;

    if (fstat(fd, &st) != 0)
	return errno;		    /* Is the close(fd) required? */

#if SIZEOF_OFF_T > SIZEOF_SIZE_T
    if (st.st_size > (SIZE_MAX / 2))	/* This should be ENOMEM? */
	return (errno = EFBIG);	    /* Is the close(fd) required? */
#endif
    ptr = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
	return errno;		    /* Is the close(fd) required? */

    mf->ptr = ptr;
    mf->sz = st.st_size;
    mf->fd = fd;
    return 0;
}

/* map shared read/write-able (modification on memory reflects file object) */
int mmfile_map_shared_rw(mmfile_t *mf, off_t trunc_len)
{
    int fd;
    void *ptr;
    off_t len = trunc_len;
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    if ((fd = open(mf->path, O_CREAT | O_RDWR, mode)) == -1) 
	return errno;

    if (trunc_len < 0) {
	struct stat st;
	if (fstat(fd, &st) != 0)
	    return errno;	    /* Is the close(fd) required? */
	len = st.st_size;
    }

#if SIZEOF_OFF_T > SIZEOF_SIZE_T
    if (len > (SIZE_MAX / 2))	    /* This should be ENOMEM? */
	return (errno = EFBIG);	    /* Is the close(fd) required? */
#endif

    if (trunc_len >= 0) {
	if (ftruncate(fd, trunc_len) != 0)
	    return errno;	    /* Is the close(fd) required? */
    }

    ptr = mmap(NULL, (size_t)len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
	return errno;		    /* Is the close(fd) required? */

    mf->ptr = ptr;
    mf->sz = len;
    mf->fd = fd;
    return 0;
}

/* map private read only */
int mmfile_map_private_rd(mmfile_t *mf)
{
    int fd;
    void *ptr;
    struct stat st;

    if ((fd = open(mf->path, O_RDONLY)) == -1)
	return errno;

    if (fstat(fd, &st) != 0)
	return errno;		    /* Is the close(fd) required? */

#if SIZEOF_OFF_T > SIZEOF_SIZE_T
    if (st.st_size > (SIZE_MAX / 2))	/* This should be ENOMEM? */
	return (errno = EFBIG);	    /* Is the close(fd) required? */
#endif
    ptr = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED)
	return errno;		    /* Is the close(fd) required? */

    mf->ptr = ptr;
    mf->sz = st.st_size;
    mf->fd = fd;
    return 0;
}

/* map private read/write-able on memory (no write-back to the file object) */
int mmfile_map_private_rw(mmfile_t *mf)
{
    int fd;
    void *ptr;
    struct stat st;

    if ((fd = open(mf->path, O_RDONLY)) == -1)
	return errno;

    if (fstat(fd, &st) != 0)
	return errno;		    /* Is the close(fd) required? */

#if SIZEOF_OFF_T > SIZEOF_SIZE_T
    if (st.st_size > (SIZE_MAX / 2))	/* This should be ENOMEM? */
	return (errno = EFBIG);	    /* Is the close(fd) required? */
#endif
    ptr = mmap(NULL, (size_t)st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED)
	return errno;		    /* Is the close(fd) required? */

    mf->ptr = ptr;
    mf->sz = st.st_size;
    mf->fd = fd;
    return 0;
}

/* synchronize memory with physical storage */
int mmfile_sync(mmfile_t *mf, int flag)
{
    if (msync(mf->ptr, (size_t)mf->sz, flag) == -1)
	return errno;
    return 0;
}

/* unmap */
int mmfile_unmap(mmfile_t *mf)
{
    if (munmap(mf->ptr, (size_t)mf->sz) != 0)
	return errno;
    mf->ptr = NULL, mf->sz = -1;
    if (mf->fd >= 0 && (close(mf->fd) != 0))
	return errno;
    mf->fd = -1;
    return 0;
}
