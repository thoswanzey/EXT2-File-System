/************* link.c file **************/

extern char gpath[128];
extern int n, dev, ninodes, nblocks, imap, bmap;
extern PROC *running;

int my_unlink(char *pathname)
{
    char child[128], parent[128], buf[128];

    int ino, pino;
    MINODE *mip, *pmip;
	INODE *pip, *ip;

    ino = getino(pathname);
    mip = iget(dev, ino);

    ip = &mip->INODE;
    pip = &mip->INODE;

    if(S_ISDIR(ip->i_mode))
    {
        printf("ERROR: File provided is a directory.\n");

        return -1;
    }

    strcpy(buf, pathname);
    strcpy(parent, dirname(buf)); // dirname destroys path

    strcpy(buf, pathname);
    strcpy(child, basename(buf)); // basename destroys path

    pino = getino(parent);
    pmip = iget(dev, pino);

    rm_child(pmip, child); // remove entry from dir

    pip->i_atime = pip->i_mtime = time(NULL);
    pmip->dirty = 1;
    iput(pmip);


    ip->i_links_count--;
    printf("INODE: %d, links_count: %d\n", mip->ino, ip->i_links_count);

    if(ip->i_links_count > 0)
    {
        mip->dirty = 1;
    } 
    else
    {
        for (int i=0; i<12; i++){ // delete inode
            if (ip->i_block[i]==0)
                break;
            bdalloc(mip->dev, ip->i_block[i]);
        }
        idalloc(mip->dev, mip->ino);
    }
    
    
    iput(mip); 

    return 0;    
}