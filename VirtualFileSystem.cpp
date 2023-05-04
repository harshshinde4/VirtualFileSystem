/*
    Customized Virtual File System
*/

#define _CRT_SECURE_NO_WARNINGS

// Header Files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <assert.h>

// Defining The MACROS
#define MAXINODE    50     // Maximum Files To be Created 50
#define READ        1
#define WRITE       2      // Give permission as 3 for both Read & Write
#define MAXFILESIZE 1024   //Max size of a File (1024 = 1 kb)
#define REGULAR      1      // i.e. Regular File
#define SPECIAL     2      // i.e  .c, .py File
#define START       0      // File Offset (lseek)
#define CURRENT     1
#define END         2       // Hole In the File is potentian gap

//SuperBlock structure
typedef struct superblock
{
    int TotalInodes;    // Initially size is 50 for both
    int FreeInode;
} SUPERBLOCK, *PSUPERBLOCK;


// Inode structure
typedef struct inode    // 86 bytes allocated for this block
{
    char FileName[50];  // file name stored
    int InodeNumber;    // inode Number
    int FileSize;       // 1024 byte
    int FileActualSize; // allocated when we write into it i.e. 10 bytes of data
    int FileType;       // type of File
    char *Buffer;       // On windows it stores the block number but in this code it stores 1024 bytes
    int LinkCount;      //linking count
    int ReferenceCount; //reference count
    int Permission;     // read/write permission
    struct inode *next; // self referencial structure

} INODE, *PINODE, **PPINODE;


// File table structure
typedef struct filetable
{
    int readoffset;     // from where to read
    int writeoffset;    // from where to write
    int count;          // remains 1 throughout the code
    int mode;           // mode of file
    PINODE ptrinode;    // pointer, LinkedList point to inode

} FILETABLE, *PFILETABLE;


// UFDT structure
typedef struct ufdt
{
    PFILETABLE ptrfiletable;    // create ufdt structure, (pointer which points to file table)
}UFDT;

UFDT UFDTArr[MAXINODE];     // create array of structure i.e. Array of pointers
SUPERBLOCK SUPERBLOCKobj;   // global variable
PINODE head = NULL;         // global pointer


/*  Function name : man
    description   : To display the description for each commands  */

void man(char *name)
{
    if(name == NULL)
        return;
    if(strcmp(name, "create") == 0)
    {
        puts("Description : Used To Create New Regular File");
        puts("Usage : Create File_name, Permission");
    }

    else if(strcmp(name, "read") == 0)
    {
        puts("Description : Used to Read data from regular file");
        puts("Usage : read File_Name, No_of_Bytes_To_Read");
    }

    else if(strcmp(name, "write") == 0)
    {
        puts("Description : Used to write data into regular file");
        printf("Usage : write File_name\n After this enter the data that we want to write\n");
    }

    else if(strcmp(name, "ls") == 0)
    {
        printf("Description : Used to list all information of files\n");
        printf("Usage : ls\n");
    }

    else if(strcmp(name, "stat") == 0)
    {
        printf("Description : Used to display information of file\n");
        printf("Usage : stat File_name\n");
    }

    else if(strcmp(name, "fstat") == 0)
    {
        printf("Description : Used to display information pf file\n");
        printf("Usage : stat File_Descriptor\n");
    }

    else if(strcmp(name, "truncate") == 0)
    {
        printf("Desription : Used to remove dat from file\n");
        printf("Usage : truncate File_name\n");
    }

    else if(strcmp(name, "open") == 0)
    {
        printf("Description : To Open existing file\n");
        printf("Usage : open File_name mode\n");
    }

    else if(strcmp(name, "close") == 0)
    {
        printf("Description : Used to close opened file\n");
        printf("Usage : close File_name\n");
    }

    else if(strcmp(name, "closeall") == 0)
    {
        printf("Description : To close all opened file\n");
        printf("Usage : closeall\n");
    }

    else if(strcmp(name, "lseek") == 0)
    {
        printf("Description : To change file offset\n");
        printf("Usage : lseek File_name changeInOffset StartPoint\n");
    }

    else if(strcmp(name, "rm") == 0)
    {
        printf("Desciption : To delete the file\n");
        printf("Usage : rm File_Name\n");
    }

    else if(strcmp(name, "backup") == 0)
    {
        printf("Description : To take backup of all Files created\n");
        printf("Usage : backup\n");
    }

    else 
        printf("ERROR : Sorry, we couldn't fine any entry with this comand\n");
}


