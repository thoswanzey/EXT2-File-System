/****************************************************************************
*                   KCW  Implement ext2 file system                         *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "type.h"

// global variables
MINODE minode[NMINODE];
MINODE *root;

PROC   proc[NPROC], *running;

char gpath[128]; // global for tokenized components
char *name[32];  // assume at most 32 components in pathname
int   n;         // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start; // disk parameters
#include "util.c"
#include "cd_ls_pwd.c"

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

int mymkdir(MINODE *pip, char *child)
{
  int ino = ialloc(dev);
  int bno = balloc(dev);
  printf("ino: %d\nbno: %d\n", ino, bno);
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

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){
  }
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->uid = p->gid = 0;
    p->cwd = 0;
    p->status = FREE;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;
  }
}

// load root INODE and set root pointer to it
int mount_root()
{  
  printf("mount_root()\n");
  root = iget(dev, 2);
}

int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}


char *disk = "diskimage";
int main(int argc, char *argv[ ])
{
  int ino;
  char buf[BLKSIZE];
  char line[128], cmd[32], pathname[128];
 
  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);
    exit(1);
  }
  dev = fd;    // fd is the global dev 

  /********** read super block  ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system ***********/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("EXT2 FS OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  inode_start = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

  init();  
  mount_root();
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->status = READY;
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  // WRTIE code here to create P1 as a USER process
  
  while(1){
    printf("\ninput command : [ls|cd|pwd|quit] ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

    if (line[0]==0)
       continue;
    pathname[0] = 0;

    sscanf(line, "%s %s", cmd, pathname);
    printf("cmd=%s pathname=%s\n", cmd, pathname);
  
    if (strcmp(cmd, "ls")==0)
       ls(pathname);
    else if (strcmp(cmd, "cd")==0)
       ch_dir(pathname);
    else if (strcmp(cmd, "pwd")==0)
       pwd(running->cwd);
    else if (strcmp(cmd, "quit")==0)
       quit();
  }
}

