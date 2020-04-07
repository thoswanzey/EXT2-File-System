/************* creat.c file **************/

extern char gpath[128];
extern int n, dev, ninodes, imap;
extern PROC *running;


int my_creat(MINODE *pmip, char *child)
{
  DIR *dp;
  int ino = ialloc(dev);

  MINODE *mip = iget(dev,ino);
  INODE *ip = &mip->INODE;

  ip->i_mode = 0x81A4;		// OR 0100644: File type and permissions
  ip->i_uid  = running->uid;	// Owner uid 
  ip->i_gid  = running->gid;	// Group Id
  ip->i_size = 0;
  ip->i_links_count = 1;	        
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
  ip->i_blocks = 0;                	// LINUX: Blocks count in 512-byte chunks
  for(int i = 0; i<15;i++)
  {
    ip->i_block[i] = 0;
  }

  mip->dirty = 1;               // mark minode dirty
  iput(mip);                    // write INODE to disk

  enter_name(pmip, ino, child);
  
  return ino;
}

int create_file(char *path) 
{ 
  char buf[128], parent[128], child[128], temp[128];
  MINODE *pip; 


  strcpy(buf, path);

  strcpy(temp, buf);
  strcpy(parent, dirname(temp)); // dirname destroys path

  strcpy(temp, buf);
  strcpy(child, basename(temp)); // basename destroys path

  int ino;

  ino = getino(parent);
  pip = iget(dev, ino);

  if(!pip){
    printf("ERROR - Provided parent directory does exists!\n");
    return -1;
  }

  if(!S_ISDIR(pip->INODE.i_mode))
  {
    printf("ERROR - Provided parent directory is not a directory!\n");
    return -2;
  }

  if(getino(path)){
    printf("ERROR - File already exists!\n");
    return -3;
  }

  my_creat(pip, child);

  pip->INODE.i_atime = time(NULL);
  pip->dirty = 1;

  iput(pip);

  return 0;
}