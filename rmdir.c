/************* mkdir.c file **************/

void clr_bit(char *buf, int bit){buf[bit/8] &= ~(1 << (bit % 8));}

int idalloc(int dev, int ino)  // deallocate an ino number
{ 
  char buf[BLKSIZE];

  if (ino > ninodes || ino < 0){
    printf("inumber %d out of range\n", ino);
    return 0;
  }

  // get inode bitmap block
  get_block(dev, imap, buf);
  clr_bit(buf, ino-1);

  // write buf back
  put_block(dev, imap, buf);
}

int bdalloc(int dev, int blk) // deallocate a blk number
{
    char buf[BLKSIZE];

    if(blk > ninodes || blk < 0){
        printf("bnumber %d out of range\n", blk);
        return 0;
    }

    get_block(dev, bmap, buf);
    clr_bit(buf, blk);
    
    put_block(dev, bmap, buf);
}


