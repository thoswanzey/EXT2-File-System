/************* open.c file **************/

int close_file(int fd)
{
    if(fd < 0 || fd >= NFD){
        printf(ERROR"ERROR -> Invalid file descriptor\n"RESET);
        return -1;
    }

    if(!running->fd[fd]){
        printf(ERROR"ERROR -> File descriptor does not exist\n"RESET);
        return -2;
    }

    OFT *oftp = running->fd[fd];
    running->fd[fd] = NULL;
    oftp->refCount--;

    //If file is accessed somewhere else, leave it
    if(oftp->refCount > 0) return 0;

    //If file no longer accessed by anything, free minode
    iput(oftp->mptr);
    oft->mptr = NULL;
    oft->offset = 0;
    return 0;
}
