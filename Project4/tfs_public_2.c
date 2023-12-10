
// Abigail Poropatich
// CPSC 3220: Operating Systems
// Project 4: Trivial File System
// 08 December 2023

// This was the hardest thing I've ever done in my life
// I probably put 45 hours into this pls have mercy

#include "tfs.h"
#include <stdbool.h>


/* implementation of assigned functions - skeleton for students */


/* tfs_seek()
 *
 * sets the byte offset in a directory entry
 *
 * preconditions:
 *   (1) the file descriptor is in range
 *   (2) the directory entry is open
 *   (3) the specified offset is less than or equal to the
 *         file size (equal would occur in the case of
 *         writing a new record at the end of a file)
 *
 * postconditions:
 *   (1) the byte offset of the directory entry is set to the
 *         specified offset
 *   (2) the current block of the directory entry is set to the
 *         corresponding block (in the case of the offset being
 *         equal to the file size and just beyond the end of
 *         the last block of the file, the current block will
 *         be set to LAST_BLOCK)
 *
 * input parameters are a file descriptor and a byte offset
 *
 */

unsigned int tfs_seek(unsigned int file_descriptor, unsigned int offset) {
    unsigned int seek_block;
    // Checks for range validity, particularly for offset, cannot be greater than the file size
    if (!tfs_is_fd_in_range(file_descriptor)) { return FALSE; }
    if (!tfs_is_fd_open(file_descriptor)) { return FALSE; }
    if (offset > directory[file_descriptor].size) { return FALSE; }

    // Find the block that the offset is in
    directory[file_descriptor].byte_offset = offset;
    for (seek_block = directory[file_descriptor].first_block; offset >= BLOCK_SIZE && seek_block != LAST_BLOCK; offset -= BLOCK_SIZE, seek_block = file_allocation_table[seek_block]) {}
    directory[file_descriptor].current_block = (seek_block == LAST_BLOCK && offset != 0) ? LAST_BLOCK : seek_block;

    return TRUE; 
}


/* tfs_delete()
 *
 * deletes a closed directory entry having the given file descriptor
 *   (changes the status of the entry to unused) and releases all
 *   allocated storage blocks
 *
 * preconditions:
 *   (1) the file descriptor is in range
 *   (2) the directory entry is closed
 *
 * postconditions:
 *   (1) the status of the directory entry is set to unused
           and all other fields are set to zero
 *   (2) all storage blocks have been set to free
 *
 * input parameter is a file descriptor
 *
 */
void nice_little_file_reset(unsigned int fd) {
    directory[fd].status = UNUSED;
    directory[fd].first_block = FREE;
    directory[fd].size = 0;
    directory[fd].byte_offset = 0;
    directory[fd].current_block = FREE;
    directory[fd].name[0] = '\0';
}

unsigned int tfs_delete(unsigned int file_descriptor) {
    // Check for range validity AND return if encountering an unused file descriptor
    if (!tfs_is_fd_in_range(file_descriptor)) { return FALSE; }
    if (directory[file_descriptor].status == UNUSED) { return FALSE; }
    // Close the file before deleting it
    if (directory[file_descriptor].status == OPEN) { tfs_close(file_descriptor); }

    unsigned int delete_block = directory[file_descriptor].first_block;
    while (delete_block != LAST_BLOCK && delete_block != FREE) {
      unsigned int next_block = file_allocation_table[delete_block ];
      // free up the block
      file_allocation_table[delete_block] = FREE;  
      delete_block  = next_block;
    }

    // Cute little encapsulating function, having fun with it
    nice_little_file_reset(file_descriptor);

    return TRUE; 
}


