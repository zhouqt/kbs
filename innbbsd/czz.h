static struct flock fl = {
        l_whence:SEEK_SET,
        l_start:0,
        l_len:0,
};

int
f_exlock(fd)
int     fd;
{
        fl.l_type = F_WRLCK;
        return fcntl(fd, F_SETLKW, &fl);
}

int
f_unlock(fd)
int     fd;
{
        fl.l_type = F_UNLCK;
        return fcntl(fd, F_SETLKW, &fl);
}
