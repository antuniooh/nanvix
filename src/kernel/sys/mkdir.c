/*
 * Copyright(C) 2011-2016 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 * 
 * This file is part of Nanvix.
 * 
 * Nanvix is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Nanvix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Nanvix. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nanvix/const.h>
#include <nanvix/dev.h>
#include <nanvix/fs.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <nanvix/klib.h>

/*
 * Returns access permissions.
 */
#define PERM(o) \
	((ACCMODE(o) == O_RDWR) ? (MAY_READ | MAY_WRITE) : ((ACCMODE(o) == O_WRONLY) ? MAY_WRITE : (MAY_READ | ((o & O_TRUNC) ? MAY_WRITE : 0))))

/*
 * Creates a Directory.
 */
PRIVATE struct inode *do_mkdir(struct inode *d, const char *name, mode_t mode, int oflag)
{
	struct inode *i;

	/* Not asked to create file. */
	if (!(oflag & O_CREAT))
	{
		curr_proc->errno = -ENOENT;
		return (NULL);
	}

	i = inode_alloc(d->sb);

	/* Failed to allocate inode. */
	if (i == NULL)
		return (NULL);

	mode = S_IFDIR;
	mode |= S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
	i->mode = mode;

	/* Failed to add directory entry. */
	if (dir_add(d, i, name))
	{
		inode_put(i);
		return (NULL);
	}

	// Links
	dir_add(i, i, ".");
	dir_add(i, d, "..");

	inode_unlock(i);

	return (i);
}

/*
 * create direcotry.
 */
PRIVATE struct inode *createDirectory(const char *path, int oflag, mode_t mode)
{
	const char *name;	  /* File name.           */
	struct inode *dinode; /* Directory's inode.   */
	ino_t num;			  /* File's inode number. */
	dev_t dev;			  /* File's device.       */
	struct inode *i;	  /* File's inode.        */

	dinode = inode_dname(path, &name);

	/* Failed to get directory. */
	if (dinode == NULL)
		return (NULL);

	num = dir_search(dinode, name);

	/* File does not exist. */
	if (num == INODE_NULL)
	{
		i = do_mkdir(dinode, name, mode, oflag);

		/* Failed to create inode. */
		if (i == NULL)
		{
			inode_put(dinode);
			return (NULL);
		}

		inode_put(dinode);
		return (i);
	}

	dev = dinode->dev;
	inode_put(dinode);

	i = inode_get(dev, num);

	/* Failed to get inode. */
	if (i == NULL)
		return (NULL);

	/* Not allowed. */
	if (!permission(i->mode, i->uid, i->gid, curr_proc, PERM(oflag), 0))
	{
		curr_proc->errno = -EACCES;
		goto error_mkdir;
	}

	/* Directory. */
	if (S_ISDIR(i->mode))
	{
		/* Directories are not writable. */
		if (ACCMODE(oflag) != O_RDONLY)
		{
			curr_proc->errno = -EISDIR;
			goto error_mkdir;
		}
	}

	inode_unlock(i);

	return (i);

error_mkdir:
	inode_put(i);
	return (NULL);
}

/*
 * Opens a file or folder.
 */
PUBLIC int sys_mkdir(const char *path, mode_t mode)
{
		struct inode *i; /* Underlying inode. */
		char *name;		 /* Path name.        */

		if ((name = getname(path)) == NULL)
		{
			return (curr_proc->errno);
		}

		if ((i = createDirectory(path, O_CREAT_D, mode)) == NULL)
		{
			return (curr_proc->errno);
		}
		return i->mode;
}
