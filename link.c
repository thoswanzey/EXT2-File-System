/************* link.c file **************/

extern char gpath[128];
extern int n, dev, ninodes, nblocks, imap, bmap;
extern PROC *running;

int my_link(char *old_file, char *new_file)
{
    char newbuf[128], parent[128], child[128], temp[128];

    int pino, ino; 
    MINODE *pmip, *mip;
	INODE *pip, *ip;

    ino = getino(old_file);
    mip = iget(dev, ino);

    ip = &mip->INODE;
    pip = &mip->INODE;

    if(S_ISDIR(ip->i_mode))
    {
        printf("ERROR: File provided is a directory.\n");

        return -1;
    }

    if(getino(new_file) != 0)
    {
        printf("ERROR: New link must not exist.\n");

        return -2;
    }

    strcpy(newbuf, new_file);

    strcpy(temp, newbuf);
    strcpy(parent, dirname(temp)); // dirname destroys path

    strcpy(temp, newbuf);
    strcpy(child, basename(temp)); // basename destroys path

    pino = getino(parent);
    pmip = iget(dev, pino);

    enter_name(pmip, ino, child); // add hard link entry

    ip->i_links_count++; // increment link count
    printf("INODE: %d, links_count: %d\n", mip->ino, ip->i_links_count);

    mip->dirty = 1;

    pip->i_atime = time(NULL);
    // release used minoids
    iput(pmip); 
    iput(mip);

    return 0;
}