/************* cat.c file **************/

int my_cat(char *path)
{
    char mybuf[BLKSIZE];
    int n, fd;

    fd = open_file(path, MODE_R);
    if(fd < 0)
    {
        prtinf(ERROR"ERROR -> Unable to open file\n"RESET);
        return -1;
    }

    while(n = my_read(fd, mybuf, BLKSIZE)){
        mybuf[n] = 0;
        for(int i = 0; i<n; i++)
        {
            write(1, mybuf, n);
        }
    }

    close_file(fd);
}
