/* CPSC 3220 Fall 2023
 *
 * trivial file system (tfs)
 *
 * - the directory is a single-level table
 * - storage blocks are mapped with a file allocation table
 * - a file can only have one open at a time => the current byte
 *     offset (i.e., the file pointer) can be placed in the
 *     directory entry instead of a per-open data structure
 * - there are no file permissions and no permission checking
 * - file name aliases are not supported
 *
 * - for this file system, we define 256 storage blocks of 128
 *     bytes each
 * - the storage blocks are assigned as:
 *     0 - 1:   directory, 16 entries of 16 bytes each
 *     2 - 3:   file allocation table, 256 entries of 1 byte each
 *     4 - 255: storage blocks containing file data; note that a
 *                storage block is assigned to an individual file
 *                and cannot be shared between files
 *
 * - the directory would normally occupy the first two storage
 *     blocks, although in this program we keep the directory as
 *     an in-memory table
 * - a directory entry is 16 bytes (with 9 bytes for the name string)
 *
 *     +--------+--------+--------+--------+--------+---------...-+
 *     | status | first_ |  size  |  byte_ | current|  name       |
 *     |        | block  |        | offset | _block |             |
 *     +--------+--------+--------+--------+--------+---------...-+
 *       1 byte   1 byte   2 bytes  2 bytes  1 byte    9 bytes
 *
 * - the status is encoded as: 0 = unused, 1 = closed, 2 = open
 * - the first storage block of the file, if allocated; zero
 *     means that no storage blocks are currently allocated to the
 *     file; otherwise this is the (0-orgin) index of a storage
 *     block
 * - the size can range from 0 to MAX_FILE_SIZE in bytes; note
 *     that the size does not have to be a multiple of the storage
 *     block size; in the cases when it is not, the last block of
 *     the file will have unused bytes (i.e., it will have internal
 *     fragmentation)
 * - the byte offset is the current file pointer within the file,
 *     which is the (0-origin) index within the length of the
 *     file of the next byte to read or write (note that this
 *     value mod the block length is the byte index within the
 *     current block of that next byte)
 * - the current storage block is the (0-origin) index of the
 *     storage block that contains the byte indexed by the byte
 *     offset; note that this value is not the same as the high
 *     bits of the byte offset; note that this value is 0 when
 *     the first storage block index is 0 and is 1 when the byte
 *     offset points just beyond the last block of the file
 *
 *     example of possible layout of a file using 4-byte blocks
 *       and several non-sequential storage blocks
 *
 *                        +------------------+
 *       bytes of file    |abcdefghijklmnopqr|  file size = 18
 *                        +------------------+
 *                                  ^
 *                                  `. byte offset = 9
 *
 *                        first second   ...   last
 *       bytes divided    +----+----+----+----+----+ internal
 *         into logical   |abcd|efgh|ijkl|mnop|qr..| fragmentation
 *         blocks         +----+----+----+----+----+ in last block
 *                                    ^
 *                                    `. byte offset = 9
 *
 *                         129  130      133      136      138
 *                        +----+----+   +----+   +----+   +----+
 *       physical blocks  |ijkl|mnop|   |qr..|   |abcd|   |efgh|
 *         in storage     +----+----+   +----+   +----+   +----+
 *                        third fourth   last     first    second
 *                          ^
 *                          `. byte offset mod 4 = 1
 *
 *       directory  first storage block of file is 136
 *         entry    size of file is 18
 *                  byte offset of file is 9
 *                  current storage block of file is 129
 *
 *             128 129 130 131 132 133 134 135 136 137 138 139
 *            +---+---+---+---+---+---+---+---+---+---+---+---+---
 *       FAT  | _ |130|133| _ | _ | 1 | _ | _ |138| _ |129| _ |...
 *            +---+---+---+---+---+---+---+---+---+---+---+---+---
 *                                  ^
 *                                  `. value of 1 marks last block
 *
 * - the name is a character string up to 8 characters in length
 *     (a 9th byte is provided for the null byte); the name can
 *     contain alphanumeric characters, underscores, and periods;
 *     there are no additional rules for the naming syntax
 *
 * - file descriptors are used as (0-origin) indices into the
 *     directory
 * - a file descriptor has a valid range of 1-15 since in many
 *     cases a return value of 0 indicates an error
 *
 * - the file allocation table would normally occupy the second
 *     two storage blocks, although in this program we keep the
 *     file allocation table as an in-memory table
 * - a storage block number for a file has a valid range of
 *     4-255; a block number of 0 indicates a free block, and
 *     a block number of 1 indicates the last block of a file
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


/* defined sizes and limits */

#define N_DIRECTORY_ENTRIES 16
#define N_BLOCKS 256
#define BLOCK_SIZE 128
#define BLOCK_SIZE_AS_POWER_OF_2 7
#define STORAGE_SIZE (256*128)
#define MAX_FILE_SIZE (252*128)
#define FILENAME_LENGTH 8
#define FIRST_VALID_FD 1
#define LAST_VALID_FD 15
#define FIRST_VALID_BLOCK 4
#define LAST_VALID_BLOCK 255


/* directory entry status */

#define UNUSED 0
#define CLOSED 1
#define OPEN 2


/* special case block index values */

#define FREE 0
#define LAST_BLOCK 1


/* logical values */

#define TRUE 1
#define FALSE 0


/* storage for file system */

char storage[STORAGE_SIZE];


/* struct declarations and pointers */

struct storage_block{
  char bytes[BLOCK_SIZE];
};

struct directory_entry{
  unsigned char status;
  unsigned char first_block;
  unsigned short size;
  unsigned short byte_offset;
  unsigned char current_block;
  char name[FILENAME_LENGTH + 1];
};

struct storage_block *blocks;
struct directory_entry *directory;
unsigned char *file_allocation_table;


/* public interface */

void tfs_init();
void tfs_list_blocks();
void tfs_list_directory();
unsigned int tfs_create( char *name );
unsigned int tfs_exists( char *name );
unsigned int tfs_copy(   char *from_name, char *to_name );
unsigned int tfs_open(   char *name );
unsigned int tfs_size(   unsigned int file_descriptor );

unsigned int tfs_seek(   unsigned int file_descriptor,
                         unsigned int offset );

unsigned int tfs_read(   unsigned int file_descriptor,
                         char *user_buffer,
                         unsigned int byte_count );

unsigned int tfs_write(  unsigned int file_descriptor,
                         char *user_buffer,
                         unsigned int byte_count );

unsigned int tfs_close(  unsigned int file_descriptor );
unsigned int tfs_delete( unsigned int file_descriptor );


/* helper functions */

unsigned int tfs_is_fd_in_range(    unsigned int fd );
unsigned int tfs_is_block_in_range( unsigned int b );
unsigned int tfs_is_fd_open(        unsigned int fd );
unsigned int tfs_is_valid_name(     char *name );

unsigned int tfs_map_name_to_fd( char *name );

unsigned int tfs_new_directory_entry();
unsigned int tfs_new_block();

unsigned int tfs_block_read(  unsigned int b, char *buf );
unsigned int tfs_block_write( unsigned int b, char *buf );

