/*********** mount.c file ****************/

int mount_init()
{   
    // set all file systems to free
    for(int i = 0; i < MT_SIZE; i++)
    {
        mtable[i].dev = 0;
    }
}

int mount(char *pathname, char *mp)
{

    int ino, fd, mounted_count = -1;
    MINODE *mip;
    char buf[BLKSIZE];
    SUPER *sp_temp;

    // if mount point is null, print current mounted file systems
    if(mp[0] == 0 || pathname[0] == 0)
    {
        printf(BOLD BLU"\n    Device Name    | Dev Number | Mount Point\n"RESET);
        for(int i = 0; i < MT_SIZE; i++)
        {
            if(mtable[i].dev != 0)
                printf("%-19s"BOLD BLU"| "RESET"%-11d"BOLD BLU"| "RESET"[%d, %d]\n", mtable[i].devName, mtable[i].dev, mtable[i].mntDirPtr->dev, mtable[i].mntDirPtr->ino);
        }
        putchar('\n');
        return 0;
    }

    if(running->uid != SUPER_USER)
    {
        printf(ERROR"ERROR -> Only root can do that\n"RESET);
        return -1;
    }

    // check to see if mounted FS already exists
    for(int i = 0; i < MT_SIZE; i++)
    {
        if(mtable[i].dev > 0 && !strcmp(mtable[i].devName, pathname))
        {
            printf(ERROR"MOUNTED FS: %s, already exists!\n"RESET, pathname);
            return -2;
        }
        if(mtable[i].dev == 0 && mounted_count == -1)
        {
            mounted_count = i;
        }
    }
    
    // check to make sure there is room in the mount table
    if(mounted_count == -1)
    {
        printf(ERROR"MOUNT Table is full!\n"RESET);
        return -3;
    }

 
    // get fd of virtual disk
    fd = open(pathname, O_RDWR);

    // checking to see if valid EXT2 FS
    get_block(fd, 1, buf);

    sp_temp = (SUPER *)buf;

    if (sp_temp->s_magic != EXT2_SUPER_MAGIC)
    {
        printf(ERROR"magic = %x is not an ext2 filesystem\n"RESET, sp_temp->s_magic);
        return -4;
    }

    // find ino, get minode of mount point     
    ino = getino(mp);
    if (!ino)
    {
        printf(ERROR"ERROR -> Mount point does not exist\n"RESET);
        return -5;
    }

    mip = iget(dev, ino);

    // check to see if mount point is DIR
    if(!S_ISDIR(mip->INODE.i_mode)){
        printf(ERROR"provied mount point is not a directory\n"RESET);
        iput(mip);
        return -6;
    }

    // check minode to see if it's being referenced anywhere else
    if(mip->refCount > 1)
    {
        printf(ERROR"ERROR -> Mount point is busy\n"RESET);
        iput(mip);
        return -7;
    }


    // store new mount table entry data
    mtable[mounted_count].dev = fd;
    strcpy(mtable[mounted_count].devName, pathname);
    strcpy(mtable[mounted_count].mntName, mp);

    // mark mount point as mounted
    mip->mounted = 1;
    mip->mptr = &mtable[mounted_count];
    mtable[mounted_count].mntDirPtr = mip;

    return 0;
}