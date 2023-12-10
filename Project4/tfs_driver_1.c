/* test driver */

#include "tfs.h"

int main(){
  unsigned int fd[20];
  char buffer1[1024], buffer2[1024], buffer3[1024];
  unsigned int length1, length2, count1, count2, count3;

  sprintf( buffer1, "%s",
    "This is a simple-minded test for the trivial file system code.  " );

  sprintf( buffer2, "%s",
    "And now for something completely different." );

  length1 = strlen( buffer1 );
  length2 = strlen( buffer2 );
  printf( "length of buffer1 is %d\n", length1 );
  printf( "length of buffer2 is %d\n", length2 );

  tfs_init();

  fd[0] = tfs_create( "file.txt" );
  fd[1] = tfs_create( "my_file" );
  tfs_list_directory();

  count1 = tfs_write( fd[0], buffer1, length1 );
  printf( "%d bytes written to first file\n", count1 );

  count2 = tfs_write( fd[1], buffer2, length2 );
  printf( "%d bytes written to second file\n", count2 );

  tfs_close( fd[0] );
  tfs_list_directory();
  tfs_list_blocks();

  printf( "reopen file.txt\n" );
  fd[2] = tfs_open( "file.txt" );

  printf( "seek to 0 (rewind) in second file\n" );
  tfs_seek( fd[1], 0 );

  count3 = tfs_read( fd[2], buffer3, 10 );
  printf( "%d bytes read from first file\n", count3 );
  buffer3[count3] = '\0';
  printf( "[%s]\n", buffer3 );

  count3 = tfs_read( fd[2], buffer3, 10 );
  printf( "%d more bytes read from first file\n", count3 );
  buffer3[count3] = '\0';
  printf( "[%s]\n", buffer3 );

  count3 = tfs_read( fd[1], buffer3, 10 );
  printf( "%d bytes read from second file\n", count3 );
  buffer3[count3] = '\0';
  printf( "[%s]\n", buffer3 );

  tfs_close( fd[1] );
  tfs_close( fd[2] );

  return 0;
}
