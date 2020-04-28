#include <xinu.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// #ifdef FS
#include <fs.h>

static struct fsystem fsd;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

extern int dev0;

char block_cache[512];

#define SB_BLK 0
#define BM_BLK 1
#define RT_BLK 2

#define NUM_FD 16
struct filetable oft[NUM_FD]; // open file table
int next_open_fd = 0;

#define INODES_PER_BLOCK (fsd.blocksz / sizeof(struct inode))
#define NUM_INODE_BLOCKS (((fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2

int fs_fileblock_to_diskblock(int dev, int fd, int fileblock);

int ceil(int num, int div)
{
    int inum = num / div;
    if (num % div != 0)
    {
        return inum + 1;
    }
    return inum;
}
int fs_fileblock_to_diskblock(int dev, int fd, int fileblock)
{
    int diskblock;

    if (fileblock >= INODEBLOCKS - 2)
    {
        printf("No indirect block support\n");
        return SYSERR;
    }

    diskblock = oft[fd].in.blocks[fileblock]; //get the logical block address

    return diskblock;
}

/* read in an inode and fill in the pointer */
int fs_get_inode_by_num(int dev, int inode_number, struct inode *in)
{
    int bl, inn;
    int inode_off;

    if (dev != 0)
    {
        printf("Unsupported device\n");
        return SYSERR;
    }
    if (inode_number > fsd.ninodes)
    {
        printf("fs_get_inode_by_num: inode %d out of range\n", inode_number);
        return SYSERR;
    }

    bl = inode_number / INODES_PER_BLOCK;
    inn = inode_number % INODES_PER_BLOCK;
    bl += FIRST_INODE_BLOCK;

    inode_off = inn * sizeof(struct inode);

    /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  printf("inn*sizeof(struct inode): %d\n", inode_off);
  */

    bs_bread(dev0, bl, 0, &block_cache[0], fsd.blocksz);
    memcpy(in, &block_cache[inode_off], sizeof(struct inode));

    return OK;
}

/* write inode indicated by pointer to device */
int fs_put_inode_by_num(int dev, int inode_number, struct inode *in)
{
    int bl, inn;

    if (dev != 0)
    {
        printf("Unsupported device\n");
        return SYSERR;
    }
    if (inode_number > fsd.ninodes)
    {
        printf("fs_put_inode_by_num: inode %d out of range\n", inode_number);
        return SYSERR;
    }

    bl = inode_number / INODES_PER_BLOCK;
    inn = inode_number % INODES_PER_BLOCK;
    bl += FIRST_INODE_BLOCK;

    /*
  printf("in_no: %d = %d/%d\n", inode_number, bl, inn);
  */

    bs_bread(dev0, bl, 0, block_cache, fsd.blocksz);
    memcpy(&block_cache[(inn * sizeof(struct inode))], in, sizeof(struct inode));
    bs_bwrite(dev0, bl, 0, block_cache, fsd.blocksz);

    return OK;
}

/* create file system on device; write file system block and block bitmask to
 * device */
int fs_mkfs(int dev, int num_inodes)
{
    int i;

    if (dev == 0)
    {
        fsd.nblocks = dev0_numblocks;
        fsd.blocksz = dev0_blocksize;
    }
    else
    {
        printf("Unsupported device\n");
        return SYSERR;
    }

    if (num_inodes < 1)
    {
        fsd.ninodes = DEFAULT_NUM_INODES;
    }
    else
    {
        fsd.ninodes = num_inodes;
    }

    i = fsd.nblocks;
    while ((i % 8) != 0)
    {
        i++;
    }
    fsd.freemaskbytes = i / 8;

    if ((fsd.freemask = getmem(fsd.freemaskbytes)) == (void *)SYSERR)
    {
        printf("fs_mkfs memget failed.\n");
        return SYSERR;
    }

    /* zero the free mask */
    for (i = 0; i < fsd.freemaskbytes; i++)
    {
        fsd.freemask[i] = '\0';
    }

    fsd.inodes_used = 0;

    /* write the fsystem block to SB_BLK, mark block used */
    fs_setmaskbit(SB_BLK);
    bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(struct fsystem));

    /* write the free block bitmask in BM_BLK, mark block used */
    fs_setmaskbit(BM_BLK);
    bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

    return 1;
}

