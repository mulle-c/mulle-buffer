//
//  fuzz-add-extract.c
//  mulle-buffer
//
//  Fuzz target: add random bytes then extract and verify length.
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

      size_t len = mulle_buffer_get_length(buffer);
      assert(len == size);

      if( size)
      {
         void *bytes = mulle_buffer_get_bytes(buffer);
         assert(bytes);
         assert(memcmp(bytes, data, size) == 0);

         // peek at last byte
         int last = mulle_buffer_get_last_byte(buffer);
         assert(last == (int) data[size - 1]);

         // get_byte at each position
         for(size_t i = 0; i < size; i++)
         {
            int b = mulle_buffer_get_byte(buffer, (unsigned int) i);
            assert(b == (int) data[i]);
         }

         // out of bounds get_byte returns -1
         assert(mulle_buffer_get_byte(buffer, (unsigned int) size) == -1);
      }
      else
      {
         assert(mulle_buffer_get_last_byte(buffer) == -1);
      }

      // extract and verify
      void *extracted = mulle_buffer_extract_bytes(buffer);
      if( size)
      {
         assert(extracted);
         assert(memcmp(extracted, data, size) == 0);
         mulle_free(extracted);
      }
      else
         assert(extracted == NULL);

      // after extract the buffer is reset; on stack-backed buffers
      // this means back to initial storage (not void), on heap-backed
      // buffers _storage==_sentinel (void). Either way, length is 0.
      assert(mulle_buffer_get_length(buffer) == 0);
   }
   return 0;
}