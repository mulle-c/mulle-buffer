#include <mulle-buffer/mulle-buffer.h>
#include <mulle-testallocator/mulle-testallocator.h>
#include <stdio.h>
#include <string.h>

static void test_null_buffer()
{
   mulle_buffer_done( NULL);
   mulle_buffer_set_allocator( NULL, NULL);
   mulle_buffer_add_byte( NULL, 'x');
   mulle_buffer_add_bytes( NULL, "test", 4);
   mulle_buffer_add_c_chars( NULL, "test", 4);
   mulle_buffer_add_string( NULL, "test");
   void *p = mulle_buffer_advance( NULL, 10);
   printf( "null_buffer: %s\n", p == NULL ? "pass" : "fail");
}

static void test_flushable_buffer_seek()
{
   char buffer[ 256];

   mulle_buffer_do_flexible( buf, buffer, sizeof( buffer))
   {
      mulle_buffer_add_string( buf, "test data");
      int result = mulle_buffer_set_seek( buf, 0, MULLE_BUFFER_SEEK_SET);
      printf( "flushable_seek_set: %d\n", result);
   }
}

static void test_add_bytes_fallback()
{
   char buffer[ 32];

   mulle_buffer_do_flexible( buf, buffer, sizeof( buffer))
   {
      mulle_buffer_add_bytes( buf, "hello", 5);
      mulle_buffer_add_bytes( buf, " world extra data here", 22);
      size_t len = mulle_buffer_get_length( buf);
      printf( "add_bytes_fallback: %zu\n", len);
   }
}

static void test_add_string_various_lengths()
{
   char buffer[ 32];

   mulle_buffer_do_flexible( buf, buffer, sizeof( buffer))
   {
      mulle_buffer_add_string( buf, "test");
      mulle_buffer_add_string( buf, "very long string that exceeds buffer");
      size_t len = mulle_buffer_get_length( buf);
      printf( "add_string_fallback: %zu\n", len);
   }
}

static void test_add_string_edge_cases()
{
   char buffer[ 64];

   mulle_buffer_do_flexible( buf, buffer, sizeof( buffer))
   {
      mulle_buffer_add_string( buf, "");
      mulle_buffer_add_string( buf, "1");
      mulle_buffer_add_string( buf, "12");
      mulle_buffer_add_string( buf, "123");
      mulle_buffer_add_string( buf, "1234");
      mulle_buffer_add_string( buf, "12345");
      mulle_buffer_add_string( buf, "123456");
      mulle_buffer_add_string( buf, "1234567");
      mulle_buffer_add_string( buf, "x very long string");
      printf( "add_string_edge: %zu\n", mulle_buffer_get_length( buf));
   }
}

static void test_set_seek_modes()
{
   char buffer[ 128];

   mulle_buffer_do_flexible( buf, buffer, sizeof( buffer))
   {
      mulle_buffer_add_string( buf, "0123456789");
      
      int r1 = mulle_buffer_set_seek( buf, 2, MULLE_BUFFER_SEEK_CUR);
      printf( "seek_cur_flushable: %d\n", r1);
      
      int r2 = mulle_buffer_set_seek( buf, -1, MULLE_BUFFER_SEEK_END);
      printf( "seek_end_flushable: %d\n", r2);
   }
}

static void test_readonly_buffer()
{
   char data[] = "readonly";
   struct mulle_buffer buf;

   mulle_buffer_init_inflexible_with_static_bytes( &buf, data, sizeof( data));
   int is_ro = mulle_buffer_is_readonly( &buf);
   printf( "readonly: %d\n", is_ro);
   mulle_buffer_done( &buf);
}

static void test_null_buffer_inlines()
{
   mulle_buffer_init( NULL, 0, NULL);
   mulle_buffer_init_inflexible_with_static_bytes( NULL, NULL, 0);
   printf( "null_inlines: pass\n");
}

static void test_remove_all()
{
   char buffer[ 128];

   mulle_buffer_do_flexible( buf, buffer, sizeof( buffer))
   {
      mulle_buffer_add_string( buf, "hello world");
      mulle_buffer_remove_all( buf);
      size_t len = mulle_buffer_get_length( buf);
      printf( "remove_all: %zu\n", len);
   }
}

static void test_buffer_capacity()
{
   struct mulle_buffer *buf = mulle_buffer_create( NULL);
   mulle_buffer_add_string( buf, "test");
   
   size_t cap = mulle_buffer_get_capacity( buf);
   printf( "buffer_capacity: %zu\n", cap);
   
   mulle_buffer_destroy( buf);
}

static void test_intersects()
{
   struct mulle_buffer *buf = mulle_buffer_create( NULL);
   mulle_buffer_add_string( buf, "hello");
   
   int intersect = mulle_buffer_intersects_bytes( buf, mulle_buffer_get_bytes( buf), 2);
   printf( "intersects: %d\n", intersect);
   
   mulle_buffer_destroy( buf);
}

static void test_setlength()
{
   char buffer[ 128];

   mulle_buffer_do_flexible( buf, buffer, sizeof( buffer))
   {
      mulle_buffer_add_string( buf, "hello");
      mulle_buffer_set_length( buf, 3, 0);
      printf( "setlength: %zu\n", mulle_buffer_get_length( buf));
   }
}

