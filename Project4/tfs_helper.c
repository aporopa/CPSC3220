#include "tfs.h"

/* implementation of helper functions - instructor supplied
 *
 * nine helper functions
 * - call log message prefix is log_h_i for i-th function
 * - error log message prefix is err_h_i for i-th function
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


/* tfs_is_fd_in_range()
 *
 * validates a file descriptor value using a range check
 *
 * input parameter is file descriptor value
 *
 * return value is TRUE when successful or FALSE when failure
 */

unsigned int tfs_is_fd_in_range( unsigned int fd ){

  if( CALL_LOGGING ){
    fprintf( stderr, "log_h_1: is_fd_in_range() called with: %d\n", fd );
  }

  if( ( fd < FIRST_VALID_FD ) || ( fd > LAST_VALID_FD ) ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_h_1: file descriptor out of range: %d\n", fd );
    }
    return( FALSE );
  }

  return( TRUE );
}


/* tfs_is_block_is_in_range()
 *
 * validates a block number value using a range check
 *
 * input parameter is block number value
 *
 * return value is TRUE when successful or FALSE when failure
 */

unsigned int tfs_is_block_in_range( unsigned int b ){

  if( CALL_LOGGING ){
    fprintf( stderr, "log_h_2: is_block_in_range() called with: %d\n", b );
  }

  if( ( b >= FIRST_VALID_BLOCK ) && ( b <= LAST_VALID_BLOCK ) ){
    return( TRUE );
  }else{
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_h_2: block number out of range: %d\n", b );
    }
    return( FALSE );
  }
}


/* tfs_is_fd_open()
 *
 * validates that a file is open
 *
 * input parameter is file descriptor
 *
 * return value is TRUE when successful or FALSE when failure
 */

unsigned int tfs_is_fd_open( unsigned int fd ){

  if( CALL_LOGGING ){
    fprintf( stderr, "log_h_3: is_fd_open() called with: %d\n", fd );
  }

  if( directory[fd].status == OPEN ){
    return( TRUE );
  }else{
    return( FALSE );
  }
}


/* tfs_is_valid_file_name()
 *
 * validates a file name
 *
 * input parameter is character string containing the file name
 *
 * return value is TRUE when successful or FALSE when failure
 */

unsigned int tfs_is_valid_name( char *name ){
  int i, len = strlen( name );

  if( CALL_LOGGING ){
    fprintf( stderr, "log_h_4: is_valid_name() called with: %s\n", name );
  }

  if( len > FILENAME_LENGTH ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_h_4.1: file name too long\n" );
    }
    return( FALSE );
  }

  for( i = 0; i < len; i++ ){
    if( !isalnum( name[i] ) && ( name[i] != '_' ) && ( name[i] != '.' ) ){
      if( ERROR_LOGGING ){
        fprintf( stderr, "err_h_4.2: file name has non-alphanumeric," );
        fprintf( stderr, " non-underscore character\n" );
      }
      return( FALSE );
    }
  }

  return( TRUE );
}


/* tfs_map_file_name_to_fd()
 *
 * seaches the directory for a file name and returns the file
 *   descriptor
 *
 * precondition:
 *   file name is valid
 *
 * postconditions:
 *   none
 *
 * input parameter is character string containing the file name
 *
 * return value is the file descriptor when successful or 0
 *   when failure
 */

unsigned int tfs_map_name_to_fd( char *name ){
  unsigned int fd;

  if( CALL_LOGGING ){
    fprintf( stderr, "log_h_5: map_name_to_fd() called with: %s\n", name );
  }

  /* precondition check */
  if( !tfs_is_valid_name( name ) ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_h_5.1: name %s not valid\n", name );
    }
    return( 0 );
  }

  for( fd = 1; fd < N_DIRECTORY_ENTRIES; fd++ ){
    if( ( directory[fd].name != UNUSED ) &&
        ( strcmp( name, directory[fd].name ) == 0 ) ){
      return( fd );
    }
  }

  if( ERROR_LOGGING ){
    fprintf( stderr, "err_h_5.2: file name %s not found\n", name );
  }
  return( 0 );
}


/* tfs_new_directory_entry()
 *
 * seaches the directory for a free entry and returns that index
 *
 * no parameters
 *
 * return value is the index of a free directory entry when
 *   successful or 0 when failure
 */

unsigned int tfs_new_directory_entry(){
  unsigned int fd;

  if( CALL_LOGGING ){
    fprintf( stderr, "log_h_6: new_directory_entry() called\n" );
  }

  for( fd = FIRST_VALID_FD; fd < N_DIRECTORY_ENTRIES; fd++ ){
    if( directory[fd].status == UNUSED ){
      return( fd );
    }
  }

  if( ERROR_LOGGING ){
    fprintf( stderr, "err_h_6: no free directory entries\n" );
  }
  return( 0 );
}


/* tfs_new_block()
 *
 * seaches FAT for a free block and returns that block number
 *
 * no parameters
 *
 * return value is the block number of a free block when
 *   successful or 0 when failure
 */

unsigned int tfs_new_block(){
  unsigned int b;

  if( CALL_LOGGING ){
    fprintf( stderr, "log_h_7: new_block() called\n" );
  }

  for( b = FIRST_VALID_BLOCK; b < N_BLOCKS; b++ ){
    if( file_allocation_table[b] == FREE ) return( b );
  }

  if( ERROR_LOGGING ){
    fprintf( stderr, "err_h_7: no free storage blocks\n" );
  }
  return( 0 );
}


/* tfs_block_read()
 *
 * transfers a block of data from a block in storage to an
 *   internal buffer
 *
 * preconditions:
 *   (1) block number is valid
 *   (2) (unchecked) buffer is internal to the file system
 *
 * postconditions:
 *   the data is transfered from the block in storage to the
 *     buffer
 *
 * input parameters are a block number and a byte pointer
 *
 * return value is TRUE when successful or FALSE when failure
 */

unsigned int tfs_block_read( unsigned int b, char *buf ){
  unsigned int i;

  if( CALL_LOGGING ){
    fprintf( stderr, "log_h_8: block_read() called with %d and %p\n",
      b, buf );
  }

  /* precondition check */
  if( !tfs_is_block_in_range( b ) ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_h_8: block out or range, %d\n", b );
    }
    return( FALSE );
  }

  for( i = 0; i < BLOCK_SIZE; i++ ){
    buf[i] = blocks[b].bytes[i];
  }

  return( TRUE );
}


/* tfs_block_write()
 *
 * transfers a block of data from an internal buffer to a block
 *   in storage
 *
 * preconditions:
 *   (1) block number is valid
 *   (2) (unchecked) buffer is internal to the file system
 *
 * postconditions:
 *   the data is transfered from the buffer to the block in
 *     storage
 *
 * input parameters are a block number and a byte pointer
 *
 * return value is TRUE when successful or FALSE when failure
 */

unsigned int tfs_block_write( unsigned int b, char *buf ){
  unsigned int i;

  if( CALL_LOGGING ){
    fprintf( stderr, "log_h_9: block_write() called with %d and %p\n",
      b, buf );
  }

  /* precondition check */
  if( !tfs_is_block_in_range( b ) ){
    if( ERROR_LOGGING ){
      fprintf( stderr, "err_h_9: block out or range, %d\n", b );
    }
    return( FALSE );
  }

  for( i = 0; i < BLOCK_SIZE; i++ ){
    blocks[b].bytes[i] = buf[i];
  }

  return( TRUE );
}