/*  Function Name : DisplayHelp
    Description : To display all List/Operations about this application     */

void DisplayHelp()
{
    puts("ls           :    To List out all files");
    puts("clear        :    To clear console");
    puts("create       :    To create new file");
    puts("open         :    To open the file");
    puts("close        :    To close the file");
    puts("closeall     :    To close all opened files");
    puts("read         :    To read the contents from the file");
    puts("write        :    To erite contents into file");
    puts("lseek        :    To reposition the file offset");
    puts("exit         :    To terminate the file system");
    puts("stat         :    To Display file information using name");
    puts("fstat        :    To Display file information using file descriptor");
    puts("truncat      :    To Remove all data from file");
    puts("rm           :    To Delete the file");
    puts("backup       :    To take backup of all created files");
}

/*  Function Name : GetFDFromName
    description   : Get File Descriptor value   */

int GetFDFromName(char *name)
{
    int i = 0;
    while(i < MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            if(strcmp((UFDTArr[i].ptrfiletable -> ptrinode -> FileName), name) == 0)
                break;
        }
        i++;
    }

    if(i == MAXINODE)
        return -1;
    else 
        return i;
}

/*  Function Name : Get_Inode 
    Description : Return Inode value of File    */

PINODE Get_Inode(char *name)
{
    PINODE temp = head;
    int i = 0;
    if(name == NULL)
        return NULL;
    
    while(temp != NULL)
    {
        if(strcmp(name, temp -> FileName) == 0)
            break;
        temp = temp->next;
    }
    return temp;
}

/*  Function Name : Create DILB 
    Description : It created the DILB when program starts   */

void CreateDILB(void)
{
    int i = 1;
    PINODE newn = NULL;
    PINODE temp = head;     // Insert last Function from LinkedList

    while(i <= MAXINODE)
    {
        newn = (PINODE)malloc(sizeof(INODE));   // 86 Bytes of memory gets allocated

        newn -> LinkCount = newn -> ReferenceCount = 0;
        newn -> FileType = newn -> FileSize = 0;
        newn -> Buffer = NULL;
        newn -> next = NULL;
        newn -> InodeNumber = i;

        if(temp == NULL)
        {
            head = newn;
            temp = head;
        }
        else 
        {
            temp -> next = newn;
            temp = temp -> next;
        }
        ++i;
    }
    printf("DILB created successfully\n");
}

/*  Function Name : InitializeSuperBlock
    Description : Initialize Inode Values   */

void InitializeSuperBlock(void)
{
    int i = 0;
    while(i < MAXINODE)
    {
        UFDTArr[i].ptrfiletable = NULL;     // this loop sets all pointers at NULL
        ++i;
    }

    SUPERBLOCKobj.TotalInodes = MAXINODE;
    SUPERBLOCKobj.FreeInode = MAXINODE;
}

/* Function Name : Create File 
    Description : Creates New Files     */

int CreateFile (char *name, int permission)
{
    int i = 0;
    PINODE temp = head;

    if((name == NULL) || (permission == 0) || (permission > 3))
        return -1;
    
    if(SUPERBLOCKobj.FreeInode == 0)
        return -2;
    
    (SUPERBLOCKobj.FreeInode)--;
    
    if(Get_Inode(name) != NULL)
        return -3;

    while(temp != NULL)
    {
        if(temp -> FileType == 0)
            break;
        temp = temp->next; 
    }

    while(i < MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
            break;
        ++i;
    }

    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    
    if(UFDTArr[i].ptrfiletable == NULL)
        return -4;

    UFDTArr[i].ptrfiletable -> count = 1;
    UFDTArr[i].ptrfiletable -> mode = permission;
    UFDTArr[i].ptrfiletable -> readoffset = 0;
    UFDTArr[i].ptrfiletable -> writeoffset = 0;

    UFDTArr[i].ptrfiletable -> ptrinode = temp;

    strcpy(UFDTArr[i].ptrfiletable -> ptrinode -> FileName, name);
    UFDTArr[i].ptrfiletable -> ptrinode -> FileType = REGULAR;
    UFDTArr[i].ptrfiletable -> ptrinode -> ReferenceCount = 1;
    UFDTArr[i].ptrfiletable -> ptrinode -> LinkCount = 1;
    UFDTArr[i].ptrfiletable -> ptrinode -> FileSize = MAXFILESIZE;
    UFDTArr[i].ptrfiletable -> ptrinode -> FileActualSize = 0;
    UFDTArr[i].ptrfiletable -> ptrinode -> Permission = permission;
    UFDTArr[i].ptrfiletable -> ptrinode -> Buffer = (char*)malloc(MAXFILESIZE);
    memset(UFDTArr[i].ptrfiletable -> ptrinode -> Buffer, 0, 1024);
    
    return i;
}

