/************* cp.c file **************/

int my_cp(char *src, char*dest)
{
    char destination[256], sourceFileName[256];

    strcpy(destination, dest);
    strcpy(sourceFileName, src);
    strcpy(sourceFileName, basename(sourceFileName));

    if(!strcmp(src, "") || !strcmp(dest, "")){
        printf(ERROR"ERROR -> Either the source or destination file was not specified\n"RESET);
        return -1;
    }

    int ino = getino(src);
    if(!ino){
        printf(ERROR"ERROR -> Source file does not exist\n"RESET);
        return -2;
    }

    if(dest[0] == '/') dev = root->dev;
    else dev = running->cwd->dev;

    //If inode exists, check if directory, otherwise file will be created in file_open()
    if(ino = getino(dest))
    {
        //Get dest in minode so we can check if it's a directory
        MINODE *mip = iget(dev, ino);

        //If a directory, add src file name to destination path
        if(S_ISDIR(mip->INODE.i_mode))
        {
            strcat(destination, "/");
            strcat(destination, sourceFileName);
        }
        iput(mip);
    }

    int fd, gd, n;
    char buf[BLKSIZE];
    fd = open_file(src, MODE_R);
    if(fd < 0){
        //open_file will print the error
        return -3;
    }

    //If destination file doesn't exist already, open_file will create it (for all write modes)
    gd = open_file(destination, MODE_RW);

    while(n = my_read(fd, buf, BLKSIZE))
    {
        my_write(gd, buf, n);
    }

    close_file(fd);
    close_file(gd);
    return 0;
}
