//
//  fuzz-string.c
//  mulle-buffer
//
//  Fuzz target: build a string from fuzz data, extract and verify.
//
#include "mulle-buffer.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
   if( size == 0)
      return 0;

   char  *s;

   mulle_buffer_do_string(buffer, NULL, s)
   {
      mulle_buffer_add_bytes(buffer, (void *) data, size);
      assert(mulle_buffer_get_length(buffer) == size);

      // ensure zero-termination
      mulle_buffer_add_byte(buffer, 0);
      assert(mulle_buffer_get_length(buffer) == size + 1);

      char *p = mulle_buffer_get_string(buffer);
      assert(p);
      assert(strlen(p) <= size);
   }  // s is extracted here

   if( s)
   {
      assert(strlen(s) <= size);
      assert(memcmp(s, data, strlen(s)) == 0);
      mulle_free(s);
   }
   return 0;
}