/*  Function Name : rm_File
    Description : remove created files  */

int rm_File(char *name)
{
    int fd = 0;
    
    fd = GetFDFromName(name);
    assert(fd);
    (UFDTArr[fd].ptrfiletable -> ptrinode -> LinkCount)--;

    if(UFDTArr[fd].ptrfiletable -> ptrinode -> LinkCount == 0)
    {
        UFDTArr[fd].ptrfiletable -> ptrinode -> FileType = 0;
        strcpy(UFDTArr[fd].ptrfiletable -> ptrinode -> FileName, " ");
        UFDTArr[fd].ptrfiletable -> ptrinode -> ReferenceCount = 0;
        UFDTArr[fd].ptrfiletable -> ptrinode -> Permission = 0;
        UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize = 0;
        free(UFDTArr[fd].ptrfiletable -> ptrinode -> Buffer);
        free(UFDTArr[fd].ptrfiletable);
    }

    UFDTArr[fd].ptrfiletable = NULL;
    (SUPERBLOCKobj.FreeInode)++;
    printf("File Successfully Deleted\n");
    return true;
}

/*  Function Name : ReadFile
    Description : Read data from the file   */

int ReadFile(int fd, char *arr, int isize)
{
    int read_size = 0;

    if(UFDTArr[fd].ptrfiletable == NULL)
        return -1;
    if(UFDTArr[fd].ptrfiletable -> mode != READ && UFDTArr[fd].ptrfiletable -> mode != READ + WRITE)
        return -2;
    if(UFDTArr[fd].ptrfiletable -> ptrinode -> Permission != READ && UFDTArr[fd].ptrfiletable -> ptrinode -> Permission != READ + WRITE)
        return -2;
    if(UFDTArr[fd].ptrfiletable -> readoffset == UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize)
        return -3;
    if(UFDTArr[fd].ptrfiletable -> ptrinode -> FileType != REGULAR)
        return -4;
    if(UFDTArr[fd].ptrfiletable -> ptrinode -> FileType != REGULAR)
        return -5;
    
    read_size = (UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize) - (UFDTArr[fd].ptrfiletable -> readoffset);
    
    if(read_size < isize)
    {
        strncpy(arr, (UFDTArr[fd].ptrfiletable -> ptrinode -> Buffer) + (UFDTArr[fd].ptrfiletable -> readoffset), read_size);
        UFDTArr[fd].ptrfiletable -> readoffset = UFDTArr[fd].ptrfiletable -> readoffset + read_size;
    }

    else
    {
        strncpy(arr, (UFDTArr[fd].ptrfiletable -> ptrinode -> Buffer) + (UFDTArr[fd].ptrfiletable -> readoffset), isize);
        (UFDTArr[fd].ptrfiletable -> readoffset) = (UFDTArr[fd].ptrfiletable -> readoffset) + isize;
    }

    return isize;
}

/*  Function Name : WriteFile
    Description : Write data on the file    */

