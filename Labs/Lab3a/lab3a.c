// Name: Hermmy Wamg
// UID: 704978214
// Email: hermmyw@hotmail.com

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>
#include <time.h>
#include "ext2_fs.h"

/* global variables */
int fd = 0;
int bcount = 0;
int icount = 0;
int bsize = 0;
int isize = 0;
int ipergroup = 0;
__u32 fbcount = 0;
__u32 ficount = 0;
__u32 fb_bitmap = 0;
__u32 fi_bitmap = 0;
__u32 itable = 0;

/* calculate the block offset given the block id */
int blk_offset(int blk_id) {return 1024 + (blk_id-1)*bsize;}

void _error(const char* str) {
    fprintf(stderr, "%s\n", str);
    close(fd);
    exit(2);
}

void superblock() {

    /* superblock has a size of 1024 at offset 1024 */
    struct ext2_super_block s;
    if (pread(fd, &s, 1024, 1024) < 0) 
        _error("pread() fails\n");

    if (s.s_magic != 0xEF53)
        _error("Magic number can't match\n");

    bcount = s.s_blocks_count;
    icount = s.s_inodes_count;
    /* bsize = EXT2_MIN_BLOCK_SIZE << s_log_block_size */
    bsize =  EXT2_MIN_BLOCK_SIZE << s.s_log_block_size;
    isize = s.s_inode_size;
    ipergroup = s.s_inodes_per_group;

    fprintf(stdout, "%s,%d,%d,%d,%d,%d,%d,%d\n", "SUPERBLOCK",
                                                 bcount,icount,bsize,isize,
                                                 s.s_blocks_per_group,
                                                 ipergroup,
                                                 s.s_first_ino);
}


void group() {
    struct ext2_group_desc g;
    /* group descriptor is located after superblock */
    if (pread(fd, &g, sizeof(struct ext2_group_desc), 1024+bsize) < 0)
        _error("pread() fails\n");

    fbcount = g.bg_free_blocks_count;
    ficount = g.bg_free_inodes_count;
    fb_bitmap = g.bg_block_bitmap;
    fi_bitmap = g.bg_inode_bitmap;
    itable = g.bg_inode_table;

    /* Produce a line for the single group with nine fields */
    fprintf(stdout, "%s,%d,%d,%d,%d,%d,%d,%d,%d\n", "GROUP", 0, bcount, icount,
                                                    g.bg_free_blocks_count, 
                                                    g.bg_free_inodes_count, 
                                                    fb_bitmap, fi_bitmap, itable); 
}

void bfree() {
    /* Scan the free block bitmap */
    /* bsize is represented in bytes and each bit represents a block */
    /* there are in total bcount/8 bytes */
    for (int i = 0; i < bcount/8; i++) {
        char bitmap_onebyte;
        if (pread(fd, &bitmap_onebyte, 1, fb_bitmap*bsize+i) < 0)
            _error("pread() fails\n");
        /* 1 byte = 8 bits */
        for (int j = 0; j < 8; j++) {
            /* 1 means "used" and 0 "free/available" */
            if ((bitmap_onebyte & (1 << j)) == 0)
                fprintf(stdout, "%s,%d\n", "BFREE", i*8+j+1);
        }

    }
    
}

void ifree() {
    /* Scan the free inode bitmap, similar to block bitmap */
    for (int i = 0; i < bcount/8; i++) {
        char bitmap_onebyte;
        if (pread(fd, &bitmap_onebyte, 1, fi_bitmap*bsize+i) < 0)
            _error("pread() fails\n");
        for (int j = 0; j < 8; j++) {
            if ((bitmap_onebyte & (1 << j)) == 0)
                fprintf(stdout, "%s,%d\n", "IFREE", i*8+j+1);
        }

    }
}


void inode() {

    struct ext2_inode inod;
    for (int i = 0; i < ipergroup; i++) {
        if (pread(fd, &inod, sizeof(struct ext2_inode), itable*bsize+i*isize) < 0)
            _error("pread() fails\n");
        if (inod.i_links_count == 0 || inod.i_mode == 0)
            continue;
        /* file type */
        char file_type = '?';
        if ((inod.i_mode & 0xF000) == 0x8000) file_type = 'f';
        if ((inod.i_mode & 0xF000) == 0xA000) file_type = 's';
        if ((inod.i_mode & 0xF000) == 0x4000) file_type = 'd';

        /* format time */
        time_t ctime = (time_t) inod.i_ctime;
        time_t mtime = (time_t) inod.i_mtime;
        time_t atime = (time_t) inod.i_atime;
        char ctimebuf[50];
        char mtimebuf[50];
        char atimebuf[50];
        strftime (ctimebuf,50,"%D %T",gmtime(&ctime));
        strftime (mtimebuf,50,"%D %T",gmtime(&mtime));
        strftime (atimebuf,50,"%D %T",gmtime(&atime));

        /* print information */
        fprintf(stdout, "%s,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d,", 
                "INODE", i+1, file_type, inod.i_mode&0xFFF, inod.i_uid, inod.i_gid, inod.i_links_count,
                ctimebuf, mtimebuf, atimebuf, inod.i_size, inod.i_blocks);

        /* print block pointers */
        for (int j = 0; j < 14; j++) {
            fprintf(stdout, "%d,", inod.i_block[j]);
        }
        if ((file_type == 'd' || file_type == 'f'))  //?? If the file length is greater than 60 bytes, print out the fifteen block numbers
            fprintf(stdout, "%d\n", inod.i_block[14]);
        else if (inod.i_size <= 60)   // Symbolic link and contains <= 60 bytes
            fprintf(stdout, "\n");
    }

}