/* tfs_write()
 *
 *     ---- students: DO NOT access the storage[] array or ----
 *     ---- the blocks[] array directly in this function;  ----
 *     ---- use tfs_block_read() and tfs_block_write() to  ----
 *     ---- to access the blocks in the storage[] array    ----
 *
 * writes a specified number of bytes from a specified buffer
 *   into a file starting at the byte offset in the directory;
 *   the byte offset in the directory entry is incremented by
 *   the number of bytes transferred
 *
 * depending on the starting byte offset and the specified
 *   number of bytes to transfer, the transfer may cross two
 *   or more storage blocks
 *
 * this function does not directly access bytes in storage;
 *   instead, each block is first read into an internal
 *   buffer using the helper function tfs_read_block();
 *   this function then moves bytes from the user buffer
 *   to the internal buffer and then writes each updated
 *   block using the helper function tfs_write_block()
 *
 * the read-modify-write approach is used since a write
 *   may only change part of the content of a block and
 *   the bytes at the unaffected start and or end of a
 *   block must be preserved (a possible optimization is
 *   to omit a read when all bytes of a block will be
 *   overwritten; however, that optimization is not
 *   required, and students should aim for correctness
 *   over eliminating block reads)
 *
 * note that this function can extend the file size by
 *   writing bytes beyond the current end of the file;
 *   this depends on:
 *   (1) the starting byte offset
 *   (2) the specified number of bytes to transfer
 *   (3) the availability of additional storage blocks
 *   (4) the maximum file size
 *
 * extra bytes may fit into the last allocated storage
 *   block, or additional storage blocks, if available,
 *   may be added to the file to hold the extra bytes
 *
 * notable special cases that this function should handle:
 *   - writing to an empty file, which has no storage
 *       blocks allocated (current block will be 0)
 *   - writing past the end of a file that originally
 *       ended at the end of a storage block (current
 *       block will be equal to LAST_BLOCK)
 *   - writing that ends at the end of a storage block
 *       (current block must be set to the correct
 *       value so that a subsequent read or write is
 *       handled correctly)
 *
 * preconditions:
 *   (1) the file descriptor is in range
 *   (2) the directory entry is open
 *   (3) the requested block count is non-zero
 *
 * postconditions:
 *   (1) the file contains bytes transferred from the user
 *         buffer to the extent possible
 *   (2) the specified number of bytes has been transferred
 *         except in the cases that
 *       (a) storage blocks needed to complete the transfer
 *             are not available
 *       (b) bytes to be written would exceed the maximum
 *             file size
 *       (c) a call to tfs_block_write() fails
 *   (3) the byte offset and current block are updated to
 *         point to the next byte to read or write
 *   (4) the file size has been updated if bytes are added
 *         past the original end of the file
 *
 * input parameters are a file descriptor, the address of a
 *   buffer of bytes to transfer, and the count of bytes to
 *   transfer
 *
 * return value is the number of bytes transferred
 */


// Helper function to write to the block buffer

void update_the_offset(unsigned int file_descriptor, unsigned int written, unsigned char current_block) {
    unsigned int new_offset = directory[file_descriptor].byte_offset + written;
    if (new_offset > directory[file_descriptor].size) {
        directory[file_descriptor].size = new_offset;
    }
    directory[file_descriptor].byte_offset = new_offset;
    directory[file_descriptor].current_block = current_block;
}

unsigned int tfs_write(unsigned int file_descriptor, char *buffer, unsigned int byte_count) {
    // File descriptor validity check
    if (tfs_is_fd_in_range(file_descriptor) ==  FALSE || tfs_is_fd_open(file_descriptor) == FALSE) {
        return FALSE;
    }

    // initiallizing with the first block and inital offset
    unsigned char block = directory[file_descriptor].first_block,
                  current_block;
    unsigned int written = 0,
                 offset1 = directory[file_descriptor].byte_offset;

    // New block driver
    if (block == FREE) {
        current_block = tfs_new_block();
        if (current_block == LAST_BLOCK) { return -1; }
        else{
          // Update to newly allocated block
          file_allocation_table[current_block] = LAST_BLOCK;
          directory[file_descriptor].first_block = current_block;
        }
    } 
    // Existing block driver
    else {
        // If the block isn't free then find the current block
        current_block = block;
        while (offset1 >= BLOCK_SIZE) {
            offset1 -= BLOCK_SIZE;
            // Allocate a new block only in the end of the FAT chain is reached
            if (file_allocation_table[current_block] == LAST_BLOCK) {
                unsigned char new_block = tfs_new_block();
                if (new_block == LAST_BLOCK) { return -1; }
                file_allocation_table[current_block] = new_block;
                file_allocation_table[new_block] = LAST_BLOCK;
            }
            current_block = file_allocation_table[current_block];
        }
    }

    // Relaying data to the blocks
    while (written < byte_count) {
        unsigned char FAT_offset = offset1 % BLOCK_SIZE;
        unsigned int space_in_block = BLOCK_SIZE - FAT_offset,
                     to_write = (byte_count - written) < space_in_block ? (byte_count - written) : space_in_block;
        char character_buff[BLOCK_SIZE];
        tfs_block_read(current_block, character_buff);

        // Memory copy
        for (int i = 0; i < to_write; i++) { character_buff[FAT_offset + i] = buffer[written + i]; }
        tfs_block_write(current_block, character_buff);

        // Update the written and offset counts
        written += to_write;
        offset1 += to_write;

        // Logic for reading mid write, transition to next block iff need be
        if (offset1 >= BLOCK_SIZE) {
          offset1 -= BLOCK_SIZE;
          // Ensure that there is a next block
          if (written < byte_count) {
              if (file_allocation_table[current_block] == LAST_BLOCK) {
                  unsigned char new_block = tfs_new_block();
                  if (new_block == LAST_BLOCK) { break; }
                  file_allocation_table[current_block] = new_block;
                  file_allocation_table[new_block] = LAST_BLOCK;
              }
          }
          current_block = file_allocation_table[current_block];
        }        
    }

    // Metadata update
    update_the_offset(file_descriptor, written, current_block);

    return written;
}


