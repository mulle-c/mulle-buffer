#include <stdio.h>

#define mulle_buffer_assert_readable( buffer) \
    do { \
        if (((buffer)->_type & MULLE_BUFFER_IS_WRITEONLY)) { \
            printf("      assert failed: %s (type: %x)\n", \
                   __FUNCTION__, \
                   (buffer)->_type); \
        } else { \
            printf("      assert succeeded: %s (type: %x)\n", \
                   __FUNCTION__, \
                   (buffer)->_type); \
        }   \
    }       \
    while(0)

#define mulle_buffer_assert_writeable( buffer) \
    do { \
        if (((buffer)->_type & MULLE_BUFFER_IS_READONLY)) { \
            printf("      assert failed: %s (type: %x)\n", \
                   __FUNCTION__, \
                   (buffer)->_type); \
        } else { \
            printf("      assert succeeded: %s (type: %x)\n", \
                   __FUNCTION__, \
                   (buffer)->_type); \
        }   \
    }       \
    while(0)


#include <mulle-buffer/mulle-buffer.h>



void test_write_operations(struct mulle_buffer *buffer)
{
    char  *s;

    printf( "   write:\n");
    mulle_buffer_add_byte(buffer, 'A');
    mulle_buffer_add_string(buffer, "Test");
    mulle_buffer_add_char(buffer, 'B');
    mulle_buffer_add_uint16(buffer, 1234);
    mulle_buffer_add_uint32(buffer, 5678);
    mulle_buffer_memset(buffer, 'C', 5);
    mulle_buffer_zero_last_byte(buffer);

    // Additional write operations
    mulle_buffer_add_c_string(buffer, "Hello");
    mulle_buffer_add_c_chars(buffer, "World", 5);
    mulle_buffer_add_string_if_empty(buffer, "NotEmpty");
    mulle_buffer_add_string_if_not_empty(buffer, "Append");
    mulle_buffer_add_string_with_maxlength(buffer, "LongString", 5);
    mulle_buffer_make_string(buffer);
    mulle_buffer_remove_all(buffer);
    mulle_buffer_remove_in_range(buffer, mulle_range_make(0, 5));
    mulle_buffer_set_length(buffer, 10);
    mulle_buffer_advance(buffer, 5);
    mulle_buffer_add_buffer(buffer, NULL);
    mulle_buffer_add_buffer_range(buffer, NULL, mulle_range_make(0, 5));

    mulle_buffer_get_string(buffer);

    // write a zero if needed
    s = mulle_buffer_extract_string(buffer);
    mulle_free( s);
}

void test_read_operations(struct mulle_buffer *buffer)
{
    char tmp[ 10];

    printf( "   read:\n");

    mulle_buffer_get_last_byte(buffer);
    mulle_buffer_get_byte(buffer, 0);

    // Additional read operations
    mulle_buffer_get_bytes(buffer);
    mulle_buffer_next_byte(buffer);
    mulle_buffer_next_character(buffer);
    mulle_buffer_peek_byte(buffer);
    mulle_buffer_seek_byte(buffer, 'A');
    mulle_buffer_next_bytes(buffer, tmp, 10);
    mulle_buffer_reference_bytes(buffer, 5);
}

void test_harmless_operations(struct mulle_buffer *buffer)
{
    printf( "   harmless:\n");
    mulle_buffer_is_empty(buffer);
    mulle_buffer_get_length(buffer);
    mulle_buffer_get_bytes(buffer);

    // Additional harmless operations
    mulle_buffer_get_capacity(buffer);
    mulle_buffer_get_allocator(buffer);
    mulle_buffer_get_seek(buffer);
    mulle_buffer_set_seek(buffer, SEEK_SET, 0);
    mulle_buffer_get_staticlength(buffer);
}


void  test( int readonly)
{
    struct mulle_buffer buffer;
    mulle_buffer_init_default(&buffer);
    if( readonly)
    {
       mulle_buffer_set_readonly(&buffer);
       printf( "READONLY:\n");
    }
    else
    {
       mulle_buffer_set_writeonly(&buffer);
       printf( "WRITEONLY:\n");
    }

    test_write_operations(&buffer);
    test_read_operations(&buffer);
    test_harmless_operations(&buffer);
    
    mulle_buffer_done(&buffer);
}


int  main( void)
{
   test( 0);
   test( 1);
   return( 0);
}