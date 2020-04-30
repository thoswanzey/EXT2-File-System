/*********** umount.c file ****************/
extern MINODE minode[NMINODE];
extern MTABLE mtable[MT_SIZE];

int umount(char *pathname)
{
    int i, j, mounted_dev = 0;
    MINODE *mip;
    
    if(running->uid != SUPER_USER)
    {
        printf(ERROR"ERROR -> Only root can do that\n"RESET);
        return -1;
    }

    //gets minode pointer without inc refcount
    MINODE *MPmip = iget_mp(pathname);
    if(!MPmip)
    {
        printf(ERROR"ERROR -> unable to find mount point\n"RESET);
        return -2;
    }

    // search entries to find provided mount point
    for(i = 0; i < MT_SIZE; i++)
    {
        // if entry is found, record mount point device and minode
        if(mtable[i].mntDirPtr == MPmip && mtable[i].mntDirPtr->mounted)
        {
            mounted_dev = mtable[i].dev;
            mip = mtable[i].mntDirPtr;

            printf(YEL"Mounted filesystem: %s found, dev: %d\n"RESET, pathname, mounted_dev);
            break;
        }
    }

    if(i == MT_SIZE)
    {
        printf(ERROR"ERROR -> Mounted filesystem: %s not found!\n"RESET, pathname);
        return -3;
    }

    // check list of minodes to see if mount point is currently busy or in use
    for(j = 0; j < NMINODE; j++)
    {
        if(minode[j].dev == mounted_dev && minode[j].refCount > 0)
        {
            printf(ERROR"ERROR -> Mounted filesystem: %s is busy\n"RESET, pathname);
            return -4;
        }
    }

    close(mtable[i].dev);

    // reset mtable entry and put back minode
    mtable[i].dev = 0;
    mip->mounted = 0;
    dev = mip->dev;

    iput(mip);

    return 0;
}