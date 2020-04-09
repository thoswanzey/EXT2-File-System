/************* touch.c file **************/
extern int dev;


int my_touch(char * path)
{
    int ino = getino(path);

    if(ino < 1){//File doesn't exist, so make it
        printf("File doesn't exist...Creating File...\n");
        create_file(path);
        return 0;
    }

    MINODE * mip = iget(dev, ino);

    mip->INODE.i_mtime = mip->INODE.i_atime = time(NULL);
    mip->dirty = 1;
    iput(mip);
    return 0;
}