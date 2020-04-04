/************* mkdir.c file **************/
extern char gpath[128];
extern int n, dev, ninodes, nblocks, imap, bmap;
extern PROC *running;


int tst_bit(char *buf, int bit) { return buf[bit / 8] & (1 << (bit % 8)); }


int set_bit(char *buf, int bit) { buf[bit / 8] |= (1 << (bit % 8)); }


int ialloc(int dev) {
  int i;
  char buf[BLKSIZE];

  get_block(dev, imap, buf);

  for (i = 0; i < ninodes; i++) {
    if (tst_bit(buf, i) == 0) {
      set_bit(buf, i);
      put_block(dev, imap, buf);
      printf("allocated ino = %d\n", i + 1);
      return i + 1;
    }
  }
  printf("ERROR - inode was not allocated because no free inodes remain\n");
  return 0;
}


int balloc(int dev) {
  char buf[BLKSIZE];

  get_block(dev, bmap, buf);

  for (int i = 0; i < nblocks; i++) {
    if (!tst_bit(buf, i)) {
      set_bit(buf, i);
      put_block(dev, bmap, buf);
      printf("allocated bno = %d\n", i);
      return i;
    }
  }
  printf("ERROR - Block was not allocated because no free blocks remain\n");
  return -1;
}

int idealloc(int ino)
{
    INODE *ip;
    char buf[BLKSIZE];
    int byte, bit;

    byte = ino / 8;
    bit = ino % 8;
    
    get_block(dev, bmap, buf);

    buf[byte] &= ~(1 << bit);

    put_block(dev, bmap, buf);

    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count++;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_blocks_count++;
    put_block(dev, 2, buf);

    return 0;
}

int my_truncate(MINODE *mip)
{
    INODE *ip;
    char buf[BLKSIZE];
    int byte, bit, ino;
    
    
    ip = &(mip->INODE);

    for(int i = 0; i < 12; i++)
    {
        ino = ip->i_block[i];

        byte = ino / 8;
        bit = ino % 8;

        get_block(dev, bmap, buf);

        buf[byte] &= ~(1 << bit);

        put_block(dev, bmap, buf);

        get_block(dev, 1, buf);
        sp = (SUPER *)buf;
        sp->s_free_blocks_count++;
        put_block(dev, 1, buf);

        get_block(dev, 2, buf);
        gp = (GD *)buf;
        gp->bg_free_blocks_count++;
        put_block(dev, 2, buf);

        return 0;
    }
}