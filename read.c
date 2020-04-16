/************* read.c file **************/

extern PROC *running;

int read_file(int fd, int nbytes)
{
    int bytes_read;

    char buf[nbytes];

    if(fd < 0 || fd > NFD)
    {
        printf("File descriptor provided is not valid.");
        return -1;
    }
    
    bytes_read = my_read(fd, buf, nbytes);

    printf("%s\n", buf);

    return bytes_read;
} 


int my_read(int fd, char buf[], int nbytes)
{
    MINODE *mip;
    OFT *oftp;
    int *indirect, *d_indirect;

    int avil, blk, lbk, dblk, startByte, remain, count = 0;

    char readbuf[BLKSIZE];
    int buf_12[256], buf_13[256], dbuf[256];

    char *cq, *cp;
    cq = buf;

    oftp = running->fd[fd];
    mip = oftp->mptr;

    // num of bytes available in the file
    avil = mip->INODE.i_size - oftp->offset; 

    while (nbytes && avil)
    {
        lbk = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;

        // DIRECT
        if (lbk < 12)
        {
            // map logical lbk to physical blk
            blk = mip->INODE.i_block[lbk];
        }
        // INDIRECT
        else if (lbk >= 12 && lbk < (256+12))
        {
            get_block(mip->dev, mip->INODE.i_block[12], buf_12);

            indirect = (int*)buf_12 + lbk - 12;
            blk = *indirect;
        }
        // DOUBLE INDIRECT
        else
        {
            lbk -= (12 + 256);

            get_block(mip->dev, mip->INODE.i_block[13], buf_13);

            dblk = buf_13[lbk/256];

            get_block(mip->dev, dblk, dbuf);

            blk = dbuf[lbk%256];

        }

        // get data block into readbuf
        get_block(mip->dev, blk, readbuf);

        cp = readbuf + startByte;
        remain = BLKSIZE - startByte;

        // reading one byte at a time
        while(remain > 0)
        {
            *cq++ = *cp++;
            oftp->offset++;
            count++;
            avil--;
            nbytes--;
            remain--;
            if (nbytes <= 0 || avil <= 0)
                break;
        }

    }

    printf("READ: read %d bytes from file descriptor %d\n", count, fd);

    return count;
}

