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
#include <ctype.h>

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
#include "open_close.c"
#include "read_cat.c"
#include "write_cp.c"
#include "mv.c"
#include "verify.c"
#include "mount_umount.c"

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
    p->status = READY;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;
    p->next = &proc[i+1];
  }
  running = proc[NPROC - 1].next = &proc[0];

  for(i = 0; i<MT_SIZE; i++)
    mtable[i].dev = 0;

  for (i=0; i<NOFT; i++){
    o = &oft[i];
    o->refCount = 0;
    o->offset = 0;
    o->mptr = NULL;
    o->mode = 0;
  }
}

// load root INODE and set root pointer to it
int mount_root(char * rootdev)
{
  char buf[BLKSIZE];
  printf("mount_root()\n");
  printf("checking EXT2 FS ....");
  if ((fd = open(rootdev, O_RDWR)) < 0){
    printf("open %s failed\n", rootdev);
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

  mp = &mtable[0];
  mp->dev = dev;
  ninodes = mp->ninodes = sp->s_inodes_count;
  nblocks = mp->nblocks = sp->s_blocks_count;
  strcpy(mp->devName, rootdev);
  strcpy(mp->mntName, "/");
  get_block(dev, 2, buf); 
  gp = (GD *)buf;
  
  bmap = mp->bmap = gp->bg_block_bitmap;
  imap = mp->imap = gp->bg_inode_bitmap;
  inode_start = mp->iblock = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);
  root = iget(dev, 2);
  mp->mntDirPtr = root;
  root->mptr = mp;
  for(int i = 0; i<NPROC;i++)
    proc[i].cwd = iget(dev, 2);
  root->INODE.i_mode |= 0777;
}

int quit()
{
  int i;
  MINODE *mip;
  if(running->uid != SUPER_USER)
  {
    printf(ERROR"ERROR -> Only root can do that\n"RESET);
    return -1;
  }
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}

void showCommands()
{
   int perRow = 7;
   putchar('\n');
   for(int i = 0; (i<perRow)?1:strcmp(commands[i - perRow], ""); i++)
   {
      if(i>0 && i%perRow == 0){
         putchar('\n');
      }
      if(i < perRow)
         printf(ULINE GRN"   "BOLD BLU"        "RESET"  ");
      else
         printf(ULINE GRN"[>|"BOLD BLU"%-7s]"RESET"  " , commands[i-perRow]);
   }
   printf("\n\n");
}


int main(int argc, char *argv[ ])
{
  char line[128], cmd[32], pathname[128], pathname_2[128];

  char *disk = (argc == 2)?argv[1]:"diskimage";

  init();  
  mount_root(disk);
  printf("root refCount = %d\n", root->refCount);

  while(1){
    showCommands();
    printf(BOLD GRN"input command : "RESET);
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
    else if (strcmp(cmd, "cat")==0) 
       my_cat(pathname);
    else if (strcmp(cmd, "open")==0) 
       open_file(pathname, atoi(pathname_2));
    else if (strcmp(cmd, "close")==0) 
       close_file(atoi(pathname));
    else if (strcmp(cmd, "read")==0)
       read_file(atoi(pathname), atoi(pathname_2));
    else if (strcmp(cmd, "write")==0) 
       write_file(atoi(pathname), pathname_2);
    else if (strcmp(cmd, "verify")==0) 
       verify_blocks(pathname);
    else if (strcmp(cmd, "mount")==0) 
       mount(pathname, pathname_2);
   else if (strcmp(cmd, "sw")==0) 
       my_sw();
   else if (strcmp(cmd, "umount")==0)
      umount(pathname);
    else
       printf(ERROR"Invalid Command!\n"RESET);
  }
}

