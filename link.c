/************* link.c file **************/

extern char gpath[128];
extern int n, dev, ninodes, nblocks, imap, bmap;
extern PROC *running;

int my_link(char *old_file, char *new_file)
{
    char newbuf[128], parent[128], child[128], temp[128];

    int pino, oino; 
    MINODE *pmip, *omip;

    oino = getino(old_file);
    omip = iget(dev, oino);

    if(S_ISDIR(omip->INODE.i_mode))
    {
        printf("ERROR: File provided is a directory.\n");
    }

    if(getino(new_file) != 0)
    {
        printf("ERROR: New link must not exist.\n");
    }

    strcpy(newbuf, new_file);

    strcpy(temp, newbuf);
    strcpy(parent, dirname(temp)); // dirname destroys path

    strcpy(temp, newbuf);
    strcpy(child, basename(temp)); // basename destroys path

    pino = getino(parent);
    pmip = iget(dev, pino);

    enter_name(pmip, oino, child); // add hard link entry

    omip->INODE.i_links_count++; // increment link count

    omip->dirty = 1;
    
    // release used minoids
    iput(omip); 
    iput(pmip);

}