int WriteFile(int fd, char *arr, int isize)
{
    if(((UFDTArr[fd].ptrfiletable -> mode) != WRITE) && (UFDTArr[fd].ptrfiletable -> mode != READ + WRITE))
        return -1;
    if(((UFDTArr[fd].ptrfiletable -> ptrinode -> Permission) != WRITE) && ((UFDTArr[fd].ptrfiletable -> ptrinode -> Permission) != READ + WRITE))
        return -1;
    if((UFDTArr[fd].ptrfiletable -> writeoffset) == MAXFILESIZE)
        return -2;
    if((UFDTArr[fd].ptrfiletable -> ptrinode -> FileType) != REGULAR)
        return -3;
    if(((UFDTArr[fd].ptrfiletable -> ptrinode -> FileType) - (UFDTArr[fd].ptrfiletable -> ptrinode ->FileActualSize)) < isize)
        return -4;
    
    strncpy((UFDTArr[fd].ptrfiletable -> ptrinode -> Buffer) + (UFDTArr[fd].ptrfiletable -> writeoffset), arr, isize);
    (UFDTArr[fd].ptrfiletable -> writeoffset) = (UFDTArr[fd].ptrfiletable -> writeoffset) + isize;
    (UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize) = (UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize) + isize;

    return isize;
}

/*  Function Name : OpenFile
    Description : Open an existing file     */

int OpenFile(char *name, int mode)
{
    int i = 0;
    PINODE temp = NULL;

    if(name == NULL || mode <= 0)
        return -1;
    
    temp = Get_Inode(name);
    assert(temp);

    if(temp -> Permission < mode)
        return -3;
    
    while(i < MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable == NULL)
            break;
        i++;
    }
    UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
    if(UFDTArr[i].ptrfiletable == NULL)
        return -1;
    
    UFDTArr[i].ptrfiletable -> count = 1;
    UFDTArr[i].ptrfiletable -> mode = mode;

    if(mode == READ + WRITE)
    {
        UFDTArr[i].ptrfiletable -> readoffset = 0;
        UFDTArr[i].ptrfiletable -> writeoffset = 0;
    }

    else if(mode == READ)
        UFDTArr[i].ptrfiletable -> readoffset = 0;
    else if(mode == WRITE)
        UFDTArr[i].ptrfiletable -> writeoffset = 0;
    
    UFDTArr[i].ptrfiletable -> ptrinode = temp;
    (UFDTArr[i].ptrfiletable -> ptrinode -> ReferenceCount)++;

    return i;
        printf("File Opened Successfully\n");
}

/*  Function Name : CloseFileByName
    Description : Close exiting file by its file descriptor     */

void CloseFileByName(int fd)
{
    UFDTArr[fd].ptrfiletable -> readoffset = 0;
    UFDTArr[fd].ptrfiletable -> writeoffset = 0;
    (UFDTArr[fd].ptrfiletable -> ptrinode ->ReferenceCount) = 0;
    printf("File Closed Successfully\n");
}

/*  Function Name : CloseFileByName
    Description : Close existing file by its name   */

int CloseFileByName(char *name)
{
    int i = 0;
    i = GetFDFromName(name);
    if( i == -1 )
        return printf("All Open Files Are Closed\n");
    if((UFDTArr[i].ptrfiletable -> ptrinode -> ReferenceCount) == 0)
        return -2;
    
    UFDTArr[i].ptrfiletable -> readoffset = 0;
    UFDTArr[i].ptrfiletable -> writeoffset = 0;
    (UFDTArr[i].ptrfiletable -> ptrinode -> ReferenceCount) = 0;
    printf("File Closed Successfully\n");
    return 0;
}

/*  Function Name : CloseAllFiles
    Description : Close all existing files  */

void CloseAllFile(void)
{
    int i = 0;
    while(i < MAXINODE)
    {
        if(UFDTArr[i].ptrfiletable != NULL)
        {
            UFDTArr[i].ptrfiletable -> readoffset = 0;
            UFDTArr[i].ptrfiletable -> writeoffset = 0;
            (UFDTArr[i].ptrfiletable -> ptrinode -> ReferenceCount) = 0;
        }
        i++;
    }
    printf("All Files Are Closed Successfully\n");
}

/*  Function Name : LseekFile
    Description : Write data into the file from perticular position     */

