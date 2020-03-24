/************* mkdir.c file **************/
extern char gpath[128];
extern int n, dev, ninodes, imap;
extern PROC *running;


int tst_bit(char *buf, int bit) { return buf[bit / 8] & (1 << (bit % 8)); }


int set_bit(char *buf, int bit) { buf[bit / 8] |= (1 << (bit & 8)); }


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

  return 0;
}


int balloc(int dev) {
  int bcount = 0;
  char buf[BLKSIZE];

  bcount = sp->s_blocks_count;

  get_block(dev, gp->bg_block_bitmap, buf);

  for (int i = 0; i < bcount; i++) {
    if (!tst_bit(buf, i)) {
      set_bit(buf, i);
      put_block(dev, gp->bg_block_bitmap, buf);
      return i;
    }
  }
}


int enter_name(MINODE *pip, int myino, char *myname){


}


int mymkdir(MINODE *pip, char *child)
{
  MINODE *mip;
  DIR *dp;
  char buf[BLKSIZE];
  int ino = ialloc(dev);
  int bno = balloc(dev);
  printf("ino: %d\nbno: %d\n", ino, bno);

  mip = iget(dev,ino);
  INODE *ip = &mip->INODE;

  ip->i_mode = 0x41ED;		// OR 040755: DIR type and permissions
  ip->i_uid  = running->uid;	// Owner uid 
  ip->i_gid  = running->gid;	// Group Id
  ip->i_size = BLKSIZE;		// Size in bytes 
  ip->i_links_count = 2;	        // Links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
  ip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks 
  ip->i_block[0] = bno;             // new DIR has one data block
  for(int i = 1; i<15;i++)
  {
    ip->i_block[i] = 0;
  }

  mip->dirty = 1;               // mark minode dirty
  iput(mip);                    // write INODE to disk

  get_block(dev, bno, buf);//Read block from disk
  dp = (DIR*)buf;

  dp->inode = ino;
  strcpy(dp->name, ".");
  dp->name_len = strlen(".");
  dp->rec_len = 12;

  dp = (DIR *)((char *)dp + dp->rec_len);//Move to next entry

  dp->inode = pip->ino;
  strcpy(dp->name, ".");
  dp->name_len = strlen(".");
  dp->rec_len = 1012;//Rest of block

  put_block(dev, bno, buf);//Write block back to disk
  enter_name(pip, ino, child);
  
  return 0;
}

int make_dir(char *path) 
{ 
  char buf[128], parent[128], child[128], temp[128];
  MINODE *pip; 


  strcpy(buf, path);

  if(buf[0] == '/')
    dev = root->dev;
  else
    dev = running->cwd->dev;

  strcpy(temp, buf);
  strcpy(parent, dirname(temp)); // dirname destroys path

  strcpy(temp, buf);
  strcpy(child, basename(temp)); // basename destroys path

  int ino;

  ino = getino(parent);
  pip = iget(dev, ino);

  if(!S_ISDIR(pip->INODE.i_mode))
  {
    printf("ERROR - Filepath does not point to a directory\n");
    return -1;
  }

  if(getino(path)){
    printf("ERROR - Directory already exists\n");
    return -2;
  }

  mymkdir(pip, child);

  pip->INODE.i_links_count++;
  pip->INODE.i_atime = time(NULL);
  pip->dirty = 1;

  iput(pip);

  return 0;
}