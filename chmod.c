/************* chmod.c file **************/
extern int dev;


int my_chmod(char * path, char * mode)
{
    if(!mode[0]){//if no mode specified
        printf(ERROR"ERROR -> no mode specified\n"RESET);
        return -1;
    }

    int ino = getino(path);
    if(!ino){
        printf(ERROR"ERROR -> file does not exist\n"RESET);
        return -2;
    }

    int newMode;

    if(mode[0] == '0')
    {
        mode++;
    }
    else
    {
        printf(ERROR"ERROR -> mode value must be in hexadecimal or octal\n"RESET);
        return -3;
    }

    //If we get here we know the value is 0*****, and it will be assumed that all remaining characters
    //(other than the possible 'X' for hex) are numbers
    if(toupper(mode[0]) == 'X')
    {
        newMode = strtol(++mode, NULL, 16);
    }
    else
    {
        newMode = strtol(mode, NULL, 8);
    }


    MINODE * mip = iget(dev, ino);

    mip->INODE.i_mode |= newMode;
    mip->dirty = 1;
    iput(mip);
    return 0;
}