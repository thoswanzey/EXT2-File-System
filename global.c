/*************** global.c file ************************/
MINODE minode[NMINODE];
MINODE *root;
SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;   
MTABLE *mp;

OFT    oft[NOFT];
PROC   proc[NPROC], *running;

MTABLE mtable[MT_SIZE];

char gpath[128]; // global for tokenized components
char *name[32];  // assume at most 32 components in pathname
int   n;         // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start; // disk parameters

char * commands[] = {"ls",
                     "cd",
                     "pwd",
                     "quit",
                     "mkdir",
                     "creat",
                     "rmdir",
                     "link",
                     "unlink",
                     "symlink",
                     "touch",
                     "stat",
                     "chmod",
                     "pfd",
                     "cp",
                     "mv",
                     "cat",
                     "open",
                     "close",
                     "read",
                     "write",
                     "verify",
                     "mount",
                     "umount",
                     "sw",
                     ""};