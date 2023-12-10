/* test driver */

#include "tfs.h"

void test_result( char *str1, char *str2, char *str3 ){
  if( strcmp(str1,str2) != 0 ){
    printf( " *** should be %s %s\n", str2, str3 );
  }
}

int main(){
  unsigned int fd[20];
  char buffer1[1024], buffer2[1024];
  unsigned int count1, count2;

  tfs_init();

  /* create a file and write one full block = 128 bytes */
  fd[0] = tfs_create( "file.txt" );
  sprintf( buffer1, "%s", " 004 008 012 016 020 024 028 032" );
  count1 = tfs_write( fd[0], buffer1, 32 );
  if( count1 != 32 ) printf( "write error 1\n" );
  sprintf( buffer1, "%s", " 036 040 044 048 052 056 060 064" );
  count1 = tfs_write( fd[0], buffer1, 32 );
  if( count1 != 32 ) printf( "write error 2\n" );
  sprintf( buffer1, "%s", " 068 072 076 080 084 088 092 096" );
  count1 = tfs_write( fd[0], buffer1, 32 );
  if( count1 != 32 ) printf( "write error 3\n" );
  sprintf( buffer1, "%s", " 100 104 108 112 116 120 124 128" );
  count1 = tfs_write( fd[0], buffer1, 32 );
  if( count1 != 32 ) printf( "write error 4\n" );

  /* create a second file and write a few bytes to it so that */
  /*   the blocks for the first file will not be contiguous   */
  fd[1] = tfs_create( "file2" );
  sprintf( buffer1, "%s", " aaa bbb ccc ddd" );
  count1 = tfs_write( fd[1], buffer1, 16 );
  if( count1 != 16 ) printf( "write error 5\n" );

  /* write another half a block to the first file = 64 bytes */
  sprintf( buffer1, "%s", " 132 136 140 144 148 152 156 160" );
  count1 = tfs_write( fd[0], buffer1, 32 );
  if( count1 != 32 ) printf( "write error 6\n" );
  sprintf( buffer1, "%s", " 164 168 172 176 180 184 188 192" );
  count1 = tfs_write( fd[0], buffer1, 32 );
  if( count1 != 32 ) printf( "write error 7\n" );

  /* first file should be
   *
   *     full first block      partially-full second block
   *   +--------...--------+   +--------...----|---...---+
   *   | 004 008... 124 128|   | 132 136... 192 ***...***|
   *   +--------...--------+   +--------...----|---...---+
   *                                          * = null byte
   */

  tfs_list_directory();
  tfs_list_blocks();

  /* rewind (that is, seek to 0) and read the first 8 bytes */
  tfs_seek( fd[0], 0 );
  count2 = tfs_read( fd[0], buffer2, 8 );
  buffer2[count2] = '\0';
  printf( "%d bytes read [%s] on seek to 0\n", count2, buffer2 );
  test_result( buffer2, " 004 008", "on seek to 0" ); 

  /* close and reopen file to reset the byte offset to 0 */
  tfs_close( fd[0] );
  fd[2] = tfs_open( "file.txt" );

  /* read first 8 bytes */
  count2 = tfs_read( fd[2], buffer2, 8 );
  buffer2[count2] = '\0';
  printf( "%d bytes read [%s] on reopening\n", count2, buffer2 );
  test_result( buffer2, " 004 008", "on reopening" ); 

  /* then read the next 8 bytes to test the byte offset updating */
  count2 = tfs_read( fd[2], buffer2, 8 );
  buffer2[count2] = '\0';
  printf( "%d bytes read [%s] subsequent bytes\n", count2, buffer2 );
  test_result( buffer2, " 012 016", "on subsequent read" ); 

  /* seek to 120 and read the last 8 bytes of the block */
  tfs_seek( fd[2], 120 );
  count2 = tfs_read( fd[2], buffer2, 8 );
  buffer2[count2] = '\0';
  printf( "%d bytes read [%s] at end of first block\n", count2, buffer2 );
  test_result( buffer2, " 124 128", "on seek to 120" ); 

  /* then read the next 8 bytes to test the byte offset updating */
  count2 = tfs_read( fd[2], buffer2, 8 );
  buffer2[count2] = '\0';
  printf( "%d bytes read [%s] at start of next block\n", count2, buffer2 );
  test_result( buffer2, " 132 136", "on subsequent read (from next block)" ); 

  /* seek to 124 and read the last 4 bytes of the block and the */
  /*   first 4 bytes of the next block to test spanned reads    */
  tfs_seek( fd[2], 124 );
  count2 = tfs_read( fd[2], buffer2, 8 );
  buffer2[count2] = '\0';
  printf( "%d bytes read [%s] spanning blocks\n", count2, buffer2 );
  test_result( buffer2, " 128 132", "on seek to 124 (spanning blocks)" ); 

  /* seek to 128 (start of second block) and read the */
  /*   first 8 bytes of the second block              */
  tfs_seek( fd[2], 128 );
  count2 = tfs_read( fd[2], buffer2, 8 );
  buffer2[count2] = '\0';
  printf( "%d bytes read [%s] at start of second block\n", count2, buffer2 );
  test_result( buffer2, " 132 136", "on seek to 128" ); 

  /* seek to 64 (middle of first block) and write 8 bytes */
  sprintf( buffer1, "%s", " uuu vvv" );
  tfs_seek( fd[2], 64 );
  count1 = tfs_write( fd[2], buffer1, 8 );
  if( count1 != 8 ) printf( "write error 8\n" );

  /* seek to 60 and read 16 bytes of the block */
  /*   to test the mid-block write of 8 bytes  */
  tfs_seek( fd[2], 60 );
  count2 = tfs_read( fd[2], buffer2, 16);
  buffer2[count2] = '\0';
  printf( "%d bytes read [%s] testing mid-block write\n", count2, buffer2 );
  test_result( buffer2, " 064 uuu vvv 076", "on seek to 60 (mid-block)" ); 

  /* seek to 120 (8 away from end of first block) and write 8 bytes */
  sprintf( buffer1, "%s", " www xxx" );
  tfs_seek( fd[2], 120 );
  count1 = tfs_write( fd[2], buffer1, 8 );
  if( count1 != 8 ) printf( "write error 9\n" );

  /* read 8 bytes to test read of subsequent block after write */
  count2 = tfs_read( fd[2], buffer2, 8);
  buffer2[count2] = '\0';
  printf( "%d bytes read [%s] at start of second block\n", count2, buffer2 );
  test_result( buffer2, " 132 136", "on read at start of next block" ); 
  
  /* seek to 124 (4 away from end of first block) and write 8 bytes */
  sprintf( buffer1, "%s", " yyy zzz" );
  tfs_seek( fd[2], 124 );
  count1 = tfs_write( fd[2], buffer1, 8 );
  if( count1 != 8 ) printf( "write error 10\n" );

  /* seek to 120 and read 16 bytes (spanning the first and        */
  /*   second blocks) to test the block-spanning write of 8 bytes */
  tfs_seek( fd[2], 120 );
  count2 = tfs_read( fd[2], buffer2, 16 );
  buffer2[count2] = '\0';
  printf( "%d bytes read [%s] spanning blocks\n", count2, buffer2 );
  test_result( buffer2, " www yyy zzz 136", "on seek to 120 (span blocks)" ); 

  /* seek to end of file and write 8 bytes */
  sprintf( buffer1, "%s", " 196 200" );
  tfs_seek( fd[2], tfs_size( fd[2] ) );
  count1 = tfs_write( fd[2], buffer1, 8 );
  if( count1 != 8 ) printf( "write error 11\n" );
  printf( "file size now %d\n", tfs_size( fd[2] ) );

  /* seek to 188 and try to read 32 bytes - should get 12 */
  tfs_seek( fd[2], 188 );
  count2 = tfs_read( fd[2], buffer2, 32 );
  buffer2[count2] = '\0';
  printf( "%d bytes read [%s] extra bytes\n", count2, buffer2 );
  test_result( buffer2, " 192 196 200", "on extending file" ); 
  
  return 0;
}
