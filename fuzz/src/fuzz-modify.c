//
//  fuzz-modify.c
//  mulle-buffer
//
//  Fuzz target: random mutations on buffer state. Add, remove, seek,
//  set_length, memset, remove_last_byte on the same buffer instance.
//
#include "mulle-buffer.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
   size_t  n;
   size_t  i;

   mulle_buffer_do(buffer)
   {
      i = 0;
      while( i + 1 < size)
      {
         unsigned char op = data[i++];
         unsigned char arg = data[i++];

         switch( op & 0xf)
         {
         case 0:
            mulle_buffer_add_byte(buffer, arg);
            break;

         case 1:
            // add a chunk of remaining bytes
            n = (size_t) arg;
            if( n > size - i)
               n = size - i;
            mulle_buffer_add_bytes(buffer, (void *) &data[i], n);
            i += n;
            break;

         case 2:
            if( ! mulle_buffer_is_empty(buffer))
               mulle_buffer_remove_last_byte(buffer);
            break;

         case 3:
            mulle_buffer_remove_all(buffer);
            break;

         case 4:
            // seek to random position
            if( ! mulle_buffer_is_void(buffer))
            {
               n = mulle_buffer_get_length(buffer);
               if( n)
                  mulle_buffer_set_seek(buffer,
                                        (long) (arg % n),
                                        MULLE_BUFFER_SEEK_SET);
            }
            break;

         case 5:
            // set_length
            mulle_buffer_set_length(buffer,
                                    (size_t) arg,
                                    MULLE_BUFFER_NO_SHRINK_OR_ZEROFILL);
            break;

         case 6:
            mulle_buffer_memset(buffer, (int) arg, (size_t) arg);
            break;

         case 7:
            if( ! mulle_buffer_is_void(buffer) && mulle_buffer_get_length(buffer))
               mulle_buffer_pop_byte(buffer);
            break;

         default:
            break;
         }

         // check invariants (will assert on overflow, bad state during add)
         assert(! mulle_buffer_has_overflown(buffer));
      }

      // final extraction must succeed or have zero length
      if( mulle_buffer_get_length(buffer))
      {
         void *extracted = mulle_buffer_extract_bytes(buffer);
         assert(extracted);
         mulle_free(extracted);
      }
   }
   return 0;
}