/************* stat.c file **************/
extern int dev;

int my_stat(char * path)
{
    char *permission = "rwxrwxrwx", type[20], access[20], temp[128];
    int ino = getino(path);
    if(!ino){
        printf("ERROR - file does not exist\n");
        return -1;
    }

    MINODE * mip = iget(dev, ino);

    if(S_ISREG(mip->INODE.i_mode))
        strcpy(type, "regular file");
    else if(S_ISDIR(mip->INODE.i_mode))
        strcpy(type, "directory file");
    else if(S_ISLNK(mip->INODE.i_mode))
        strcpy(type, "link file");
    else
        strcpy(type, "unknown file type");

    S_ISDIR(mip->INODE.i_mode)? strcpy(access, "0755"): strcpy(access, "0644");
    strcpy(temp, path);
    printf("\nFile: %s\n", basename(temp));
    printf("Size: %-15d Blocks: %-13d %s\n", mip->INODE.i_size, mip->INODE.i_blocks, type);

    printf("Device: %-13d Inode: %-14d Links: %-14d\n", mip->dev, mip->ino, mip->INODE.i_links_count);
    
    printf("Access: (%s/", access);

    for(int i = 0; i < strlen(permission); i++)
    {
        putchar(mip->INODE.i_mode & (1 << (strlen(permission) - 1 - i)) ? permission[i] : '-');
    }

    printf(")  Uid: %-10d Gid: %d\n", mip->INODE.i_uid, mip->INODE.i_gid);
    printf("Access: %d\n",ctime((time_t *)&(mip->INODE.i_atime)));
    printf("Modify: %s\n",ctime((time_t *)&mip->INODE.i_mtime));
    printf("Change: %s\n",ctime((time_t *)&(mip->INODE.i_ctime)));

    iput(mip);
    return 0;
}