int LseekFile(int fd, int size, int from)
{
    if((fd < 0) || (from > 2))
        return -1;
    if(UFDTArr[fd].ptrfiletable == NULL)
        return -1;
    if(UFDTArr[fd].ptrfiletable -> ptrinode -> ReferenceCount == 0)
        return -2;
    
    if((UFDTArr[fd].ptrfiletable -> mode == READ) || (UFDTArr[fd].ptrfiletable -> mode == READ + WRITE))
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable -> readoffset) + size) > UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize)
                return -1;
            if(((UFDTArr[fd].ptrfiletable -> readoffset) + size) < 0)
                return -1;
            return (UFDTArr[fd].ptrfiletable -> readoffset) = (UFDTArr[fd].ptrfiletable -> readoffset) + size;
        }

        else if(from == START)
        {
            if(size > (UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize))
                return -1;
            if(size < 0)
                return -1;
            return (UFDTArr[fd].ptrfiletable -> readoffset) = size;
        }

        else if(from == END)
        {
            if((UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize) + size > MAXFILESIZE)
                return -1;
            if(((UFDTArr[fd].ptrfiletable -> readoffset) + size) < 0)
                return -1;
            return (UFDTArr[fd].ptrfiletable -> readoffset) = (UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize) + size;

        }

    }

    else if(UFDTArr[fd].ptrfiletable -> mode == WRITE)
    {
        if(from == CURRENT)
        {
            if(((UFDTArr[fd].ptrfiletable -> writeoffset) + size) > MAXFILESIZE)
                return -1;
            if(((UFDTArr[fd].ptrfiletable -> writeoffset) + size) < 0)
                return -1;
            
            if(((UFDTArr[fd].ptrfiletable -> writeoffset) + size) > (UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize))
            {
                (UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize) = (UFDTArr[fd].ptrfiletable -> writeoffset) + size;
                return (UFDTArr[fd].ptrfiletable -> writeoffset) = (UFDTArr[fd].ptrfiletable -> writeoffset) + size;
            }
        }

        else if(from == START)
        {
            if(size > MAXFILESIZE)
                return -1;
            if(size < 0)
                return -1;
            
            if(size > UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize)
            {
                (UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize) = size;
                return (UFDTArr[fd].ptrfiletable -> writeoffset) = size;
            }
        }

        else if(from == END)
        {
            if((UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize) + size > MAXFILESIZE)
                return -1;
            if(((UFDTArr[fd].ptrfiletable ->ptrinode) + size) < 0)
                return -1;

            return (UFDTArr[fd].ptrfiletable -> writeoffset) = (UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize) + size;
        }
    }
    printf("Successfully changed\n");
}


/*  Function Name : ls_file
    Description : List out all existing file names      */

void ls_file(void)
{
    int i = 0;
    PINODE temp = head;

    if(SUPERBLOCKobj.FreeInode == MAXINODE)
    {
        puts("Error : There are NO files");
        return;
    }
    printf("\nFile Name\tInode Number\tFile Size\tLink Count\n");
    puts("----------------------------------------------------------");

    while(temp != NULL)
    {
        if(temp -> FileType != 0)
            printf("%s\t\t%d\t\t%d\t\t%d\n", temp->FileName, temp->InodeNumber, temp->FileActualSize, temp->LinkCount);
    }
    temp = temp -> next;
}


/*  Function Name : fstat_file  
    Description : Display statistical information on file by FD     */

int fstat_file(int fd)
{
    PINODE temp = head;
    int i = 0;
    
    if(fd < 0)
        return -1;
    if(UFDTArr[fd].ptrfiletable -> ptrinode);

    printf("\n---------------Statistical Information about files---------------\n");
    printf("File Name : %s\n", temp -> FileName);
    printf("Inode Number : %d\n", temp -> InodeNumber);
    printf("File Size : %d\n", temp -> FileSize);
    printf("Actual File Size : %d\n", temp -> FileActualSize);
    printf("Link Count : %d\n", temp -> LinkCount);
    printf("Reference Count : %d\n", temp -> ReferenceCount);

    if(temp -> Permission == 1)
        printf("File Permission : Read only\n");
    else if(temp -> Permission == 2)
        printf("File Permission : Write\n");
    else if(temp -> Permission == 3)
        printf("File Permission : Read & Write\n");

    printf("------------------------------------------------------------------\n\n");
    return 0;
}

/*  Function Name : stat_file
    Description : Display statistical Information of the file by file name  */

