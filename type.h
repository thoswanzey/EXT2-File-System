/*************** type.h file ************************/
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;   

#define FREE        0
#define READY       1

#define MT_SIZE  16
#define BLKSIZE  1024
#define NMINODE   128
#define NFD        16
#define NOFT       40
#define NPROC       2

#define SUPER_USER  0


typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;
  int dirty;
  int mounted;
  struct mtable *mptr;
}MINODE;


typedef struct mtable{
    int dev;
    int ninodes;
    int nblocks;
    int free_blocks;
    int free_inodes;
    int bmap;
    int imap;
    int iblock;
    MINODE *mntDirPtr;
    char devName[64];
    char mntName[64];
}MTABLE;


typedef struct oft{
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;

typedef struct proc{
  struct proc *next;
  int          pid;
  int          status;
  int          uid, gid;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;

#ifndef COLOR_H
#define COLOR_H

    /* FANCY */
    #define RED   "\x1B[31m"
    #define GRN   "\x1B[32m"
    #define YEL   "\x1B[33m"
    #define BLU   "\x1B[34m"
    #define MAG   "\x1B[35m"
    #define CYN   "\x1B[36m"
    #define WHT   "\x1B[37m"
    #define RESET "\x1B[0m"
    #define BOLD  "\033[1m"

    #define ERROR       BOLD RED
    #define WARNING     BOLD YEL

#endif // COLOR_H

/*________________FUNCTIONS________________*/

//**************alloc.c**************
int tst_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
void clr_bit(char *buf, int bit);
int idalloc(int dev, int ino);
int bdalloc(int dev, int blk);
int ialloc(int dev);
int balloc(int dev);

//**************cat.c**************
int my_cat(char *path);

//**************cd_ls_pwd.c**************
int ch_dir(char *pathname);
void print_info(MINODE *mip, char *name);
void print_directory(MINODE *mip);
void ls(char *pathname);

//**************chmod.c**************
int my_chmod(char * path, char * mode);

//**************close.c**************
int close_file(int fd);

//**************cp.c**************
int my_cp(char *src, char*dest);

//**************link_unlink.c**************
int my_link(char *old_file, char *new_file);
int my_unlink(char *pathname);

//**************lseek.c**************
int my_lseek(int fd, int position);

//**************mkdir_creat.c**************
int enter_name(MINODE *pmip, int myino, char *myname);
int my_mkdir(MINODE *pip, char *child);
int make_dir(char *path) ;
int my_creat(MINODE *pmip, char *child);
int create_file(char *path) ;

//**************mount.c**************
int mount_init();
int mount(char *pathname, char *mp);

//**************mv.c**************
int my_mv(char *src, char*dest);

//**************open.c**************
int open_file(char *path, int mode);

//**************pfd.c**************
int my_pfd(void);

//**************read.c**************
int my_read(int fd, char buf[], int nbytes, int verbose);
int read_file(int fd, int nbytes);

//**************rmdir.c**************
int rm_child(MINODE *pip, char *name);
int my_rmdir(char * path);

//**************stat.c**************
int my_stat(char * path);

//**************symlink.c**************
int my_symlink(char *old_file, char *new_file);
int my_readlink(char *pathname, char buf[]);

//**************touch.c**************
int my_touch(char * path);

//**************umount.c**************
int umount(char *pathname);

//**************util.c**************
int get_block(int dev, int blk, char *buf);
int put_block(int dev, int blk, char *buf);
int tokenize(char *pathname);
MINODE *iget(int dev, int ino);
void iput(MINODE *mip);
int search(MINODE *mip, char *name);
int getino(char *pathname);
MINODE * iget_mp(char *pathname);
int findmyname(MINODE *parent, u32 myino, char *myname);
int findino(MINODE *mip, u32 *myino);
int my_truncate(MINODE * mip);
int my_access(char *pathname, char mode);
int my_maccess(MINODE *mip, char mode);
void my_sw(void);


//**************verify.c**************
int verify_blocks(char *path);

//**************write.c**************
int my_write(int fd, char buf[], int nbytes);
int write_file(int fd, char *str);
