/* for dlvsym() */
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
    fprintf(stderr, "\nwrapper: error: %s\n", message);
    abort();
}


typedef void *(*dlsym_type)(void *, const char *);


static dlsym_type
get_original_dlsym()
{
    static dlsym_type result = NULL;

    if (result) {
        return result;
    }

    result = (dlsym_type)dlvsym(RTLD_NEXT, "dlsym", "GLIBC_2.0");

    if (!result) {
        bomb("failed to find original dlsym()");
    }

    return result;
}


static size_t
get_non_profile_attribute_count(const int * const attributes)
{
    const int *entry = NULL;
    size_t result = 0;

    for (entry = attributes; entry[0]; entry += 2) {
        if (entry[0] != GLX_CONTEXT_PROFILE_MASK_ARB) {
            ++result;
        }
    }

    return result;
}


static int *
get_filtered_attributes(const int * const attributes)
{
    const int *entry = NULL;
    size_t index = 0;
    int * const result = (int *)calloc(
             2 * (1 + get_non_profile_attribute_count(attributes)),
             sizeof(*result));

    if (!result) {
        bomb("calloc() failed");
    }

    for (entry = attributes; entry[0]; entry += 2) {
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
                    const int * const attributes)
{
    int * const filtered_attributes = get_filtered_attributes(attributes);

    const GLXContext result = glXCreateContextAttribsARB(display,
                                                         config,
                                                         share_context,
                                                         direct,
                                                         filtered_attributes);

    free(filtered_attributes);

    return result;
}


typedef void (*get_proc_address_result_type)(void);


static get_proc_address_result_type
wrap_get_proc_address(const GLubyte * const name)
{
    if (0 == strcmp((const char *)name, "glXCreateContextAttribsARB")) {
        return (get_proc_address_result_type)wrap_create_context;
    }

    return glXGetProcAddressARB(name);
}


void *
dlsym(void * const handle, const char * const symbol)
{
    if (0 == strcmp(symbol, "glXGetProcAddressARB")) {
        return (void *)wrap_get_proc_address;
    }

    return get_original_dlsym()(handle, symbol);
}
