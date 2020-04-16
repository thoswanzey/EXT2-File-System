/************* write.c file **************/

int write_file(int fd, char *str)
{
    char buf[256];

    if(!fd_is_valid(fd))
    {
        printf("File descriptor provided is not valid.");
        return -1;
    }

    strcpy(buf, str);

    return my_write(fd, buf, sizeof(buf));
}

int my_write(int fd, char buf[], int nbytes)
{
    MINODE *mip;
    OFT *oftp;
    int *indirect, *d_indirect;

    int avil, blk, lbk, dblk, startByte, remain, count = 0;

    char writebuf[BLKSIZE];
    int buf_12[256], buf_13[256], dbuf[256];

    char *cq, *cp;
    cq = buf;

    oftp = running->fd[fd];
    mip = oftp->mptr;


    while (nbytes > 0)
    {
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

                // get new allocated block
                get_block(mip->dev, mip->INODE.i_block[12], buf_12);

                // zero out the block on disk
                for(int i = 0; i < BLKSIZE; i++)
                {
                    buf_12[i] = 0;
                }

                // put zero'd out block back
                put_block(mip->dev, mip->INODE.i_block[12], buf_12);

            }
            
            indirect = (int*)buf_12 + lbk - 12;

            blk = indirect[lbk - 12];

            // allocate new block if does not exist
            if(blk == 0)
            {
                // allocate new block
                blk = balloc(mip->dev);
                // record block
                put_block(mip->dev, mip->INODE.i_block[12], blk);
            }
        }
        // DOUBLE INDIRECT
        else
        {
            if(mip->INODE.i_block[13] == 0)
            {
                mip->INODE.i_block[13] = balloc(mip->dev);

                // get new allocated block
                get_block(mip->dev, mip->INODE.i_block[13], buf_13);

                // zero out the block on disk
                for(int i = 0; i < BLKSIZE; i++)
                {
                    buf_13[i] = 0;
                }

                // put zero'd out block back
                put_block(mip->dev, mip->INODE.i_block[13], buf_13);
            }

            lbk -= (12 + 256);

            get_block(mip->dev, mip->INODE.i_block[13], buf_13);

            dblk = buf_13[lbk/256];

            get_block(mip->dev, dblk, dbuf);

            blk = dbuf[lbk%256];

            // allocate new block if does not exist
            if(dblk == 0)
            {
                d_indirect = balloc(mip->dev);
                dblk = d_indirect;

                get_block(mip->dev, dblk, writebuf);
                for(int i = 0; i < BLKSIZE; i++)
                {
                    writebuf[i] = 0;
                }

                put_block(mip->dev, dblk, writebuf);
            }

            // allocate new block if does not exist
            if(blk == 0)
            {
                d_indirect = balloc(mip->dev);
                blk = d_indirect;

                get_block(mip->dev, blk, writebuf);
                for(int i = 0; i < BLKSIZE; i++)
                {
                    writebuf[i] = 0;
                }

                put_block(mip->dev, blk, writebuf);
            }

        }

        // get data block into readbuf
        get_block(mip->dev, blk, writebuf);

        cp = writebuf + startByte;
        remain = BLKSIZE - startByte;

        // reading one byte at a time
        while(remain > 0)
        {
            *cp++ = *cq++;
            nbytes--;
            remain--;
            oftp->offset++;
            if(oftp->offset > mip->INODE.i_size)
            {
                mip->INODE.i_size++;
            }

            if(nbytes <= 0) break;
        }

        put_block(mip->dev, blk, writebuf);

    }
    mip->dirty = 1;
    printf("wrote %d bytes into file descriptor fd=%d\n", nbytes, fd);

    return nbytes;
}