/************* symlink.c file **************/

extern char gpath[128];
extern int n, dev, ninodes, nblocks, imap, bmap;
extern PROC *running;


int my_symlink(char *old_file, char *new_file)
{

    char buf[128], parent[128], temp[128];

    int pino, ino, nino; 
    MINODE *pmip, *mip, *new_mip;
	INODE *pip, *ip, *new_ip;

    ino = getino(old_file);
    mip = iget(dev, ino);

    ip = &mip->INODE;

    if(!S_ISDIR(ip->i_mode) && !S_ISREG(ip->i_mode))
    {
        printf("ERROR -> old file provided must be a file or directory\n");
        return -1;
    }

    if(getino(new_file) > 0)
    {
        printf("ERROR -> new file already exists\n");
        return -2;
    }

    strcpy(buf, new_file);
    strcpy(parent, dirname(buf)); // dirname destroys path

    pino = getino(parent);
    pmip = iget(dev, pino);
    pip = &pmip->INODE;

    // creat new file for symlink
    ino = my_creat(pmip, new_file);

    nino = getino(new_file);
    new_mip = iget(dev, nino);
    new_ip = &new_mip->INODE;

    // store filename in memory
    char *blocks = (char *)new_ip->i_block;
    memcpy(blocks, old_file, sizeof(old_file));

    // update new symlink inode file type and size
    new_ip->i_mode = 0120000;
    new_ip->i_size = strlen(old_file);
    
    pmip->dirty = 1;
    iput(pmip);
    iput(mip);

    new_mip->dirty = 1;
    iput(new_mip);

    return 0;
}

int my_readlink(char *pathname, char buf[])
{
    MINODE *mip;
    int ino;

    ino = getino(pathname);
    mip = iget(dev, ino);

    char *blocks = (char*)mip->INODE.i_block;
    memcpy(buf, blocks, sizeof(blocks));

    printf("Reading symlink: %s\n", blocks);
    return 0;
}