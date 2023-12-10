/* test driver */

#include "tfs.h"

int main(){
  unsigned int fd[20];
  char buffer1[1024], buffer2[1024], buffer3[1024];
  unsigned int length1, length2, count1, count2, count3;

  sprintf( buffer1, "%s",
    "This is a simple-minded test for the trivial file system code.  " );
  sprintf( buffer1 + 64, "%s",
    "This is a simple-minded test for the trivial file system code.  " );
  sprintf( buffer1 + 128, "%s",
    "This is a simple-minded test for the trivial file system code.  " );
  sprintf( buffer1 + 192, "%s",
    "This is a simple-minded test for the trivial file system code.  " );
  sprintf( buffer1 + 256, "%s",
    "This is a simple-minded test for the trivial file system code.  " );

  sprintf( buffer2, "%s",
    "And now for something completely different." );

  length1 = strlen( buffer1 );
  length2 = strlen( buffer2 );
  printf( "length of buffer1 is %d\n", length1 );
  printf( "length of buffer2 is %d\n", length2 );

  tfs_init();

  tfs_list_directory();

  fd[0] = tfs_create( "file.txt" );
  if( fd[0] == 0 ) printf( "first create failed\n" );

  fd[1] = tfs_create( "my_file" );
  if( fd[1] == 0 ) printf( "second create failed\n" );

  tfs_list_directory();

  count1 = tfs_write( fd[0], buffer1, length1 );
  printf( "%d bytes written to first file\n", count1 );

  count2 = tfs_write( fd[1], buffer2, length2 );
  printf( "%d bytes written to second file\n", count2 );

  count1 = tfs_write( fd[0], buffer1, length1 );
  printf( "%d bytes written to first file\n", count1 );

  tfs_close( fd[1] );

  tfs_list_directory();
  tfs_list_blocks();

  tfs_seek( fd[0], 600 );
  count3 = tfs_read( fd[0], buffer3, 640 );
  printf( "%d bytes read from first file\n", count3 );
  buffer3[count3] = '\0';
  printf( "[%s]\n", buffer3 );

  tfs_seek( fd[0], 250 );
  count3 = tfs_read( fd[0], buffer3, 20 );
  printf( "%d bytes read from first file\n", count3 );
  buffer3[count3] = '\0';
  printf( "[%s]\n", buffer3 );

  fd[2] = tfs_create( "file.txt" );
  printf( "fd for creating a file with identical name" );
  printf( " as existing file - %d\n", fd[2] );
  fd[2] = tfs_create( "file3" );
  fd[3] = tfs_create( "file4" );
  fd[4] = tfs_create( "file5" );
  fd[5] = tfs_create( "file6" );
  fd[6] = tfs_create( "file7" );
  fd[7] = tfs_create( "file8" );
  fd[8] = tfs_create( "file9" );
  fd[9] = tfs_create( "file10" );
  fd[10] = tfs_create( "file11" );
  fd[11] = tfs_create( "file12" );
  fd[12] = tfs_create( "file13" );
  fd[13] = tfs_create( "file14" );
  fd[14] = tfs_create( "file15" );
  fd[15] = tfs_create( "file16" );
  printf( "fd for creating a sixteenth file - %d\n", fd[15] );

  tfs_list_directory();

  tfs_close( fd[0] );
  tfs_delete( fd[0] );

  tfs_list_directory();

  tfs_close( fd[3] );
  tfs_close( fd[4] );
  tfs_close( fd[5] );
  tfs_close( fd[6] );
  tfs_close( fd[7] );

  tfs_delete( fd[6] );
  tfs_delete( fd[7] );

  tfs_list_directory();

  fd[16] = tfs_create( "added_1" );
  fd[17] = tfs_create( "added_2" );

  tfs_list_directory();
  tfs_list_blocks();

  return 0;
}
