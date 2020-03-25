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
  char buf[BLKSIZE], *cp;
  DIR *dp;

  INODE *iparent = &pip->INODE; // get parent inode

  int pblk = 0, remain = 0;
  int ideal_length = 0, need_length = 0;

  for(int i = 0; i < (iparent->i_size / BLKSIZE); i++)
  {
    if(iparent->i_block[i]==0)
      break;

    pblk = iparent->i_block[i]; // get current block number

    ideal_length = 4 * ((8 + dp->name_len + 3)/4); // length until next directory entry
    need_length = 4 * ((8 + strlen(myname) + 3)/4); // needed length of new dir name in bytes

    printf("ideal = %d\nneed = %d\n", ideal_length, need_length);

    get_block(dev, pblk, buf); // get current parent inode block

    dp = (DIR*)buf; // cast current inode block as directory pointer
    cp = buf;

    printf("Entering last entry in data block %d\n", pblk);

    while((cp + dp->rec_len) < (buf + BLKSIZE))
    {
      cp += dp->rec_len;
      dp = (DIR *)cp; // dp now points at last entry in block
    }
    
    cp = (char*)dp;

    remain = dp->rec_len - ideal_length; // remaining length
    printf("remain = %d\n", remain);

    if(remain >= need_length)
    {
      dp->rec_len = ideal_length;

      cp += dp->rec_len; // set cp to end of ideal
      dp = (DIR*)cp; // end of last entry

      dp->inode = myino; // set end of entry to provided inode

      dp->rec_len = BLKSIZE - ((u32)cp - (u32)buf);

      dp->name_len = strlen(myname);

      dp->file_type = EXT2_FT_DIR;

      strcpy(dp->name, myname);

      put_block(dev, pblk, buf); // write block back

      return 1;
    }

    printf("Block number = %d\n", i);

    pblk = balloc(dev); // get the first available block for new inode

    iparent->i_block[i] = pblk;

    iparent->i_size += BLKSIZE;
    pip->dirty = 1;

    get_block(dev, pblk, buf);

    cp = (DIR*)buf;
    cp = buf;

    printf("Directory Name = %s\n", dp->name);

    dp->inode = myino;

    dp->rec_len = BLKSIZE;

    dp->name_len = strlen(myname);

    dp->file_type = EXT2_FT_DIR;

    strcpy(dp->name, myname);

    put_block(dev, pblk, buf); // write block

    return 1;
  }
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
  dp->name[0] = '.';
  dp->name_len = strlen(".");
  dp->rec_len = 12;

  dp = (DIR *)((char *)dp + dp->rec_len);//Move to next entry

  dp->inode = pip->ino;
  dp->name[0] = dp->name[1] = '.';
  dp->name_len = strlen("..");
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