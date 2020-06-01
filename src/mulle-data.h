#ifndef mulle_data_h__
#define mulle_data_h__

#include "include.h"

#include <stddef.h>


//
// Since length is the max object size according to POSIX, this
// sounds like the sane type to use for length
// Not yet used by mulle-buffer, but will eventually
struct mulle_data
{
   void     *bytes;
   size_t   length;
};


static inline struct mulle_data   mulle_data_make( void *bytes, size_t length)
{
   struct mulle_data   data;

   data.bytes  = bytes;
   data.length = length;
   return( data);
}

#endif
