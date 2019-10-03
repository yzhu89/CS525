#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "storage_mgr.h"
#include "dberror.h"

/*
 *******************  Initialize *************************
 */
void initStorageManager(){
    printf("The Storage Manager has been initialized\n\n");
}

/*
 ****************  manipulating page files ****************
 */
FILE *myFile;
int check;

RC createPageFile (char *fileName)
    {
    char t;
    //create a file
    myFile = fopen(fileName, "r");
    //fopen(): Opens a file for reading. The file must exist.
    //if the file already exists, ask the client whether or not to cover it.
    if(myFile != NULL)
    {
        printf("The file already exists. Do you wish to replace it?(y/n)\n");
        scanf("%c", &t);
        fclose(myFile);
    }
    /*if client choose to replace the file
     *OR the file does not exist at the very beginning
     */
    if(t == 'y' || t == 'Y' || myFile == NULL)
    {
        myFile = fopen(fileName, "w");//Creates an empty file for writing.
        /*
         *The initial file size should be one page.
         *This method should fill this single page with '\0' bytes.
         */
        //create a page in cache
        char *newPage = (char *)calloc(PAGE_SIZE, sizeof(char));
        //allocate the newPage into myFile
        fwrite(newPage, sizeof(char), PAGE_SIZE, myFile);
        //allocate the position
        fseek(myFile, 0, SEEK_END);
        //sets the file position of the stream to the given offset from 0 till SEEK_END
        free(newPage);
        fclose(myFile);
        printf("Page File Created Successfully!\n\n");
        return RC_OK;
    }
    else
    {
        printf("Page File Creation Failed.\n\n");
        return RC_FILE_NOT_FOUND;
    }

}

//Opens an existing page file.
RC openPageFile (char *fileName, SM_FileHandle *fHandle)
    {
        myFile = fopen(fileName, "r");
        /*If opening the file is successful
        *Then the fields of this file handle should be initialized
        *with the information about the opened file.
        */
        if(myFile != NULL)
        {
            fHandle->fileName = fileName;
            //find the file size
            int fileSize = ftell(myFile);
            fHandle->curPagePos = fileSize/PAGE_SIZE;

            //find the number of pages
            fseek(myFile, 0, SEEK_END);//move to the end of file
            fileSize = ftell(myFile);
            int numOfPages = fileSize/PAGE_SIZE;
            if(fileSize % PAGE_SIZE != 0)
            {
                fHandle->totalNumPages = numOfPages + 1;
            }
            else
            {
                fHandle->totalNumPages = numOfPages;
            }
            fHandle->mgmtInfo = myFile;
            printf("Page File opened successfully!\n\n");
            fclose(myFile);
            return RC_OK;
        }
        else //in case myFile==NULL
        {
        printf("Failed to open page file.\n\n");
        return RC_FILE_NOT_FOUND;
        }
}


RC closePageFile (SM_FileHandle *fHandle)// To close an open page file
{
    //Checking whether the file handle is void or not
    myFile = fopen(fHandle->fileName, "r");
    if(fHandle == NULL || myFile == NULL)
    {
        printf("Failed to read the block\n\n");
        return RC_FILE_NOT_FOUND;
    }
    /*fclose(): This method returns zero
     *if the stream is successfully closed.
     */
    check = fclose(fHandle->mgmtInfo);
    if(check != 0)
    {
        printf("Failed to close the page file.\n\n");
        return RC_FILE_NOT_FOUND;
    }else
    {
        printf("Closed the page file!\n\n");
        return RC_OK;
    }
}

//Destroy (delete) a page file
RC destroyPageFile (char *fileName)
{
    //Checking whether the fileName is void or not
    if(fileName == NULL)
    {
        printf("The given file name is invalid.\n\n");
        return RC_FILE_NOT_FOUND;
    }
    /*remove(): On success, zero is returned.*/
    check = remove(fileName);
    if(check != 0)
    {
        printf("Failed to delete the page file.\n\n");
        return RC_FILE_NOT_FOUND;
    }else
    {
        printf("Deleted the page file!\n\n");
        return RC_OK;
    }
}

/*
 ***************   Reading blocks from disc  ****************
 */
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
    {
    /*The method reads the pageNumth block from a file
     *Then stores its content in the memory pointed to by the memPage page handle
     */
    //Checking whether the pageNum is void or not
        if(pageNum > fHandle->totalNumPages || pageNum < 0)
        {
            printf("Fail to read block\n\n");
            return RC_READ_NON_EXISTING_PAGE;
        }
    //Checking whether the fHandle/memPage is void or not
    myFile = fopen(fHandle->fileName, "r");
    if(fHandle == NULL || myFile == NULL || memPage == NULL)
    {
        printf("Failed to read the block.\n\n");
        return RC_FILE_NOT_FOUND;
    }
    int pos=pageNum*PAGE_SIZE; //position can be found by page number* size of one page
    fseek(fHandle->mgmtInfo, pos, SEEK_SET);// The pointer is set to position of file stream
    fread(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);//requested block is read
    fHandle->curPagePos = pageNum;// current page position updated to the page number
    fclose(myFile);
    printf("The block is read.\n\n");
    return RC_OK;
}