/* print information related to inodes*/
void fs_print_fsd(void)
{
    printf("fsd.ninodes: %d\n", fsd.ninodes);
    printf("sizeof(struct inode): %d\n", sizeof(struct inode));
    printf("INODES_PER_BLOCK: %d\n", INODES_PER_BLOCK);
    printf("NUM_INODE_BLOCKS: %d\n", NUM_INODE_BLOCKS);
}

/* specify the block number to be set in the mask */
int fs_setmaskbit(int b)
{
    int mbyte, mbit;
    mbyte = b / 8;
    mbit = b % 8;

    fsd.freemask[mbyte] |= (0x80 >> mbit);
    return OK;
}

/* specify the block number to be read in the mask */
int fs_getmaskbit(int b)
{
    int mbyte, mbit;
    mbyte = b / 8;
    mbit = b % 8;

    return (((fsd.freemask[mbyte] << mbit) & 0x80) >> 7);
    return OK;
}

/* specify the block number to be unset in the mask */
int fs_clearmaskbit(int b)
{
    int mbyte, mbit, invb;
    mbyte = b / 8;
    mbit = b % 8;

    invb = ~(0x80 >> mbit);
    invb &= 0xFF;

    fsd.freemask[mbyte] &= invb;
    return OK;
}

/* This is maybe a little overcomplicated since the lowest-numbered
   block is indicated in the high-order bit.  Shift the byte by j
   positions to make the match in bit7 (the 8th bit) and then shift
   that value 7 times to the low-order bit to print.  Yes, it could be
   the other way...  */
void fs_printfreemask(void)
{ // print block bitmask
    int i, j;

    for (i = 0; i < fsd.freemaskbytes; i++)
    {
        for (j = 0; j < 8; j++)
        {
            printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
        }
        if ((i % 8) == 7)
        {
            printf("\n");
        }
    }
    printf("\n");
}

int fs_open(char *filename, int flags)
{
    if (next_open_fd == 0)
    {
        printf("No files exist in the system!");
    }
    for (int iterator = 0; iterator < next_open_fd; iterator++)
    {
        if (strcmp(oft[iterator].de->name, filename) == 0)
        {
            if (oft[iterator].state == FSTATE_OPEN)
            {
                printf("The file is perhaps already open!\n");
                return SYSERR;
            }
            else
            {
                oft[iterator].state = FSTATE_OPEN;
                oft[iterator].flag = flags;
                return iterator;
            }
        }
    }

    printf("The File you tried to open, does not exist!\n");
    return SYSERR;
}

int fs_close(int fd)
{
    if (fd <= next_open_fd)
    {
        if (fd >= 0)
        {
            if (oft[fd].state == FSTATE_CLOSED)
            {
                printf("The file you tried is close is already closed or does not exist!\n");
                return SYSERR;
            }
            else
            {
                oft[fd].state = FSTATE_CLOSED;
                printf("\nFile successfully closed\n");
                return OK;
            }
        }
        else
        {
            printf("\nNegative File descriptor provided!\n");
            return SYSERR;
        }
    }
    else
    {
        printf("\nFile Descriptor was more than number of enteries in oft!");
        return SYSERR;
    }
}
int fs_create(char *filename, int mode)
{ //Check if the mode is valid
    if (mode != O_CREAT)
    {
        printf("File mode is not create while calling  fs_create!\n");
        return SYSERR;
    }
    if (fsd.ninodes == fsd.inodes_used)
    {
        printf("Inodes dont exist!\n");
        return SYSERR;
    }
    for (int iterator = 0; iterator < next_open_fd; iterator++)
    {
        if (strcmp(oft[iterator].de->name, filename) == 0)
        {
            printf("Looks like the file name you are trying to create already exists!\n");
            return SYSERR;
        }
    }
    oft[next_open_fd].state = FSTATE_OPEN;
    oft[next_open_fd].fileptr = 0;
    oft[next_open_fd].de = getmem(sizeof(struct dirent));
    strcpy(oft[next_open_fd].de->name, filename);
    oft[next_open_fd].de->inode_num = fsd.inodes_used;
    oft[next_open_fd].in.id = fsd.inodes_used;
    oft[next_open_fd].in.device = 0;
    oft[next_open_fd].in.size = 0;
    oft[next_open_fd].in.type = INODE_TYPE_FILE;
    oft[next_open_fd].in.nlink = 1;
    oft[next_open_fd].flag = O_RDWR;
    fs_put_inode_by_num(0, oft[next_open_fd].in.id, &oft[next_open_fd].in);
    int filedescriptor = next_open_fd;
    next_open_fd = next_open_fd + 1;
    fsd.inodes_used = fsd.inodes_used + 1;
    return filedescriptor;
}

