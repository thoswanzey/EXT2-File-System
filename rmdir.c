/************* rmdir.c file **************/


int rm_child(MINODE *pip, char *name){
    
    char buf[BLKSIZE], *cp, temp[128];
    DIR *dp, *dp_prev;;

    for(int i = 0; i < 12 ; i++)
	{
		if(pip->INODE.i_block[i] == 0)
			return;

        get_block(dev, pip->INODE.i_block[i], buf);
        cp = buf;
        dp = (DIR*)cp;

        while(cp < buf + BLKSIZE){
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = '\0';

            if(!strcmp(name, temp)){ //Found child
                if(cp + dp->rec_len == buf + 1024)//Last entry in block
                {
                    dp_prev->rec_len += dp->rec_len;
                    put_block(dev, pip->INODE.i_block[i], buf);
                }
                
                else if(cp == buf && cp + dp->rec_len == buf + BLKSIZE)//first and only entry
                {
                    bdalloc(dev, pip->INODE.i_block[i]);
                    for(; pip->INODE.i_block[i] && i < 11; i++)
                    {
                        get_block(dev, pip->INODE.i_block[i + 1], buf);
                        put_block(dev, pip->INODE.i_block[i], buf);
                    }
                    pip->INODE.i_size -= 1024;
                }
                else//in middle of inodes
                {
                    char *final_cp = cp + dp->rec_len;
                    DIR *final_dp = (DIR*)final_cp;
                    while(final_cp + final_dp->rec_len < buf + BLKSIZE)
                    {
                        final_cp += final_dp->rec_len;
                        final_dp = (DIR*)final_cp;
                    }
                    final_dp->rec_len += dp->rec_len;
                    memmove(cp, cp + dp->rec_len, buf + 1024 -(cp + dp->rec_len));
                    put_block(dev, pip->INODE.i_block[i], buf);
                }
                pip->dirty = 1;
                iput(pip);
            }
            dp_prev = dp;
            cp += dp->rec_len;
            dp = (DIR*)cp;
        }
    }
    return -1;
}


int my_rmdir(char * path){
    if(!strcmp(path, "/" ) || !strcmp(path, ".." ) || !strcmp(path, "." ))
    {
        printf("ERROR - Cannot delete root directory, current directory, or parent of current directory\n");
        return -1;
    }
    int ino;
    MINODE * mip;

    ino = getino(path);
    if(ino < 0){
        printf("ERROR - Directory not found\n");
        return -2;
    }

    mip = iget(dev, ino);
    if(mip->INODE.i_uid != running->uid && running->uid != 0){
        printf("ERROR - You do not have permission to do this\n");
        return -3;
    }

    if(!S_ISDIR(mip->INODE.i_mode)){
        printf("ERROR - provided path is not a directory\n");
        iput(mip);
        return -4;
    }

    if(mip->INODE.i_links_count > 2){
        printf("ERROR - Cannot delete a directory that isn't empty\n");
        iput(mip);
        return -5;
    }

    //Dir only has 2 links at this point, but may still contain files
    char buf[BLKSIZE], *cp, filename[64];
	DIR *dp;

    get_block(dev, mip->INODE.i_block[0], buf);
    cp = buf;
    dp = (DIR*)buf;
    while(cp < buf + BLKSIZE){
        strncpy(filename, dp->name, dp->name_len);
        filename[dp->name_len] = '\0';  //Add null termination character
        if(strcmp(filename, ".") && strcmp(filename, "..")){
            iput(mip);
            printf("ERROR - Cannot delete a directory that isn't empty\n");
            return -6;
        }
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    //If we get here, the directory is empty

    for (int i=0; i<12; i++){
        if (mip->INODE.i_block[i]==0)
            break;
        bdalloc(mip->dev, mip->INODE.i_block[i]);
    }
    idalloc(mip->dev, mip->ino);
    iput(mip); //(which clears mip->refCount = 0);


    char parent_path[128], base[128], temp[128];
    strcpy(temp, path);
    strcpy(parent_path, dirname(temp));
    strcpy(temp, path);
    strcpy(base, basename(temp));
    int pino = getino(parent_path);
    MINODE * pip = iget(mip->dev, pino); 

    rm_child(pip, base);

    pip->INODE.i_links_count--;
    pip->INODE.i_atime = pip->INODE.i_mtime = time(NULL);
    pip->dirty = 1;
    iput(pip);

    return 0;
}
