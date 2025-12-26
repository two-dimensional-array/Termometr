#ifndef __WEB_PAGES_H__
#define __WEB_PAGES_H__

#include <stddef.h>

typedef struct
{
    const char* uri;
    const char* content_start;
    const char* content_end;
    const char* content_type;
} web_page_t;

extern const web_page_t WEB_PAGES[];
extern const size_t WEB_PAGES_COUNT;

#endif // __WEB_PAGES_H__
