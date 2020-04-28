/************* cd_ls_pwd.c file **************/

extern int dev;
extern MINODE *root;
extern PROC *running;

int ch_dir(char *pathname)   
{
  int ino = getino(pathname);
  if(ino == 0){
	printf(ERROR"ERROR -> Directory does not exist\n"RESET);
  	return -1;
  }
  
  MINODE *mip = iget(dev, ino);
  if(!S_ISDIR(mip->INODE.i_mode)){
	printf(ERROR"ERROR -> not a directory!\n"RESET);
	iput(mip);
  	return -1;
  }
  iput(running->cwd);
  running->cwd = mip;
}

void print_info(MINODE *mip, char *name)
{
	int i;
	INODE *ip = &mip->INODE;

	char *permission = "rwxrwxrwx";

  u16 uid    = ip->i_uid;           // owner uid
  u16 gid    = ip->i_gid;           // group id
  u32 size   = ip->i_size;          // size in bytes
  u16 mode   = ip->i_mode;          // DIR type, permissions
  u16 links  = ip->i_links_count;   // links count
	int ino = mip->ino;

  switch(mode&0xF000)
	{
    case 0x4000:  putchar('d');     break;
    case 0xA000:  putchar('l');     break; 
		case 0x8000:  putchar('-');     break;
    default:      putchar('?');     break;
	}

  char *time_str = ctime((time_t*)&ip->i_mtime);

	time_str[strlen(time_str) - 1] = 0; // remove carriage return

	for(i = 0; i < strlen(permission); i++)
  {
    putchar(mode & (1 << (strlen(permission) - 1 - i)) ? permission[i] : '-');
  }

	printf("%7hu %4hu %4d %4hu %8u %26s  %s", links, gid, uid, ino, size, time_str, name);

	S_ISLNK(mode)?printf(" -> %s\n", (char *)ip->i_block):putchar('\n');



}

void print_directory(MINODE *mip)
{
	int i;
	char *cp;
	char buf[1024], temp_str[1024];

  DIR *dp;
	INODE *ip = &mip->INODE;
	MINODE *temp_mip;

	printf(BOLD BLU"\n  MODE      LINKS  GID  UID  INO     SIZE          MODIFIED           NAME\n"RESET);

	for(i = 0; i < ip->i_size/1024; i++)
	{
		if(ip->i_block[i] == 0)
			break;

		get_block(dev, ip->i_block[i], buf);
		dp = (DIR*)buf;
		cp = buf;

		while(cp < buf + BLKSIZE)
		{
			strncpy(temp_str, dp->name, dp->name_len);
			temp_str[dp->name_len] = 0;

			temp_mip = iget(dev, dp->inode);
			if(temp_mip)
			{
				print_info(temp_mip, temp_str);
				iput(temp_mip);
			}
			else
				printf(ERROR"MINODE ERROR: cannot print info\n"RESET);

			memset(temp_str, 0, 1024);
			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
	}

	printf("\n");
}

void ls(char *pathname)
{
	int ino, offset;
	MINODE *mip = running->cwd;
	char name[64][64], temp[64];
	char buf[1024];


	if(!strcmp(pathname, "/")) // root
	{
		print_directory(root);
		return;
	}
	else if(!strcmp(pathname, "")) // cwd
	{
		print_directory(mip);
		return;
	}
	else if(pathname)
	{
		if(pathname[0] == '/')
		{
			mip = root;
		}

		ino = getino(pathname);
		if(ino == 0)
		{
			return;
		}

		mip = iget(dev, ino);
		if(!S_ISDIR(mip->INODE.i_mode))
		{
			printf(ERROR"ERROR -> %s not a directory\n"RESET, pathname);
			iput(mip);
			return;
		}

		print_directory(mip);
		iput(mip);
	}
	else // is a directory
	{
		print_directory(root);
	}
}

char *pwd(MINODE *wd)
{
  char dirname[256];

  if (wd == root){
    printf("/");
    return 0;
  }

  int parent_ino, my_ino;
  my_ino = wd->INODE.i_block[0];

  parent_ino = findino(wd, &my_ino);

  MINODE * pip = iget(dev, parent_ino);
  findmyname(pip, my_ino, dirname);
  pwd(pip);
  printf("%s/", dirname);
}