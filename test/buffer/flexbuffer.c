//
//  main.c
//  test-buffer
//
//  Created by Nat! on 04.11.15.
//  Copyright (c) 2015 Nat! - Mulle kybernetiK.
//  Copyright (c) 2015 Codeon GmbH.
//  All rights reserved.
//

#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>

#include <stdio.h>
#include <ctype.h>


static void  simple( int n)
{
   unsigned int   i;

   mulle_flexbuffer_do( buffer, 4, n + 1)
   {

      for( i = 0; i < n; i++)
      {
         buffer[ i] = '0' + i % 10;
      }
      buffer[ i] = 0;

      printf( "%.1000s\n", buffer);
   }
}


static void   flex_return( int n)
{
   mulle_flexbuffer_do( buffer, 4, n)
   {
      mulle_flexbuffer_return_void( buffer);
   }
}


static void  flex_explicit( int unused)
{
   mulle_flexbuffer( copy, 32);
   size_t n;
   char *s1  = "hello";
   char *s2 = " world";

   n = strlen(s1) + 1;
   mulle_flexbuffer_alloc( copy, n);

   strcpy(copy, s1);
   copy[ 0] = toupper( copy[ 0]);
   printf("Copied string: %s\n", copy);
   if( copy[ 0] == 'Q')
      mulle_flexbuffer_return_void( copy);

   mulle_flexbuffer_realloc( copy, n + strlen( s2));
   strcat(copy, s2);

   printf("Copied string: %s\n", copy);

   mulle_flexbuffer_done( copy);
}


// the mulle_testallocator detects and aborts on leaks
static void  run_test( void (*f)( int), char *name, int n)
{
   mulle_testallocator_reset();
  // printf( "%s\n", name);
   (f)( n);
   mulle_testallocator_reset();
}


int main(int argc, const char * argv[])
{
   run_test( simple, "simple", 1);
   run_test( simple, "simple", 4);
   run_test( simple, "simple", 16);
   run_test( simple, "simple", 100000);
   run_test( simple, "simple", 0);

   run_test( (void (*)( int)) flex_return, "flex_return", 100);
   run_test( (void (*)( int)) flex_explicit, "flex_explicit", 100);

   return( 0);
}