int stat_file(char *name)
{
    PINODE temp = head;
    int i = 0;

    assert(name);

    while(temp != NULL)
    {
        if(strcmp(name, temp -> FileName) == 0)
            break;
        temp = temp -> next;
    }

    assert(temp);

    printf("\n-------------Statistical Information about file-------------\n");
    printf("File Name : %s\n", temp -> FileName);
    printf("Inode Number : %d\n", temp -> InodeNumber);
    printf("File size : %d\n", temp -> FileSize);
    printf("Actual File Size : %d\n", temp -> FileActualSize);
    printf("Link Count : %d\n", temp -> LinkCount);
    printf("Reference Count : %d\n", temp -> ReferenceCount);

    if(temp -> Permission == 1)
        printf("File Permission : Read only\n");
    else if(temp -> Permission == 2)
        printf("File permission : Write\n");
    else if(temp -> Permission == 3)
        printf("File Permission : Read & Write\n");
    printf("-------------------------------------------------------------------\n\n");

    return 0;
}

/*  Function Name : truncate_File 
    Description : Delete all data from the file     */

int truncate_File(char *name)
{
    int fd = GetFDFromName(name);
    assert(fd);

    memset(UFDTArr[fd].ptrfiletable -> ptrinode -> Buffer, 0, MAXFILESIZE);
    UFDTArr[fd].ptrfiletable -> readoffset = 0;
    UFDTArr[fd].ptrfiletable -> writeoffset = 0;
    UFDTArr[fd].ptrfiletable -> ptrinode -> FileActualSize = 0;
    printf("Data Successfully Removed\n");
    return true;
}

/*  Function Name : Backup
    Description : Take Backup of all created files into hard-disk   */

void backup(void)
{
    PINODE temp = head;
    int fd = 0;
    while(temp != NULL)
    {
        if(temp -> FileType != 0)
        {
            fd = creat(temp -> FileName, 0777);
            write(fd, temp -> Buffer, temp -> FileActualSize);
        }
        temp = temp -> next;
    }
    printf("Successfully Get the Backup of all created files...\n");
}

/*  Function Name : main 
    Description : Entry point function  */

