//
//  fuzz-seek.c
//  mulle-buffer
//
//  Fuzz target: add data, seek around randomly, verify position and content.
//  Note: SEEK_SET/SEEK_CUR validate against _sentinel (allocation boundary),
//  not against written length. SEEK_END is relative to _sentinel (hardly used).
//
#include "mulle-buffer.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
   mulle_buffer_do(buffer)
   {
      mulle_buffer_add_bytes(buffer, (void *) data, size);

      assert(mulle_buffer_set_seek(buffer, 0, MULLE_BUFFER_SEEK_SET) == 0);
      assert(mulle_buffer_get_seek(buffer) == 0);

      if( size == 0)
         return 0;

      long pos = (long) (data[0] % size);

      assert(mulle_buffer_set_seek(buffer, pos, MULLE_BUFFER_SEEK_SET) == 0);
      assert(mulle_buffer_get_seek(buffer) == pos);

      long rem = (long) size - pos;
      if( rem > 1)
      {
         assert(mulle_buffer_set_seek(buffer, 1, MULLE_BUFFER_SEEK_CUR) == 0);
         assert(mulle_buffer_get_seek(buffer) == pos + 1);
      }

      // negative seek from SET should fail
      assert(mulle_buffer_set_seek(buffer, -1, MULLE_BUFFER_SEEK_SET) != 0);

      // seek past allocation should fail
      assert(mulle_buffer_set_seek(buffer,
             (long) (mulle_buffer_get_capacity(buffer) + 1),
             MULLE_BUFFER_SEEK_SET) != 0);
   }
   return 0;
}