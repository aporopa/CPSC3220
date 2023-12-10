#include "tfs.h"

/* implementation of public functions - instructor supplied
 *
 * nine public functions in this source file
 * - call log message prefix is log_p_i for i-th function
 * - error log message prefix is err_p_i for i-th function
 *     or err_h_i.j for j-th error case within i-th function
 * - each type of logging is controlled by a defined value
 *     using 0 = silent and 1 = logging
 *
 * notes
 * - you can redirect stderr using "2>" on the command line
 * - you might want to change the logging scheme to use
 *     conditional compilation using #ifdef and #endif
 * - you might want to replace some of the precondition
 *     testing and error logging with asserts
 */

#define CALL_LOGGING 0
#define ERROR_LOGGING 0



/* tfs_init()
 *
 * initializes the directory as empty and the file allocation table
 *   to have all blocks free
 *
 * no parameters
 *
 * no return value
 */

void tfs_init(){
  unsigned int i;

  if( CALL_LOGGING ){
    fprintf( stderr, "log_p_1: init() called\n" );
  }

  directory = (struct directory_entry *) storage;
  blocks = (struct storage_block *) storage;
  file_allocation_table = (unsigned char *) storage + 2*BLOCK_SIZE;

  for( i = 0; i < STORAGE_SIZE; i++ ){
    storage[i] = 0;
  }
}


/* tfs_list_blocks()
 *
 * list file blocks that are being used and next block values
 *   from the file allocation table
 *
 * no parameters
 *
 * no return value
 */

void tfs_list_blocks(){
  unsigned int b;

  if( CALL_LOGGING ){
    fprintf( stderr, "log_p_2: list_blocks() called\n" );
  }

  printf( "-- file alllocation table listing of used blocks --\n" );

  for( b = FIRST_VALID_BLOCK; b < N_BLOCKS; b++ ){
    if( file_allocation_table[b] != FREE ){
      printf( "  block %3d is used and points to %3d\n",
        b, file_allocation_table[b] );
    }
  }
  printf( "-- end --\n" );
}


/* tfs_list_directory()
 *
 * list all directory entries
 *
 * no parameters
 *
 * no return value
 */

void tfs_list_directory(){
  unsigned int fd, more_to_print, fd2;
  unsigned char b;

  if( CALL_LOGGING ){
    fprintf( stderr, "log_p_3: list_directory() called\n" );
  }

  printf( "-- directory listing --\n" );

  for( fd = FIRST_VALID_FD; fd < N_DIRECTORY_ENTRIES; fd++ ){
    printf( "  fd = %2d: ", fd );
    if( directory[fd].status == UNUSED ){
      more_to_print = 0;
      for( fd2 = fd; fd2 < N_DIRECTORY_ENTRIES; fd2++ ){
        if( directory[fd2].status != UNUSED ) more_to_print = 1;
      }
      if( more_to_print || ( fd == LAST_VALID_FD ) ){
        printf( "unused\n" );
      }else{
        printf( "this entry and the following entries are unused\n" );
        printf( "-- end --\n" );
        return;
      }
    }else if( directory[fd].status == CLOSED ){
      printf( "%s, currently closed, %d bytes in size\n",
        directory[fd].name, directory[fd].size );
    }else if( directory[fd].status == OPEN ){
      printf( "%s, currently open, %d bytes in size\n",
        directory[fd].name, directory[fd].size );
    }else{
      if( ERROR_LOGGING ){
        fprintf( stderr, "err_p_3: invalid file status for fd %d: %d\n",
          fd, directory[fd].status );
      }
    }

    if( ( directory[fd].status == CLOSED ) ||
        ( directory[fd].status == OPEN ) ){
      printf( "           FAT:" );
      if( directory[fd].first_block == 0 ){
        printf( " no blocks in use\n" );
      }else{
        b = directory[fd].first_block;
        while( b != LAST_BLOCK ){
          printf( " %d", b );
          b = file_allocation_table[b];
        }
        printf( "\n" );
      }
    }
  }
  printf( "-- end --\n" );
}