int fs_seek(int fd, int offset)
{

    if (fd <= next_open_fd)
    {
        if (fd >= 0)
        {
            if (oft[fd].state == FSTATE_CLOSED)
            {
                printf("The file you tried is close is already closed or does not exist!\n");
                return SYSERR;
            }
            else
            {
                oft[fd].fileptr = oft[fd].fileptr + offset;
                printf("\nFile successfully closed\n");
                return OK;
            }
        }
        else
        {
            printf("\nNegative File descriptor provided!\n");
            return SYSERR;
        }
    }
    else
    {
        printf("\nFile Descriptor was more than number of enteries in oft!");
        return SYSERR;
    }
}

int fs_read(int fd, void *buf, int nbytes)
{

    if (oft[fd].state != FSTATE_CLOSED)
    {

        if (oft[fd].flag != O_WRONLY)
        {
            int bytes_written = 0;
            int initial_block = oft[fd].fileptr / MDEV_BLOCK_SIZE;
            int blocks_number = (nbytes / MDEV_BLOCK_SIZE) + 1;
            int initial_block_num = oft[fd].fileptr;
            int block_iterator = initial_block;

            int temp_num = oft[fd].de->inode_num;

            struct inode *temp_inode = getmem(sizeof(struct inode));
            fs_get_inode_by_num(0, temp_num, temp_inode);
            int block_offset = oft[fd].fileptr - initial_block_num;
            int temp_block = 0;
            int inode_iterator = 0;
            int partial_block_bits;

            while (nbytes - bytes_written > 0)
            {

                temp_block = temp_inode->blocks[block_iterator];

                if (inode_iterator != 0)
                {
                    if (nbytes - bytes_written <= MDEV_BLOCK_SIZE)
                    {
                        bs_bread(dev0, temp_block, 0, block_cache, nbytes - bytes_written);
                        strncat(buf, block_cache, nbytes - bytes_written);
                        bytes_written = nbytes;
                    }
                    else
                    {
                        bs_bread(dev0, temp_block, 0, block_cache, MDEV_BLOCK_SIZE);
                        strncat(buf, block_cache, MDEV_BLOCK_SIZE);
                        bytes_written = bytes_written + MDEV_BLOCK_SIZE;
                        block_offset = block_offset + MDEV_BLOCK_SIZE;
                    }
                }
                else
                {

                    if (MDEV_BLOCK_SIZE - (oft[fd].fileptr % MDEV_BLOCK_SIZE) >= nbytes - bytes_written)
                    {
                        bs_bread(dev0, temp_block, block_offset, block_cache, nbytes - bytes_written);
                        memcpy(buf, block_cache, partial_block_bits);
                        block_offset = block_offset + nbytes - bytes_written;
                        bytes_written = nbytes;
                    }
                    else
                    {
                        partial_block_bits = MDEV_BLOCK_SIZE - (oft[fd].fileptr % MDEV_BLOCK_SIZE);
                        bs_bread(dev0, temp_block, block_offset, block_cache, partial_block_bits);
                        memcpy(buf, block_cache, partial_block_bits);
                        bytes_written = bytes_written + partial_block_bits;
                        block_offset = block_offset + partial_block_bits;
                    }
                    inode_iterator++;
                }
                inode_iterator++;
                block_iterator++;
            }

            oft[fd].fileptr = bytes_written;
            return oft[fd].fileptr;
        }
        else
        {
            printf("\nFile has write only access");
            return SYSERR;
        }
    }
    else
    {
        printf("\nOpen the file first to write to it\n");
        return SYSERR;
    }
}

