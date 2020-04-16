/************* mv.c file **************/

int my_mv(char *src, char*dest)
{
    char destination[256], temp[256], sourceFileName[64];
    int src_ino, dest_ino, src_dev, dest_dev;

    if(!strcmp(src, "") || !strcmp(dest, "")){
        printf(ERROR"ERROR -> Either the source or destination file was not specified\n"RESET);
        return -1;
    }

    int ino = getino(src);
    if(!ino){
        printf(ERROR"ERROR -> Source file does not exist\n"RESET);
        return -2;
    }

    if(src[0] == '/') src_dev = root->dev;
    else src_dev = running->cwd->dev;

    if(dest[0] == '/') dest_dev = root->dev;
    else dest_dev = running->cwd->dev;

    //If inode exists, check if directory
    if(ino = getino(dest))
    {
        //Get dest in minode so we can check if it's a directory
        MINODE *mip = iget(dev, ino);
        //If a directory, add src file name to destination path
        if(S_ISDIR(mip->INODE.i_mode))
        {
            strcpy(destination, dest);
            strcat(destination, "/");
            strcat(destination, sourceFileName);
        }
        iput(mip);
    }

    int success;
    if(dest_dev == src_dev){
        if(my_link(src, destination) < 0){
            return -3;
        }
    }

    else{
        if(my_cp(src, destination) < 0){
            return -4;
        }
    }

    //Function will Return negative value if unsuccessful
    return my_unlink(src);
}

