/************* lseek.c file **************/


int lseek(int fd, int position)
{
    int initial_offset;
    OFT *oftp = running->fd[fd];

    if(!oftp){
        printf(ERROR"ERROR -> fd does not exist\n"RESET);
        return -1;
    }

    if(position > oftp->mptr->INODE.i_size || position < 0){
        printf(ERROR"ERROR -> position is not within file bounds\n"RESET);
        return -2;
    }

    initial_offset = oftp->offset;
    oftp->offset = position;
    return initial_offset;
}