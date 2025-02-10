#pragma once

#include <stdint.h>
#include <stddef.h>

extern const uint8_t index_html_gz_start[] asm("_binary_data_gzip_Index_html_gz_start");
extern const uint8_t index_html_gz_end[] asm("_binary_data_gzip_Index_html_gz_end");
const size_t index_html_gz_size = index_html_gz_end - index_html_gz_start;

extern const uint8_t favicon_ico_gz_start[] asm("_binary_data_gzip_favicon_ico_gz_start");
extern const uint8_t favicon_ico_gz_end[] asm("_binary_data_gzip_favicon_ico_gz_end");
const size_t favicon_ico_gz_size = favicon_ico_gz_end - favicon_ico_gz_start;