static void test_static_with_static_bytes()
{
   char data[ 256];
   struct mulle_buffer buf;

   mulle_buffer_init_inflexible_with_static_bytes( &buf, data, sizeof( data));
   mulle_buffer_add_string( &buf, "test");
   size_t len = mulle_buffer_get_length( &buf);
   printf( "static_with_static: %zu\n", len);
   mulle_buffer_done( &buf);
}

static void test_overflow_detection()
{
   char data[ 8];
   struct mulle_buffer buf;

   mulle_buffer_init_inflexible_with_static_bytes( &buf, data, sizeof( data));
   mulle_buffer_add_bytes( &buf, "hello", 5);
   mulle_buffer_add_bytes( &buf, "world", 5);
   
   int overflown = mulle_buffer_has_overflown( &buf);
   printf( "overflow_detected: %d\n", overflown);
   mulle_buffer_done( &buf);
}

static void test_get_string_on_overflow()
{
   char data[ 4];
   struct mulle_buffer buf;

   mulle_buffer_init_inflexible_with_static_bytes( &buf, data, sizeof( data));
   mulle_buffer_add_string( &buf, "toolongstring");
   
   int overflow = mulle_buffer_has_overflown( &buf);
   char *s = mulle_buffer_get_string( &buf);
   printf( "get_string_overflow: overflow=%d string=%s\n", overflow, s ? "has_data" : "NULL");
   mulle_buffer_done( &buf);
}

static void test_get_string_void_buffer()
{
   char data[ 256];
   struct mulle_buffer buf;

   mulle_buffer_init_inflexible_with_static_bytes( &buf, data, sizeof( data));
   
   char *s = mulle_buffer_get_string( &buf);
   printf( "get_string_void: %s\n", s == NULL ? "NULL" : "not-null");
   mulle_buffer_done( &buf);
}

static void test_set_seek_invalid_mode()
{
   char data[ 256];
   struct mulle_buffer buf;

   mulle_buffer_init_inflexible_with_static_bytes( &buf, data, sizeof( data));
   mulle_buffer_add_string( &buf, "0123456789");
   
   int result = mulle_buffer_set_seek( &buf, 0, 999);
   printf( "set_seek_invalid_mode: %d\n", result);
   mulle_buffer_done( &buf);
}

static void test_init_with_int_max()
{
   char buffer[ 256];

   mulle_buffer_do_flexible( buf, buffer, sizeof( buffer))
   {
      mulle_buffer_add_string( buf, "12345678");
      mulle_buffer_add_string( buf, "abcdefgh");
      mulle_buffer_add_string( buf, "ijklmnop");
      printf( "init_int_max: %zu\n", mulle_buffer_get_length( buf));
   }
}

static void test_seek_cur_overflow()
{
   char data[ 256];
   struct mulle_buffer buf;

   mulle_buffer_init_inflexible_with_static_bytes( &buf, data, sizeof( data));
   mulle_buffer_add_string( &buf, "test");
   
   mulle_buffer_set_seek( &buf, 2, MULLE_BUFFER_SEEK_CUR);
   size_t len = mulle_buffer_get_length( &buf);
   printf( "seek_cur_overflow: %zu\n", len);
   mulle_buffer_done( &buf);
}

static void test_add_string_alignment_1byte()
{
   char buffer[ 256];

   mulle_buffer_do_flexible( buf, buffer, sizeof( buffer))
   {
      mulle_buffer_add_string( buf, "a");
      mulle_buffer_add_string( buf, "12345678");
      printf( "add_string_align_1: %zu\n", mulle_buffer_get_length( buf));
   }
}

static void test_add_string_alignment_7bytes()
{
   char buffer[ 256];

   mulle_buffer_do_flexible( buf, buffer, sizeof( buffer))
   {
      mulle_buffer_add_string( buf, "1234567");
      mulle_buffer_add_string( buf, "abcdefgh");
      printf( "add_string_align_7: %zu\n", mulle_buffer_get_length( buf));
   }
}

static void test_add_string_break_in_loop()
{
   char buffer[ 256];

   mulle_buffer_do_flexible( buf, buffer, sizeof( buffer))
   {
      mulle_buffer_add_string( buf, "abc\0hidden");
      mulle_buffer_add_string( buf, "test");
      printf( "add_string_break: %zu\n", mulle_buffer_get_length( buf));
   }
}

int main()
{
   test_null_buffer();
   test_flushable_buffer_seek();
   test_add_bytes_fallback();
   test_add_string_various_lengths();
   test_add_string_edge_cases();
   test_set_seek_modes();
   test_readonly_buffer();
   test_null_buffer_inlines();
   test_remove_all();
   test_buffer_capacity();
   test_intersects();
   test_setlength();
   test_static_with_static_bytes();
   test_overflow_detection();
   test_get_string_on_overflow();
   test_get_string_void_buffer();
   test_set_seek_invalid_mode();
   test_init_with_int_max();
   test_seek_cur_overflow();
   test_add_string_alignment_1byte();
   test_add_string_alignment_7bytes();
   test_add_string_break_in_loop();

   return 0;
}
