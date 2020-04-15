/*************** global.c file ************************/
MINODE minode[NMINODE];
MINODE *root;

OFT    oft[NOFT];
PROC   proc[NPROC], *running;

char gpath[128]; // global for tokenized components
char *name[32];  // assume at most 32 components in pathname
int   n;         // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start; // disk parameters