/* tfs_exists()
 *
 * return TRUE if a file name is associated with an active
 *   directory entry
 *
 * preconditions:
 *   (1) the name is valid
 *   (2) the name is associated with an active directory entry
 *
 * postconditions:
 *   there are no changes to the file data structures
 *
 * input parameter is file name
 *
 * return value is TRUE or FALSE
 */

unsigned int tfs_exists( char *name ){

  if( CALL_LOGGING ){
    fprintf( stderr, "log_p_4: exists() called with: %s\n", name );
  }

  /* precondition checks */
  if( !tfs_is_valid_name( name ) ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_4.1: invalid file name: %s\n", name );
    }
    return( FALSE );
  }
  if( tfs_map_name_to_fd( name ) == 0 ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_4.2: unable to map file name: %s\n", name );
    }
    return( FALSE );
  }

  return( TRUE );
}


/* tfs_create()
 *
 * create a new directory entry with the given file name and
 *   set the status to open, the first block to invalid, the
 *   size to 0, the byte offset to 0, and the current block
 *   to invalid
 *
 * preconditions:
 *   (1) the name is valid
 *   (2) the name is not already associated with any active
 *         directory entry
 *   (3) an unused directory entry is available
 *
 * postconditions:
 *   (1) a new directory entry overwrites an unused entry
 *   (2) the new entry is appropriately initialized
 *
 * input parameter is file name
 *
 * return value is the file descriptor of a directory entry
 *   when successful or 0 when failure
 */

unsigned int tfs_create( char *name ){
  unsigned int file_descriptor;

  if( CALL_LOGGING ){
    fprintf( stderr, "log_p_5: create() called with: %s\n", name );
  }

  /* precondition checks */
  if( !tfs_is_valid_name( name ) ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_5.1: invalid file name: %s\n", name );
    }
    return( 0 );
  }
  if( tfs_map_name_to_fd( name ) != 0 ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_5.2: attempt to create existing file: %s\n",
        name );
    }
    return( 0 );
  }
  if( ( file_descriptor = tfs_new_directory_entry() ) == 0 ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_5.3: directory entry not available\n" );
    }
    return( 0 );
  }

  directory[file_descriptor].status = OPEN;
  directory[file_descriptor].first_block = 0;
  directory[file_descriptor].size = 0;
  directory[file_descriptor].byte_offset = 0;
  directory[file_descriptor].current_block = 0;
  strcpy( directory[file_descriptor].name, name );

  return( file_descriptor );
}


/* tfs_open()
 *
 * opens the directory entry having the given file name and
 *   sets the status to open, the byte offset to 0, and the
 *   current block to the first block
 *
 * preconditions:
 *   (1) the name is valid
 *   (2) the name is associated with an active directory entry
 *   (3) the directory entry is not already open
 *
 * postconditions:
 *   (1) the status of the directory entry is set to open
 *   (2) the byte offset of the directory entry is set to 0
 *   (3) the current block is set to the first block
 *
 * input parameter is file name
 *
 * return value is the file descriptor of a directory entry
 *   when successful or 0 when failure
 */

unsigned int tfs_open( char *name ){
  unsigned int file_descriptor;

  if( CALL_LOGGING ){
    fprintf( stderr, "log_p_6: open() called with: %s\n", name );
  }

  /* precondition checks */
  if( !tfs_is_valid_name( name ) ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_6.1: invalid file name: %s\n", name );
    }
    return( 0 );
  }
  if( ( file_descriptor = tfs_map_name_to_fd( name ) ) == 0 ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_6.2: unable to map file name: %s\n", name );
    }
    return( 0 );
  }
  if( tfs_is_fd_open( file_descriptor ) ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_6.3: attempt to open an open file: %d\n",
        file_descriptor );
    }
    return( 0 );
  }

  directory[file_descriptor].status = OPEN;
  directory[file_descriptor].byte_offset = 0;
  directory[file_descriptor].current_block =
    directory[file_descriptor].first_block;

  return( file_descriptor );
}


