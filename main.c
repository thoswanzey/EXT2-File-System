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
#include <errno.h>

#include "type.h"
#include "global.c"
#include "util.c"
#include "alloc.c"
#include "cd_ls_pwd.c"
#include "mkdir_creat.c"
#include "rmdir.c"
#include "link_unlink.c"
#include "symlink.c"
#include "touch.c"
#include "stat.c"
#include "chmod.c"
#include "open.c"
#include "close.c"
#include "lseek.c"
#include "pfd.c"
#include "read.c"
#include "cat.c"
#include "write.c"
#include "cp.c"
#include "mv.c"

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;
  OFT    *o;
  printf("init()\n");

  for (i=0; i<NMINODE; i++){
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
  for (i=0; i<NOFT; i++){
    o = &oft[i];
    o->refCount = 0;
    o->offset = 0;
    o->mptr = NULL;
    o->mode = 0;
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
  char line[128], cmd[32], pathname[128], pathname_2[128];
 
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
   write_file(open_file("test", 2), "hi123456789");

  while(1){
    printf(GRN"\n[ls|cd|pwd|quit|mkdir|rmdir|create|link|unlink|symlink|touch|stat|chmod]\n" BOLD "input command : "RESET);
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

    if (line[0]==0)
       continue;
    pathname[0] = 0;
    pathname_2[0] = 0;
   


    sscanf(line, "%s %s %s", cmd, pathname, pathname_2);
    printf("cmd=%s pathname=%s pathname2=%s\n", cmd, pathname, pathname_2);
  
    if (strcmp(cmd, "ls")==0)
       ls(pathname);
    else if (strcmp(cmd, "cd")==0)
       ch_dir(pathname);
    else if (strcmp(cmd, "pwd")==0)
       pwd(running->cwd);
    else if (strcmp(cmd, "quit")==0)
       quit();
    else if (strcmp(cmd, "mkdir")==0)
       make_dir(pathname);
    else if (strcmp(cmd, "creat")==0)
       create_file(pathname);
    else if (strcmp(cmd, "rmdir")==0)
       my_rmdir(pathname);
    else if (strcmp(cmd, "link")==0)
       my_link(pathname, pathname_2);
    else if (strcmp(cmd, "unlink")==0) 
       my_unlink(pathname);
    else if (strcmp(cmd, "symlink")==0) 
       my_symlink(pathname, pathname_2);
    else if (strcmp(cmd, "symlink")==0) 
       my_symlink(pathname, pathname_2);
    else if (strcmp(cmd, "touch")==0) 
       my_touch(pathname);
    else if (strcmp(cmd, "stat")==0) 
       my_stat(pathname);
    else if (strcmp(cmd, "chmod")==0) 
       my_chmod(pathname, pathname_2);
    else if (strcmp(cmd, "pfd")==0) 
       my_pfd();
    else if (strcmp(cmd, "cp")==0) 
       my_cp(pathname, pathname_2);
    else if (strcmp(cmd, "mv")==0) 
       my_mv(pathname, pathname_2);
    else
       printf("Invalid Command!\n");
  }
}

