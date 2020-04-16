/************* pfd.c file **************/

int my_pfd(void)
{
    char * modes[] = {"READ", "WRITE", "READ/WRITE", "APPEND"};
    printf(BOLD" fd      mode     offset     INODE\n"RESET);
    printf("----  ----------  ------  -----------\n");
    for( int i = 0; i < NFD; i++)
    {
        if(running->fd[i]){
            printf("%-4d  %-10s  %-6d  [%-4d,%-4d]\n", i, modes[running->fd[i]->mode], running->fd[i]->offset, running->fd[i]->mptr->dev, running->fd[i]->mptr->ino);
        }
    }
    return 0;
}