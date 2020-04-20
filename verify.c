/*********** verify.c file ****************/

int verify_blocks(char *path)
{
    int ino = getino(path);

    if(ino == 0)
    {
        printf(ERROR"ERROR -> File doesn't exist\n"RESET);
        return -1;
    }

    MINODE *mip = iget(dev, ino);

    int blk, dblk;

    int buf_12[256], buf_13[256], dbuf[256];

    char bitMapBuf[BLKSIZE];
    get_block(dev, bmap, bitMapBuf);

//------------------------------------- Check direct blocks-------------------------------
    for(int i = 0; i < 12; i++)
    {
        // map logical lbk to physical blk
        blk = mip->INODE.i_block[i];
        if(blk == 0) return i;
        if (!tst_bit(bitMapBuf, blk)) {
            printf(ERROR"FOUND Block used by file, but not reserved in bitmap (bno = %d)\n"RESET, blk);
            set_bit(bitMapBuf, blk);
            put_block(dev, bmap, bitMapBuf);
            printf(GRN BOLD"Set bno = %d as allocated\n"RESET, blk);
        }
    }
    if(mip->INODE.i_block[12] == 0) return 12;

//------------------------------------- Check indirect blocks-------------------------------
    if (!tst_bit(bitMapBuf, mip->INODE.i_block[12])) {
        printf(ERROR"i_block[12] used, but not reserved in bitmap (bno = %d)\n"RESET, mip->INODE.i_block[12]);
        set_bit(bitMapBuf, mip->INODE.i_block[12]);
        put_block(dev, bmap, bitMapBuf);
        printf(GRN BOLD"Set bno = %d as allocated\n"RESET, mip->INODE.i_block[12]);
    }
    get_block(mip->dev, mip->INODE.i_block[12], buf_12);
    for(int i = 0; i < 256; i++){
        // map logical lbk to physical blk
        blk = buf_12[i];
        if(blk == 0) return i + 12;
        if (!tst_bit(bitMapBuf, blk)) {
            printf(ERROR"FOUND Block used by file, but not reserved in bitmap (bno = %d)\n"RESET, blk);
            set_bit(bitMapBuf, blk);
            put_block(dev, bmap, bitMapBuf);
            printf(GRN BOLD"Set bno = %d as allocated\n"RESET, blk);
        }
    }

//------------------------------------- Check double indirect blocks-------------------------------
    if(mip->INODE.i_block[13] == 0) return 12 + 256;

    if (!tst_bit(bitMapBuf, mip->INODE.i_block[13])) {
        printf(ERROR"i_block[13] used, but not reserved in bitmap (bno = %d)\n"RESET, mip->INODE.i_block[13]);
        set_bit(bitMapBuf, mip->INODE.i_block[13]);
        put_block(dev, bmap, bitMapBuf);
        printf(GRN BOLD"Set bno = %d as allocated\n"RESET, mip->INODE.i_block[13]);
    }
    get_block(mip->dev, mip->INODE.i_block[13], buf_13);
    for(int i = 0; i < 256; i++){
        dblk = buf_13[i/256];
        if(dblk == 0) return 12 + 256 + 256*i;
        if (!tst_bit(bitMapBuf, dblk)) {
            printf(ERROR"FOUND Block containing 256 block numbers used by file, but not reserved in bitmap (bno = %d)\n"RESET, dblk);
            set_bit(bitMapBuf, dblk);
            put_block(dev, bmap, bitMapBuf);
            printf(GRN BOLD"Set bno = %d as allocated\n"RESET, dblk);
        }
        get_block(mip->dev, dblk, dbuf);
        for(int j = 0; j < 256; j++){
            blk = dbuf[j];
            if(blk == 0) return 12 + 256 + 256*i + j;
            if (!tst_bit(bitMapBuf, blk)) {
                printf(ERROR"FOUND Block used by file, but not reserved in bitmap (bno = %d)\n"RESET, blk);
                set_bit(bitMapBuf, blk);
                put_block(dev, bmap, bitMapBuf);
                printf(GRN BOLD"Set bno = %d as allocated\n"RESET, blk);
            }
        }
    }

    return 0;
}

