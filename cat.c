/************* cat.c file **************/

int my_cat(char *path)
{
    char mybuf[BLKSIZE];
    int n, fd;

    fd = open_file(path, MODE_R);
    if(fd < 0)
    {
        printf(ERROR"ERROR -> Unable to open file\n"RESET);
        return -1;
    }

    while(n = my_read(fd, mybuf, BLKSIZE)){
        write(1, mybuf, n);
        bzero(mybuf, BLKSIZE); // clear buffer, otherwise stack overflow
    }

    close_file(fd);
}
