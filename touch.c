extern char gpath[128];
extern int n, dev, ninodes, nblocks, imap, bmap;
extern PROC *running;


int my_touch(char * path){
    int ino = getino(path);

    if(!ino){
        printf("ERROR - file does not exist\n");
        return -1;
    }

    MINODE * mip = iget(dev, ino);

    mip->INODE.i_atime = mip->INODE.i_mtime = time(NULL);
    mip->dirty = 1;
    iput(mip);
    return 0;
}