int fs_write(int fd, void *buf, int nbytes)
{
    if (oft[fd].state == FSTATE_OPEN)
    {
        if (oft[fd].flag != O_RDONLY)
        {
            int index = oft[fd].in.id;
            struct inode *inode_temp = (struct inode *)getmem(sizeof(struct inode));
            fs_get_inode_by_num(dev0, oft[fd].de->inode_num, inode_temp);
            int num_blocks_needed = ceil(nbytes, MDEV_BLOCK_SIZE);
            int block_offset = nbytes % MDEV_BLOCK_SIZE;
            int block_iterator, inode_iterator;
            char *writing_buffer = getmem(fsd.blocksz);

            for (block_iterator = 0; block_iterator < num_blocks_needed; block_iterator++)
            {
                void *offset = buf + block_iterator * MDEV_BLOCK_SIZE;
                for (inode_iterator = NUM_INODE_BLOCKS + FIRST_INODE_BLOCK; inode_iterator < MDEV_NUM_BLOCKS; inode_iterator++)
                {
                    if (fs_getmaskbit(inode_iterator) == 0)
                    {
                        fs_setmaskbit(inode_iterator);
                        if (block_iterator != num_blocks_needed - 1)
                        {

                            memcpy((void *)writing_buffer, offset, MDEV_BLOCK_SIZE);
                            bs_bwrite(dev0, inode_iterator, 0, writing_buffer, fsd.blocksz);
                            inode_temp->blocks[block_iterator] = inode_iterator;
                        }
                        else
                        {
                            memcpy((void *)writing_buffer, offset, block_offset);
                            bs_bwrite(dev0, inode_iterator, 0, (void *)writing_buffer, fsd.blocksz);
                            inode_temp->blocks[block_iterator] = inode_iterator;
                        }
                        break;
                    }
                }
            }

            fs_put_inode_by_num(dev0, index, inode_temp);

            oft[fd].in = *inode_temp;
            oft[fd].fileptr = nbytes;

            return oft[fd].fileptr;
        }
        else
        {
            printf("\nFile is read only mode, cant write\n");
            return SYSERR;
        }
    }
    else
    {
        printf("\nOpen the file first to write to it\n");
        return SYSERR;
    }
}

int fs_link(char *src_filename, char *dst_filename)
{
    if (next_open_fd == 0)
    {
        printf("The file system doesnt have any open files\n");
        return SYSERR;
    }
    else
    {
        for (int iterator = 0; iterator < next_open_fd; iterator++)
        {
            if (strcmp(oft[iterator].de->name, src_filename) == 0)
            {
                int inode_temp = oft[iterator].de->inode_num;
                fs_get_inode_by_num(dev0, inode_temp, &oft[iterator].in);
                oft[iterator].in.nlink = oft[iterator].in.nlink + 1;
                fs_put_inode_by_num(dev0, inode_temp, &oft[iterator].in);
                oft[next_open_fd].de->inode_num = inode_temp;
                strcpy(oft[next_open_fd].de->name, dst_filename);
                next_open_fd = next_open_fd + 1;
                fsd.root_dir.numentries = fsd.root_dir.numentries + 1;
                return OK;
            }
        }
        printf("The File you tried to link couldnt be found!\n");
        return SYSERR;
    }
}

int fs_unlink(char *filename)
{
    if (next_open_fd == 0)
    {
        printf("No Files exist in the system!");
        return SYSERR;
    }
    else
    {
        for (int iterator = 0; iterator < next_open_fd; iterator++)
        {
            if (strcmp(oft[iterator].de->name, filename) == 0)
            {
                int inode_num = oft[iterator].de->inode_num;
                fs_get_inode_by_num(dev0, inode_num, &oft[iterator].in);
                if (oft[iterator].in.nlink == 1) //deleting if only link left
                {
                    int blk;
                    int maskpointer = oft[iterator].fileptr / MDEV_BLOCK_SIZE;
                    int nbytes = 0;
                    while (nbytes < 1200) //this was flipped
                    {
                        blk = oft[iterator].in.blocks[maskpointer];
                        fs_clearmaskbit(blk);
                        nbytes = nbytes + MDEV_BLOCK_SIZE;
                        maskpointer++;
                    }
                    fsd.inodes_used--;
                }
                else
                {
                    oft[iterator].in.nlink -= 1;
                }
                next_open_fd--;
                fsd.root_dir.numentries--;
                return OK;
            }
        }
        printf("\nThe File you tried to link couldnt be found!\n");
        return SYSERR;
    }
}
// #endif /* FS */