/* tfs_close()
 *
 * closes the directory entry having the given file descriptor
 *   (sets the status to closed, the byte offset to 0, and the
 *   current block to the first block); the function fails if
 *   (1) the file descriptor is out of range,
 *   (2) the file descriptor is within range but the directory
 *         entry is not open
 *
 * preconditions:
 *   (1) the file descriptor is in range
 *   (2) the directory entry is open
 *
 * postconditions:
 *   (1) the status of the directory entry is set to closed
 *   (2) the byte offset of the directory entry is set to 0
 *   (3) the current block is set to the first block
 *
 * input parameter is a file descriptor
 *
 * return value is TRUE when successful or FALSE when failure
 */

unsigned int tfs_close( unsigned int file_descriptor ){

  if( CALL_LOGGING ){
    fprintf( stderr, "log_p_7: close() called with: %d\n", file_descriptor );
  }

  /* precondition checks */
  if( !tfs_is_fd_in_range( file_descriptor ) ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_7.1: file descriptor out of range: %d\n",
        file_descriptor );
    }
    return( FALSE );
  }
  if( !tfs_is_fd_open( file_descriptor ) ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_7.2: attempt to close a closed file: %d\n",
        file_descriptor );
    }
    return( FALSE );
  }

  directory[file_descriptor].status = CLOSED;
  directory[file_descriptor].byte_offset = 0;
  directory[file_descriptor].current_block =
    directory[file_descriptor].first_block;

  return( TRUE );
}


/* tfs_size()
 *
 * returns the file size for an active directory entry
 *
 * preconditions:
 *   (1) the file descriptor is in range
 *   (2) the directory entry is active
 *
 * postconditions:
 *   there are no changes to the file data structures
 *
 * input parameter is a file descriptor
 *
 * return value is the file size when successful or MAX_FILE_SIZE+1
 *   when failure
 */

unsigned int tfs_size( unsigned int file_descriptor ){

  if( CALL_LOGGING ){
    fprintf( stderr, "log_p_8: size() called with: %d\n", file_descriptor );
  }

  /* precondition checks */
  if( !tfs_is_fd_in_range( file_descriptor ) ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_8.1: file descriptor out of range: %d\n",
        file_descriptor );
    }
    return( MAX_FILE_SIZE + 1 );
  }
  if( directory[file_descriptor].status == UNUSED ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_8.2: file status is unused: %d\n",
        file_descriptor );
    }
    return( MAX_FILE_SIZE + 1 );
  }

  return( directory[file_descriptor].size );
}


/* tfs_read()
 *
 * reads a specified number of bytes from a file starting
 *   at the byte offset in the directory into the specified
 *   buffer; the byte offset in the directory entry is
 *   incremented by the number of bytes transferred
 *
 * depending on the starting byte offset and the specified
 *   number of bytes to transfer, the transfer may cross two
 *   or more storage blocks
 *
 * this function does not directly access bytes in storage;
 *   instead, each block is first read into an internal
 *   buffer using the helper function tfs_read_block();
 *   this function then moves bytes from the internal buffer
 *   to the user buffer
 *
 * the function will read fewer bytes than specified if the
 *   end of the file is encountered before the specified number
 *   of bytes have been transferred
 *
 * preconditions:
 *   (1) the file descriptor is in range
 *   (2) the directory entry is open
 *   (3) the requested byte count is non-zero
 *   (4) the file has allocated storage blocks and is
 *         non-empty
 *   (5) the current byte offset is less than the current
 *         file size
 *   (6) (unchecked) the user buffer has enough room to
 *         contain the requested number of bytes
 *
 * postconditions:
 *   (1) the user buffer contains bytes transferred from the
 *         storage blocks
 *   (2) the specified number of bytes has been transferred
 *         except in the case that end of file was encountered
 *         before the transfer was complete or a block read
 *         failed
 *   (3) the byte offset and current block are updated to
 *         point to the next byte to read or write (in the
 *         case of the offset being equal to the file size
 *         and just beyond the end of the last block of the
 *         file, the current block will be set to LAST_BLOCK)
 *
 * input parameters are a file descriptor, the address of a
 *   buffer of bytes to transfer, and the count of bytes to
 *   transfer
 *
 *   note that the count of bytes to transfer may be larger
 *     than the number of bytes in the file past the current
 *     byte offset
 *
 * return value is the number of bytes transferred
 */

