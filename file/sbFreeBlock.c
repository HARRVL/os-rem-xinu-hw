/* sbFreeBlock.c - sbFreeBlock */
#include <xinu.h>
#include <device.h>
#include <memory.h>
#include <disk.h>
#include <file.h>

devcall swizzle(int diskfd, struct freeblock *freeblk)
{
	struct freeblock *free2 = freeblk->fr_next;
	if (freeblk->fr_next == NULL)
	{
		freeblk->fr_next = 0;
	} else {
		freeblk->fr_next = free2->fr_blocknum;
	}

	seek(diskfd, freeblk->fr_blocknum);
	if (SYSERR == write(diskfd, freeblk, sizeof(struct freeblock)))
	{
		return SYSERR;
	}

	freeblk->fr_next = free2;

	return OK;
}

devcall swizzleSuperBlock(int diskfd, struct superblock *psuper)
{
	
	struct freeblock *swizzle = psuper->sb_freelst;
	struct dirblock *swizzle2 = psuper->sb_dirlst;

	psuper->sb_freelst = swizzle->fr_blocknum;
	psuper->sb_dirlst = swizzle2->db_blocknum;

	seek(diskfd, psuper->sb_blocknum);
	
	if (SYSERR == write(diskfd, psuper, sizeof(struct superblock)))
	{
		return SYSERR;
	}

	psuper->sb_freelst = swizzle; 
	psuper->sb_dirlst = swizzle2;

	return OK;

	
}

devcall sbFreeBlock(struct superblock *psuper, int block)
{
    // TODO: Add the block back into the filesystem's list of
    //  free blocks.  Use the superblock's locks to guarantee
    //  mutually exclusive access to the free list, and write
    //  the changed free list segment(s) back to disk.
 

       int diskfd;
       struct dentry *phw;

       if (NULL == psuper)
       {
	       return SYSERR;
       }

       phw = psuper->sb_disk;
       if (NULL == phw)
       {
	       return SYSERR;
       }

       if ((block <= 0) || (block > DISKBLOCKTOTAL)) 
       {
	       return SYSERR;
       }

       diskfd = phw - devtab;
       wait(psuper->sb_freelock);

       struct freeblock *freeblk = psuper->sb_freelst;


       if (NULL == freeblk)
       {
	    //CASE 1
    	    //malloc space for freeblk & error check
	    freeblk = malloc(sizeof(struct freeblock));
	    if (NULL == freeblk)
	    {
		    return SYSERR;
	    }
	    //set its info
	    freeblk->fr_blocknum = block;
	    freeblk->fr_count = 0;
	    freeblk->fr_next = NULL;
	    //set psuper sb_Freelist to freeblk that was just malloc'd
	    psuper->sb_freelst = freeblk;
	    //swizzle superblock& error check
	    if (NULL == swizzleSuperBlock(diskfd, psuper))
	    {
		    return SYSERR;
	    }
	    //swizzle and write new block to disk & error check
	    if (NULL == swizzle(diskfd, freeblk))
	    {
		    return SYSERR;
	    }

	    //signal to free lock and return OK if all is good
	    signal(psuper->sb_freelock);
	    
	    return OK; //
       }

       while (freeblk->fr_next != NULL)
       {
	       //move freeblk to its next
	       freeblk = freeblk->fr_next;
       }


       //CASE 2
       if (freeblk->fr_count >= FREEBLOCKMAX || ((freeblk->fr_count == 0) && (psuper->sb_freelst == freeblk))) 
       { //how to check if collector node is completely full or completely empty

	       struct freeblock *collector;
	       //malloc space for collector & error check
	       collector = malloc(sizeof(struct freeblock));

	       if (NULL == collector)
	       {
		       return SYSERR;
	       }
	   	
	       freeblk->fr_next = collector;
	       collector->fr_blocknum = block;
	       collector->fr_count = 0;
	       collector->fr_next = NULL;
	       //swizzle & error check
	       if (NULL == swizzle(diskfd, collector))
	       {
		       return SYSERR;
	       }
	       //signal free lock & return OK
	       signal(psuper->sb_freelock);
	       return OK; 

       }

	freeblk->fr_free[freeblk->fr_count] = block;
	freeblk->fr_count++;
	//swizzle & error check
	if (NULL == swizzle(diskfd, freeblk))
	{
		return SYSERR;
	}
	//signal free lock & return OK
	signal(psuper->sb_freelock);

	return OK;
}
