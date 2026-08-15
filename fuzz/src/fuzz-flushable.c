//
//  fuzz-flushable.c
//  mulle-buffer
//
//  Fuzz target: push bytes through a flushable buffer, verify flushed total.
//
#include "mulle-buffer.h"
#include "mulle-flushablebuffer.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static size_t  fuzz_total_flushed = 0;

static size_t  flusher(void *buf, size_t one, size_t len, void *userinfo)
{
   MULLE_C_UNUSED(userinfo);
   fuzz_total_flushed += one * len;
   return len;
}


int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
   char                          storage[MULLE_FLUSHABLEBUFFER_DEFAULT_CAPACITY];
   struct mulle_flushablebuffer  fbuffer;
   struct mulle_buffer           *buffer;

   fuzz_total_flushed = 0;

   mulle_flushablebuffer_init(&fbuffer,
                              storage,
                              sizeof(storage),
                              flusher,
                              NULL);
   buffer = mulle_flushablebuffer_as_buffer(&fbuffer);

   mulle_buffer_add_bytes(buffer, (void *) data, size);

   mulle_flushablebuffer_done(&fbuffer);

   assert(fuzz_total_flushed == size);
   return 0;
}