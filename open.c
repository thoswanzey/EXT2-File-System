/************* open.c file **************/


#define MODE_R          0
#define MODE_W          1
#define MODE_RW         2
#define MODE_APPEND     3

#define PERMISSIONS_USER    0700
#define PERMISSIONS_GROUP   070
#define PERMISSIONS_OTHER   07

#define PERMISSIONS_R   0444
#define PERMISSIONS_W   0222
#define PERMISSIONS_X   0111

int open_file(char *path, int mode)
{
    if(mode > 3 || mode < 0)
    {
        printf(ERROR"ERROR -> Invalid mode\n"RESET);
        return -1;
    }

    if(path[0] == '/') dev = root->dev;
    else dev = running->cwd->dev;

    int ino = getino(path);

    //If file doesn't exist and are opening in write mode, create it
    if(ino == 0)
    {   
        if(mode == MODE_R)
        {
            printf(ERROR"ERROR -> File doesn't exist\n"RESET);
            return -2;
        }
        create_file(path);
        ino = getino(path);
        if(!ino)
        {
            printf(ERROR"ERROR -> File doesn't exist and could not be created\n"RESET);
            return -3;
        }
    }

    MINODE *mip = iget(dev, ino);

    if(!S_ISREG(mip->INODE.i_mode))
    {
        printf(ERROR"ERROR -> Not a regular file\n"RESET);
        iput(mip);
        return -3;
    }

    int permissionBits = PERMISSIONS_OTHER;
    if(mip->INODE.i_gid == running->gid) permissionBits |= PERMISSIONS_GROUP;
    if(mip->INODE.i_uid == running->uid) permissionBits |= PERMISSIONS_USER;

    //Check for read permission if necessary
    if(mode == MODE_R || mode == MODE_RW)
    {
        if(!(permissionBits & PERMISSIONS_R & mip->INODE.i_mode) && mip->INODE.i_uid != SUPER_USER)
        {
            printf(ERROR"ERROR -> You do not have permission to do this\n"RESET);
            iput(mip);
            return -4;
        }
    }

    //Check for write permission if necessary
    if(mode == MODE_W || mode == MODE_APPEND || mode == MODE_RW)
    {
        if(!(permissionBits & PERMISSIONS_W & mip->INODE.i_mode) && mip->INODE.i_uid != SUPER_USER)
        {
            printf(ERROR"ERROR -> You do not have permission to do this\n"RESET);
            iput(mip);
            return -5;
        }
    }


    OFT * findOFT = NULL;
    for(int i = 0; i < NOFT; i++)
    {
        if(oft[i].refCount == 0 && !findOFT)   findOFT = &oft[i];
        else if(oft[i].mptr == mip)
        {
            //If already open and not in read mode
            if(oft[i].mode != MODE_R){
                printf(ERROR"ERROR -> File is currently being accessed\n"RESET);
                iput(mip);
                return -6;
            }
            //If already open in read mode, but trying to open in a different mode
            else if(mode != MODE_R){
                printf(ERROR"ERROR -> File is currently being accessed\n"RESET);
                iput(mip);
                return -7;
            }
            //Already open in read mode, and trying to open in Read mode, so OK
            else{
                findOFT = &oft[i];
                findOFT->refCount++;
                break;
            }
        }
    }

    if(!findOFT)
    {
        printf(ERROR"ERROR -> No available OFT found\n"RESET);
        iput(mip);
        return -8; 
    }

    //This means we didn't find existing OFT, so initialize it
    if(findOFT->refCount == 0)
    {
        findOFT->refCount = 1;
        findOFT->offset = (mode == MODE_APPEND) ? mip->INODE.i_size : 0;
        findOFT->mode = mode;
        findOFT->mptr = mip;
    }

    //Find first open FD
    int j;
    for(j = 0; j<NFD; j++)
    {
        if(!running->fd[j])
        {
            running->fd[j] = findOFT;
            break;
        }
    }

    //If nowhere to put, show error, decrement ref count, and release minode
    if(j == NFD){
        printf(ERROR"ERROR -> No available FD's found\n"RESET);
        findOFT->refCount--;
        iput(mip);
        return -9;
    }

    //update access time
    mip->INODE.i_atime = time(NULL);

    //update modified time
    if(mode != MODE_R)  mip->INODE.i_mtime = mip->INODE.i_atime;

    mip->dirty = 1;
    iput(mip);
    //return FD
    return j;
}