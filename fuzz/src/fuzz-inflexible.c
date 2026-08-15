//
//  fuzz-inflexible.c
//  mulle-buffer
//
//  Fuzz target: push random bytes into an inflexible buffer, verify contents
//  match up to capacity and overflow is reported correctly.
//
#include "mulle-buffer.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
   char   storage[64];

   mulle_buffer_do_inflexible(buffer, storage, sizeof(storage))
   {
      mulle_buffer_add_bytes(buffer, (void *) data, size);

      // an inflexible buffer never grows: writes beyond capacity set the
      // sticky overflow flag and the pre-overflow content is preserved
      if( size > sizeof(storage))
      {
         assert(mulle_buffer_has_overflown(buffer));
         assert(mulle_buffer_get_length(buffer) <= sizeof(storage));
         if( data && mulle_buffer_get_length(buffer))
            assert(memcmp(mulle_buffer_get_bytes(buffer), data,
                          mulle_buffer_get_length(buffer)) == 0);
      }
      else
      {
         assert( ! mulle_buffer_has_overflown(buffer));
         assert(mulle_buffer_get_length(buffer) == size);
         if( data && size)
            assert(memcmp(mulle_buffer_get_bytes(buffer), data, size) == 0);
      }
   }
   return 0;
}