unsigned int tfs_read( unsigned int file_descriptor,
                       char *user_buffer,
                       unsigned int byte_count ){

  unsigned int actual_count,   /* index into user buffer     */
               internal_index, /* index into internal buffer */
               current_block;

  char internal_buffer[BLOCK_SIZE];

  if( CALL_LOGGING ){
    fprintf( stderr, "log_p_9: read() called with: %d, %p, and %d\n",
      file_descriptor, user_buffer, byte_count );
  }

  /* precondition checks */
  if( !tfs_is_fd_in_range( file_descriptor ) ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_9.1: file descriptor out of range: %d\n",
        file_descriptor );
    }
    return( 0 );
  }
  if( !tfs_is_fd_open( file_descriptor ) ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_9.2: attempt to read closed file: %d\n",
        file_descriptor );
    }
    return( 0 );
  }
  if( byte_count == 0 ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_9.3: attempt to read 0 bytes\n" );
    }
    return( 0 );
  }
  if( directory[file_descriptor].first_block == 0 ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_9.4: attempt to read empty file: %d\n",
        file_descriptor );
    }
    return( 0 );
  }
  if( directory[file_descriptor].size == 0 ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_9.5: attempt to read empty file: %d\n",
        file_descriptor );
    }
    return( 0 );
  }
  if( directory[file_descriptor].byte_offset >=
        directory[file_descriptor].size         ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_9.6: attempt to read past end of file: %d\n",
        file_descriptor );
    }
    return( 0 );
  }

  /* obtain the initial storage block from which to read */
  current_block = directory[file_descriptor].current_block;
  if( !tfs_block_read( current_block, internal_buffer ) ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_p_9.7: fail to read block: %d\n",
        current_block );
    }
    return( 0 );
  }

  /* initialize each buffer index value
   * - note that the internal buffer index starts
   *     where the most recent open, read, write,
   *     or seek for this file set the byte offset;
   *     this may be in the middle of a storage block
   */
  actual_count = 0;
  internal_index = directory[file_descriptor].byte_offset % BLOCK_SIZE;

  /* loop to transfer bytes from internal buffer to user buffer
   *
   * updates on each iteration:
   *   (1) read new block if needed
   *   (2) transfer a byte
   *   (3) increment actual count
   *   (4) increment internal index
   *   (5) increment file byte offset
   *
   * stopping conditions:
   *   (1) actual count == byte count
   *   (2) byte_offset  == file size
   *
   * ending edge case:
   *   internal index off edge of internal block
   */

  while( ( actual_count < byte_count )                &&
         ( directory[file_descriptor].byte_offset <
           directory[file_descriptor].size          )    ){

    /* span blocks - need the next storage block from which to read */
    if( internal_index >= BLOCK_SIZE ){
      /* set next block as new current block and read it*/
      current_block = file_allocation_table[current_block];
      directory[file_descriptor].current_block = current_block;
      if( !tfs_block_read( current_block, internal_buffer ) ){
        if( ERROR_LOGGING ){
          fprintf( stderr, "err_p_9.8: fail to read block: %d\n",
            current_block );
        }
        return( actual_count );
      }
      internal_index = 0;
    }

    user_buffer[actual_count++] = internal_buffer[internal_index++];
    directory[file_descriptor].byte_offset++;
  }

  if( internal_index >= BLOCK_SIZE ){
    /* set next block as new current block to prepare    */
    /*   for next read or write with no intervening seek */
    current_block = file_allocation_table[current_block];
    directory[file_descriptor].current_block = current_block;
  }

  return( actual_count );
}
