/*********** mount.c file ****************/

int mount_init()
{   
    // keeps track of how many file systems are mounted
    mounted_count = 0;

    // set all file systems to free
    for(int i = 0; i < MT_SIZE; i++)
    {
        mtable[i].dev = 0;
    }
}

int mount(char *pathname, char *mp)
{

    int ino, fd;
    MINODE *mip;
    char buf[BLKSIZE];
    SUPER *sp_temp;

    // if mount point is null, print current mounted file systems
    if(mp[0] == 0)
    {
        for(int i = 0; i < MT_SIZE; i++)
        {
            if(mtable[i].dev != 0)
                printf(ERROR"MOUNTED FS: %s, device: %d\n"RESET, mtable[i].devName, mtable[i].dev);
        }
    }

    // check to see if mounted FS already exists
    for(int i = 0; i < MT_SIZE; i++)
    {
        if(mtable[i].dev > 0 && !strcmp(mtable[i].devName, pathname))
        {
            printf(ERROR"MOUNTED FS: %s, already exists!\n"RESET, pathname);
            return -1;
        }
    }
    
    // check to make sure there is room in the mount table
    if(mounted_count >= MT_SIZE)
    {
        printf(ERROR"MOUNT Table is full!\n"RESET);
        return -2;
    }

    // allocate free mount table entry
    mtable[mounted_count].dev = 0;
 
    // get fd of virtual disk
    fd = open(pathname, O_RDWR);

    // checking to see if valid EXT2 FS
    get_block(fd, 1, buf);

    sp_temp = (SUPER *)buf;

    if (sp_temp->s_magic != 0xEF53)
    {
        printf(ERROR"magic = %x is not an ext2 filesystem\n"RESET, sp_temp->s_magic);
        return -3;
    }

    // find ino, get minode of mount point     
    ino = getino(mp);
    mip = iget(dev, ino);

    // check to see if mount point is DIR and not busy
    if(!S_ISDIR(mip->INODE.i_mode) || running->cwd == mip){
        printf(ERROR"provied mount point is not a directory or it is busy\n"RESET);
        iput(mip);
        return -4;
    }

    // store new mount table entry data
    mtable[mounted_count].dev = fd;
    strcpy(mtable[mounted_count].devName, pathname);
    strcpy(mtable[mounted_count].mntName, mp);

    // mark mount point as mounted
    mip->mounted = 1;
    mip->mptr = &mtable[mounted_count];
    mtable[mounted_count].mntDirPtr = mip;
    
    mounted_count++;

    return 0;
}