/* tfs_copy()
 *
 *     ---- students: DO NOT access the storage[] array or ----
 *     ---- the blocks[] array directly in this function;  ----
 *     ---- use tfs_block_read() and tfs_block_write() or  ----
 *     ---- tfs_read() and tfs_write() to access storage   ----
 *
 * copies one file to another
 *
 * note that if the destination file was originally larger
 *   than the source file then any unneeded storage blocks
 *   must be freed; one possible way to accomplish this is
 *   to delete an existing destination file and create it
 *   again as an empty file
 *
 * preconditions:
 *   (1) the source file exists, has size greater than 0,
 *         and is not currently open
 *   (2) the destination file either
 *       (a) does not exist yet and name is valid, or
 *       (b) does exist and is not currently open
 *
 * postcondition:
 *   (1) the source file is unchanged and is closed
 *   (1) the destination file exists, is a copy of the
 *         source file, and is closed except in these cases:
 *       (a) a source file precondition was not met - result
 *             is that either there is no destination file
 *             or the destination file remains unchanged
 *       (b) the destination file does not exist and the
 *             name is invalid - result is that there is no
 *             destination file
 *       (c) there were not enough directory entries
 *             available to create a new file - the result
 *             is that there is no destination file
 *       (d) there were not enough storage blocks available
 *             to completely copy the source file - the
 *             result is an incomplete copy
 *       (e) there were failures in tfs_read() and/or
 *             tfs_write() - the result is an incomplete
 *             copy
 *
 * input parameters are two file names
 *
 * return value is the number of bytes copied (0 in error cases)
 */

unsigned int tfs_copy(char *from_name, char *to_name) {
    // Mapping the file names to file descriptors
    unsigned int from_fd = tfs_map_name_to_fd(from_name),
                 to_fd = tfs_map_name_to_fd(to_name);

    // Normal function error checking, particularly validating from the source file
    if (from_fd == 0 || directory[from_fd].size == 0 || directory[from_fd].status != CLOSED) { return FALSE; }
    if (to_fd != 0 && directory[to_fd].status != CLOSED) { return FALSE; }
    if (to_fd != 0) { tfs_delete(to_fd); }

    // Creating and opening the dest file
    to_fd = tfs_create(to_name);
    if (to_fd == 0 || from_fd == 0) { return FALSE; }
    from_fd = tfs_open(from_name);


    char buffer[BLOCK_SIZE];
    unsigned int copied = 0,
                 read, write;

    // Read from the source file and write to the destination file
    for (;(read = tfs_read(from_fd, buffer, BLOCK_SIZE)) > 0;) {
      write = tfs_write(to_fd, buffer, read);
      if (write != read) {
          tfs_delete(to_fd); 
          tfs_close(from_fd);
          return 0;
      }
      copied += write;
    }

    // Close both files after done copying
    tfs_close(from_fd);
    tfs_close(to_fd);

    return copied;
}