int main(void)
{
    char *ptr = NULL;
    int ret = 0, fd = 0, count = 0;
    char command[4][80], str[80], arr[MAXFILESIZE];

    InitializeSuperBlock();
    CreateDILB();

    while(1)
    {
        fflush(stdin);
        strcpy(str, "");
        printf("\nCustomized VFS : > ");
        fgets(str, 80, stdin);  // scanf("%[^'\n']s", str);

        count = sscanf(str, "%s %s %s %s", command[0], command[1], command[2], command[3]);

        if(count == 1)
        {
            if(strcmp(command[0], "ls") == 0)
                ls_file();

            else if(strcmp(command[0], "closeall") == 0)
            {
                CloseAllFile();
                continue;
            }
            else if(strcmp(command[0], "clear") == 0)
            {
                system("cls");
                continue;
            }
            else if(strcmp(command[0], "help") == 0)
            {
                DisplayHelp();
                continue;
            }
            else if(strcmp(command[0], "backup") == 0)
            {
                backup();
                continue;
            }
            else if(strcmp(command[0], "exit") == 0)
            {
                puts("Terminating the customized Virtual File System");
                break;
            }
            else 
            {
                printf("\nERROR : Command not found !!\n");
                continue;
            }
        }

        else if(count == 2)
        {
            if(strcmp(command[0], "stat") == 0)
            {
                ret = stat_file(command[1]);
                if(ret == -1)
                    puts("ERROR : Incorrect parameters");
                if(ret == -2)
                    puts("ERROR : There is no such file");
                continue;
            }

            else if(strcmp(command[0], "fstat") == 0)   // fstat 0
            {
                ret = fstat_file(atoi(command[1]));
                if(ret == -1)
                    puts("ERROR : Incorrect parameters");
                if(ret == -2)
                    puts("ERROR : There is no such file");
                continue;
            }

            else if(strcmp(command[0], "close") == 0)
            {
                ret = CloseFileByName(command[1]);
                if(ret == -1)
                    puts("ERROR : Incorrect parameters");
                if(ret == -2)
                    puts("ERROR : There is no such file");
                continue;
            }

            else if(strcmp(command[0], "rm") == 0)
            {
                ret = rm_File(command[1]);
                if(ret == -1)
                    puts("ERROR : There is no such file");
                continue;
            }
            else if(strcmp(command[0], "man") == 0)
            {
                man(command[1]);
            }
                
            else if(strcmp(command[0], "write") == 0)
            {
                fd = GetFDFromName(command[1]);
                if(fd == -1)
                {
                    puts("ERROR : Incorrect parameters");
                    continue;
                }
                if(UFDTArr[fd].ptrfiletable -> ptrinode -> ReferenceCount == 0)
                    puts("ERROR : File is not Opened");
                else
                {
                    printf("Enter the data : \n");
                    scanf("%[^\n]", arr);
                }
                //fflush(stdin); // empty input buffer

                ret = strlen(arr);
                if(ret == 0)
                {
                    printf("ERROR : Incorrect parameters\n");
                    continue;
                }

                ret = WriteFile(fd, arr, ret);  // 0, addr, 4
                if(ret == -1)
                    puts("ERROR : Permission denied");
                if(ret == -2)
                    puts("ERROR : There is no sufficient memory to write");
                if(ret == -3)
                    puts("ERROR : It is not regular file");
                if(ret == -4)
                    puts("ERROR : There is no sufficient memory available");
                if(ret > 0)
                    printf("Successfully written : %d bytes\n", ret);
            }

            else if(strcmp(command[0], "truncate") == 0)
            {
                ret = truncate_File(command[1]);
                if(ret == -1)
                    puts("ERROR : Incorrect parameter");
            }

            else 
                printf("\nERROR : Command not found..!\n");
            
            continue;
        }

        else if(count == 3)
        {
            if(strcmp(command[0], "create") == 0)
            {
                ret = CreateFile(command[1], atoi(command[2])); // ASCII to integer
                if(ret >= 0)
                    printf("File created succesfully with file descriptor : %d\n", ret);
                if(ret == -1)
                    puts("ERROR : Incorrect parameters");
                if(ret ==-2)
                    puts("ERROR : There is no inode\n");
                if(ret == -3)
                    puts("ERROR : File already exists\n");
                if(ret == -4)
                    puts("ERROR : Memory allocation failure");
                continue;
            }

            else if(strcmp(command[0], "open") == 0)
            {
                ret = OpenFile(command[1], atoi(command[2]));
                
                if(ret >= 0)
                    printf("File is successfully opened with file descriptor : %d\n", ret);
                if(ret == -1)
                    puts("ERROR : Incorrect parameters");
                if(ret == -2)
                    puts("ERROR : File not present");
                if(ret == -3)
                    puts("ERROR : Permission denied");
                continue;
            }

            else if(strcmp(command[0], "read") == 0)
            {
                fd = GetFDFromName(command[1]);
                if(fd == -1)
                {    
                    puts("ERROR : Incorrect parameter");
                    continue;
                }

                ptr = (char*)malloc(sizeof(atoi(command[2])) + 1);
                if(ptr == NULL)
                {
                    printf("Error : Memory allocation failure\n");
                    continue;
                }

                ret = ReadFile(fd, ptr, atoi(command[2]));

                if(ret == -1)
                    puts("ERROR : File not existing\n");
                if(ret == -2)
                    printf("ERROR : Permission denied\n");
                if(ret == -3)
                    puts("ERROR : Reached at end of file");
                if(ret == -4)
                    puts("ERROR : Not a Regular file");
                if(ret == -5)
                    puts("ERROR : File is not opened");
                if(ret == 0)
                    puts("ERROR : File empty");
                if(ret > 0)
                    write(2, ptr, ret); // 0 for stdin, 1 for stdout
                continue;
            }

            else 
            {
                printf("\nERROR : Command not found..!\n");
                continue;
            }
        }

        else if(count == 4)
        {
            if(strcmp(command[0], "lseek") == 0)
            {
                fd = GetFDFromName(command[1]);
                if(fd == -1)
                {
                    puts("ERROR : Incorrect parameter\n");
                    continue;
                }

                ret = LseekFile(fd, atoi(command[2]), atoi(command[3]));
                if(ret == -1)
                    puts("ERROR : Unable to perform lseek");
                
                if(ret == -2)
                    puts("ERROR : File is not opened");                
            }

            else 
            {
                printf("\nERROR : Command not found..!\n Please try again.\n");
                continue;
            }
        }

        else 
        {
            printf("\nERROR : Command not found..!\n please try again\n");
            continue;
        }
    }
    return 0;
    
}
