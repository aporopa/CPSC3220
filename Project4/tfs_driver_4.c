/* test driver */

#include "tfs.h"


int main(){
  unsigned int fd[20];
  char buffer[1024];
  unsigned int count;

  tfs_init();

  /* create file a and write two blocks */
  fd[0] = tfs_create( "a" );
  memset( buffer, 'a', 256 );
  count = tfs_write( fd[0], buffer, 256 );
  if( count == 0 ) printf( "*** write 1 count of 0\n" );

  /* create file b and write two blocks */
  fd[1] = tfs_create( "b" );
  memset( buffer, 'b', 256 );
  count = tfs_write( fd[1], buffer, 256 );
  if( count == 0 ) printf( "*** write 2 count of 0\n" );

  /* interleave writing blocks to the two files */
  memset( buffer, 'a', 128 );
  count = tfs_write( fd[0], buffer, 128 );
  if( count == 0 ) printf( "*** write 3 count of 0\n" );

  memset( buffer, 'b', 128 );
  count = tfs_write( fd[1], buffer, 128 );
  if( count == 0 ) printf( "*** write 4 count of 0\n" );

  memset( buffer, 'a', 128 );
  count = tfs_write( fd[0], buffer, 128 );
  if( count == 0 ) printf( "*** write 5 count of 0\n" );

  memset( buffer, 'b', 128 );
  count = tfs_write( fd[1], buffer, 128 );
  if( count == 0 ) printf( "*** write 6 count of 0\n" );

  tfs_close( fd[0] );
  tfs_close( fd[1] );

  tfs_list_directory();

  tfs_copy( "a", "c" );

  fd[2] = tfs_open( "c" );
  count = tfs_read( fd[2], buffer, 8 );
  buffer[8] = '\0';
  printf( "read 8 bytes [%s] from file c\n", buffer );
  tfs_close( fd[2] );

  /* create file d and write two blocks */
  fd[3] = tfs_create( "d" );
  memset( buffer, 'd', 256 );
  count = tfs_write( fd[3], buffer, 256 );
  if( count == 0 ) printf( "*** write 7 count of 0\n" );
  tfs_close( fd[3] );

  tfs_list_directory();

  tfs_copy( "d", "a" );

  tfs_list_directory();
  tfs_list_blocks();

  fd[4] = tfs_open( "a" );
  count = tfs_read( fd[4], buffer, 8 );
  buffer[8] = '\0';
  printf( "read 8 bytes [%s] from file a\n", buffer );
  tfs_seek( fd[4], tfs_size( fd[4] ) ); 
  count = tfs_write( fd[4], buffer, 8 );
  if( count == 0 ) printf( "*** write 8 count of 0\n" );
  tfs_close( fd[4] );

  tfs_delete( 2 );
  tfs_delete( 3 );
  tfs_list_directory();

  tfs_copy( "a", "e" );
  tfs_copy( "e", "f" );
  tfs_list_directory();

  fd[5] = tfs_open( "f" );
  count = tfs_read( fd[5], buffer, 8 );
  buffer[8] = '\0';
  printf( "read 8 bytes [%s] from file f\n", buffer );
  tfs_close( fd[5] );

  return 0;
}
