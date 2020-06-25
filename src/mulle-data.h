#ifndef mulle_data_h__
#define mulle_data_h__

#include "include.h"

#include <stddef.h>


//
// Since length is the max object size according to POSIX, this
// sounds like the sane type to use for length
// Not yet used by mulle-buffer, but will eventually
//
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


static inline struct mulle_data   mulle_data_make_empty( void)
{
   struct mulle_data   data;

   data.bytes  = NULL;
   data.length = 0;
   return( data);
}


static inline struct mulle_data   mulle_data_make_invalid( void)
{
   struct mulle_data   data;

   data.bytes  = NULL;
   data.length = (size_t) -1;
   return( data);
}


static inline int   mulle_data_is_empty( struct mulle_data data)
{
   return( data.length == 0);
}


static inline int   mulle_data_is_invalid( struct mulle_data data)
{
   return( data.length == (size_t) -1);
}


#endif