//Return the current page position in a file
int getBlockPos (SM_FileHandle *fHandle)
{

    myFile = fopen(fHandle->fileName, "r");
    if(fHandle == NULL || myFile == NULL) //Check the fHandle and the file itself is void or not
        {
        printf("Failed to get the block position.\n\n");
        return RC_FILE_NOT_FOUND;
        }
    printf("The current page position inside the file is: ");
    printf("%d", fHandle->curPagePos);
    printf("\n\n");
    fclose(myFile);
    return fHandle->curPagePos; //returning the current page position
}
// in the below functions, the readBlock() function is used to read blocks with the given specifications.

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    printf("Reading the first block.\n\n");
    return readBlock(0, fHandle, memPage);
    // implements readBlock() with pagenum=0, and the specified file and page handle
}

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    printf("Reading the previous block.\n\n");
    int newpos=fHandle->curPagePos-1;
    //the position of the block to be read is one less than the current position
    //readBlock() is implemented from the new block position (newpos),
    //and the specified file and page handle
    return readBlock(newpos, fHandle, memPage);
}

RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    printf("Reading the current block.\n\n");
    int curpos=fHandle->curPagePos;
    // the position of the current position is set to curPagePos
    //readBlock() is implemented from the new block position (curpos),
    //and the specified file and page handle
    return readBlock(curpos, fHandle, memPage);
}

RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    printf("Reading the next block.\n\n");
    int nextpos=fHandle->curPagePos+1;
    //the position of the new block is one more than the current position
    //readBlock() is implemented from the new block position (nextpos),
    //and the specified file and page handle
    return readBlock(nextpos, fHandle, memPage);
}

RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    printf("Reading the last block.\n\n");
    int lastpos=fHandle->totalNumPages;
    //the position of last block is set to the value of total pages
    //readBlock() is implemented from the new block position (lastpos),
    //and the specified file and page handle
    return readBlock(lastpos, fHandle, memPage);
}

/*
 ***************   writing blocks to a page file  ****************
 */
//Write a page to disk using an absolute position
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    /* Check the parameter pageNum is lesser than the total number of pages
     * or is 0 in which case the respective error is returned */
    if(pageNum > fHandle->totalNumPages|| pageNum < 0)
        {
        printf("Failed to write the block.\n\n");
        return RC_READ_NON_EXISTING_PAGE;
        }

    //opening the file in reading and writing mode
    myFile = fopen(fHandle->fileName, "r+");
    /* checking if any of the parameters is null
     *in which case block cannot be written in */
    if(fHandle == NULL || myFile == NULL || memPage == NULL){
        printf("Failed to write the block.\n\n");
        return RC_FILE_NOT_FOUND;
    }
    //move pointer to the beginning of the request page
    fseek(myFile, pageNum*PAGE_SIZE*sizeof(char),SEEK_SET);
    //writing in the requested block
    fwrite(memPage, sizeof(char), PAGE_SIZE, myFile);
    fHandle->curPagePos = pageNum;
    fclose(myFile);
    printf("Page is written to block.\n\n");
    return RC_OK;
}

//Write a page to disk using the current position
//calls the writeBlock() function starting from the current position with the specified file and page handle
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)
{
    printf("Writing the current page to the block.\n\n");
    return writeBlock (fHandle->curPagePos, fHandle, memPage);
}
/* The function will add an empty block to the end of the block
 * Increase the number of pages in the file by one.
 * he new last page should be filled with zero bytes.*/
RC appendEmptyBlock (SM_FileHandle *fHandle)
{
/* Checking whether the fHandle and the file itself is void or not
 * in which case an empty block will not be appended
 * The file stream is opened in reading and writing mode
 * since both functions are to be performed */
    myFile = fopen(fHandle->fileName, "r+");
    if(fHandle == NULL || myFile == NULL)
        {
        printf("Failed to append the empty block\n\n");
        return RC_FILE_NOT_FOUND;
        }
    //A new page is created
    SM_PageHandle newPage = (char *)calloc(PAGE_SIZE, sizeof(char));
    //adding the newPage into myFile
    fseek(myFile, 0, SEEK_END); //move to end of the file
    fwrite(newPage, sizeof(char), PAGE_SIZE, myFile); //write the new empty block
    fHandle->totalNumPages += 1; //update the number of pages
    fHandle->curPagePos = fHandle->totalNumPages; //update the current position
    free(newPage);
    //deallocates the previously allocated memory by a call to calloc, malloc, or realloc.
    fclose(myFile);
    printf("An empty block is appended.\n\n");
    return RC_OK;
}

RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle)
{
    //Check the fHandle and the file itself is void or not
    myFile = fopen(fHandle->fileName, "r");
    if(fHandle == NULL || myFile == NULL)
    {
        printf("Failed to ensure the capacity.\n\n");
        return RC_FILE_NOT_FOUND;
    }
    if (fHandle->totalNumPages < numberOfPages)
    {
        printf("Appending empty blocks to ensure the capacity.\n\n");
        while (fHandle->totalNumPages < numberOfPages)
        {
            appendEmptyBlock(fHandle);
            //appendEmptyBlock() is called until totalNumPages==numberOfPages
        }
    }
    return RC_OK;
}