/* Directories are stored as data block and referenced by an inode. */
void dirEnt() {
    struct ext2_inode inod;
    for (int i = 0; i < ipergroup; i++) {
        if (pread(fd, &inod, sizeof(struct ext2_inode), itable*bsize+i*isize) < 0) // parent inode
            _error("pread() fails\n");  
        if (inod.i_links_count == 0 || inod.i_mode == 0 || (inod.i_mode & 0xF000) != 0x4000)
            continue;

        /* Scan every data block */
        for (int j = 0; j < 12; j++) {
            struct ext2_dir_entry d;
            int blkptr = inod.i_block[j];
            if (inod.i_block[j] == 0)
                continue;

            // fprintf(stdout, "%s\n", "Enter dirEnt"); //140
            /* Each directory inode is a linked list of directory entries of block size */
            // k = offset 
            for (int k = 0; k < bsize; ) {
                if (pread(fd, &d, sizeof(struct ext2_dir_entry), k+blk_offset(blkptr)) < 0)
                    _error("pread() fails\n"); 
                
                if (d.inode == 0){
                    k += d.rec_len;
                    continue;
                }
                char d_name[EXT2_NAME_LEN+1];
                memcpy(d_name, d.name, d.name_len);
                d_name[d.name_len] = '\0';
                fprintf(stdout, "%s,%d,%d,%d,%d,%d,\'%s\'\n",
                "DIRENT", i+1, k, d.inode, d.rec_len, d.name_len, d_name);

                k += d.rec_len;

            }

        }



    }

}

void indir_rec(int inode, int level, int log_offset, int blk_id, int isDirectory) {
    if (level == 0)
        return; 
    int referenced_blk[bsize/4];  // contain a block of block_ids
                                  // eg. a block of 1024B stores 256 ids.

    for (int i = 0; i < bsize/4; i++) {

        /* read each block pointer in the indirect blocks */
        if (pread(fd, &referenced_blk[i], 4, blk_offset(blk_id)+4*i) < 0)
            _error("pread() fails\n"); 
        if (referenced_blk[i] != 0) {

            /* print information */
            fprintf(stdout, "%s,%d,%d,%d,%d,%d\n", "INDIRECT", inode, level, 
                    log_offset, blk_id, referenced_blk[i]);

            /* print information for directory entries */
            if (isDirectory) {
                struct ext2_dir_entry d;
                for (int k = 0; k < bsize; ) {
                    if (pread(fd, &d, sizeof(struct ext2_dir_entry), blk_offset(referenced_blk[i])+k) < 0)
                        _error("pread() fails\n"); 
                    
                    if (d.inode == 0){
                        k += d.rec_len;
                        continue;
                    }
                    char d_name[EXT2_NAME_LEN+1];
                    memcpy(d_name, d.name, d.name_len);
                    d_name[d.name_len] = '\0';
                    fprintf(stdout, "%s,%d,%d,%d,%d,%d,\'%s\'\n",
                        "DIRENT", inode, blk_offset(referenced_blk[i])+k, d.inode, d.rec_len, d.name_len, d_name);

                    k += d.rec_len;
                }
            }



            /* recursion */
            if (level > 1) {
                indir_rec(inode, level-1, log_offset, referenced_blk[i], isDirectory);
                if (level == 2) log_offset += bsize/4;
                if (level == 3) log_offset += bsize/4*bsize/4;
            }
        }
        log_offset++;
    }
}

void indirect() {
    struct ext2_inode inod;
    for (int i = 0; i < ipergroup; i++) {
        if (pread(fd, &inod, sizeof(struct ext2_inode), itable*bsize+i*isize) < 0)  // parent inode
            _error("pread() fails\n"); 
        if (inod.i_links_count == 0 || inod.i_mode == 0)
            continue;
        if ((inod.i_mode & 0xF000) != 0x4000 && (inod.i_mode & 0xF000) != 0x8000) // file or directory inode
            continue;

        int isDirectory = 0;
        if ((inod.i_mode & 0xF000) == 0x4000)
            isDirectory = 1;

        /* scan the 12~15th blocks */
        for (int j = 12; j < 15; j++) {  
            // A value of 0 in this array effectively terminates it
            if (inod.i_block[j] == 0)
                break;

            if (j==12) indir_rec(i+1, 1, 12, inod.i_block[j], isDirectory);// first level
            if (j==13) indir_rec(i+1, 2, 12+bsize/4, inod.i_block[j], isDirectory);// second level
            if (j==14) indir_rec(i+1, 3, 12+bsize/4+bsize/4*bsize/4, inod.i_block[j], isDirectory);// third level

        }

    }

}



int main(int argc, char const *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments\n");
        close(fd);
        exit(1);
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Can't open file image\n");
        close(fd);
        exit(1);
    }

    superblock();
    group();
    bfree();
    ifree();
    inode();
    dirEnt();
    indirect();

    close(fd);

    return 0;
}