/************* write.c file **************/


int my_write(int fd, char buf[], int nbytes)
{
    MINODE *mip;
    OFT *oftp;

    int avil, blk, lbk, dblk, startByte, remain, count = 0;

    char writebuf[BLKSIZE], tempbuf[BLKSIZE];
    int buf_12[256], buf_13[256], dbuf[256];

    char *cq, *cp;
    cq = buf;

    oftp = running->fd[fd];
    mip = oftp->mptr;


    lbk = oftp->offset / BLKSIZE;
    startByte = oftp->offset % BLKSIZE;

    // DIRECT
    if (lbk < 12)
    {
        if(mip->INODE.i_block[lbk] == 0)
        {
            mip->INODE.i_block[lbk] = balloc(mip->dev);
        }
        blk = mip->INODE.i_block[lbk];
    }
    // INDIRECT
    else if (lbk >= 12 && lbk < (256+12))
    {
        // if free, allocate new block
        if (mip->INODE.i_block[12] == 0)
        {
            mip->INODE.i_block[12] = balloc(mip->dev);
        }
        
        lbk -= 12;
        get_block(mip->dev, mip->INODE.i_block[12], buf_12);

        blk = buf_12[lbk];
        // allocate new block if does not exist
        if(blk == 0)
        {
            // allocate new block
            blk = buf_12[lbk] = balloc(mip->dev);
            // record block
            put_block(mip->dev, mip->INODE.i_block[12], buf_12);
        }
    }
    // DOUBLE INDIRECT
    else
    {
        if(mip->INODE.i_block[13] == 0)
        {
            mip->INODE.i_block[13] = balloc(mip->dev);
        }

        lbk -= (12 + 256);

        get_block(mip->dev, mip->INODE.i_block[13], buf_13);

        dblk = buf_13[lbk/256];

        // allocate new block if does not exist
        if(dblk == 0)
        {
            dblk = buf_13[lbk/256] = balloc(mip->dev);
            put_block(mip->dev, mip->INODE.i_block[13], buf_13);
        }

        get_block(mip->dev, dblk, dbuf);

        blk = dbuf[lbk%256];

        // allocate new block if does not exist
        if(blk == 0)
        {
            blk = dbuf[lbk%256] = balloc(mip->dev);
            put_block(mip->dev, dblk, dbuf);
        }

    }

    // get data block into readbuf
    get_block(mip->dev, blk, writebuf);

    cp = writebuf + startByte;
    remain = BLKSIZE - startByte;

    strncpy(cp, cq, nbytes);
    oftp->offset+=nbytes;
    if(oftp->offset > mip->INODE.i_size)
    {
        mip->INODE.i_size+=nbytes;
    }

    put_block(mip->dev, blk, writebuf);

    mip->dirty = 1;
    printf("wrote %d bytes into file descriptor fd=%d\n", count, fd);

    return nbytes;
}

int write_file(int fd, char *str)
{
    char buf[256];

    if(fd < 0 || fd > NFD)
    {
        printf("File descriptor provided is not valid.");
        return -1;
    }

    strcpy(buf, str);

    return my_write(fd, buf, strlen(buf));
}