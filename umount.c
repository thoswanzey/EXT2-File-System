/*********** umount.c file ****************/
extern MINODE minode[NMINODE];
extern MTABLE mtable[MT_SIZE];

int umount(char *pathname)
{
    int i, j, mounted_dev = 0;
    MINODE *mip;
    
    // search entries to find provided mount point
    for(i = 0; i < MT_SIZE; i++)
    {
        // if entry is found, record mount point device and minode
        if(mtable[i].mntDirPtr->mounted && strcmp(pathname, mtable[i].mntName) == 0)
        {
            mounted_dev = mtable[i].dev;
            mip = mtable[i].mntDirPtr;

            printf("Mounted filesystem: %s found, dev: %d\n", pathname, mip->dev);
            break;
        }
    }

    if(mounted_dev == 0)
    {
        printf("Mounted filesystem: %s not found!\n", pathname);
        return -1;
    }

    // check list of minodes to see if mount point is currently busy or in use
    for(j = 0; j < NMINODE; j++)
    {
        if(minode[j].dev == mounted_dev && minode[j].dev > 0)
        {
            printf("Mounted filesystem: %s is busy\n", pathname);
            return -2;
        }
    }

    // reset mtable entry and put back minode
    mtable[i].dev = 0;
    mip->mounted = 0;
    iput(mip);
    
    return 0;
}