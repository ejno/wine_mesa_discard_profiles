// for dlvsym()
#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dlfcn.h>

#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>


static void
bomb(const char * const message)
{
    fprintf(stderr, "\nwine_mesa_discard_profiles wrapper: %s\n", message);
    exit(EXIT_FAILURE);
}


static int *
filter_attribs(const int * const attribs)
{
    const int *entry = NULL;
    size_t attribs_count = 0;
    int *result = NULL;
    size_t index = 0;

    for (entry = attribs; entry[0]; entry += 2) {
        if (entry[0] != GLX_CONTEXT_PROFILE_MASK_ARB) {
            ++attribs_count;
        }
    }

    result = (int *)calloc(2 * (1 + attribs_count), sizeof(*result));

    if (!result) {
        bomb("calloc() failed");
    }

    for (entry = attribs; entry[0]; entry += 2) {
        if (entry[0] != GLX_CONTEXT_PROFILE_MASK_ARB) {
            result[2 * index] = entry[0];
            result[2 * index + 1] = entry[1];
            ++index;
        }
    }

    return result;
}


static GLXContext
wrap_create_context(Display * const display,
                    const GLXFBConfig config,
                    const GLXContext share_context,
                    const Bool direct,
                    const int * const attribs)
{
    int * const filtered_attribs = filter_attribs(attribs);

    const GLXContext result = glXCreateContextAttribsARB(display,
                                                         config,
                                                         share_context,
                                                         direct,
                                                         filtered_attribs);

    free(filtered_attribs);

    return result;
}


typedef void (*fptr_get_proc_address_result)(void);


static fptr_get_proc_address_result
wrap_get_proc_address(const GLubyte * const name)
{
    if (0 == strcmp((const char *)name, "glXCreateContextAttribsARB")) {
        return (fptr_get_proc_address_result)wrap_create_context;
    }

    return glXGetProcAddressARB(name);
}


__attribute__((visibility("default"))) void *
dlsym(void * const handle, const char * const symbol)
{
    typedef void *(*fptr_dlsym)(void *, const char *);

    static fptr_dlsym original_dlsym = NULL;

    if (!original_dlsym) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        original_dlsym = (fptr_dlsym)dlvsym(RTLD_NEXT, "dlsym", "GLIBC_2.0");
#pragma GCC diagnostic pop

        if (!original_dlsym) {
            bomb("failed to find original dlsym()");
        }
    }

    if (0 == strcmp(symbol, "glXGetProcAddressARB")) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
        return (void *)wrap_get_proc_address;
#pragma GCC diagnostic pop
    }

    return original_dlsym(handle, symbol);
}
