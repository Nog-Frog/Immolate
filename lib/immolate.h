#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mach-o/dyld.h> // For _NSGetExecutablePath
#ifdef _WIN32
    #include <windows.h>
    #define PATH_SEPARATOR "\\"
#else
    // Define undefined MAX_PATH in Linux
    #define MAX_PATH (1024)
    #define PATH_SEPARATOR "/"

    #include <unistd.h>
    #include <libgen.h>

    /* Use unsafe not _s functions
     * An alternative is using safeclib implementation with the following:
     *
     * #define __STDC_WANT_LIB_EXT1__ 1
     * #include <safeclib/safe_str_lib.h>
     * 
     * Unfortunately, the strcat_s and strcpy_s do not work for malloced strings
     */
    #define strcat_s(a,b,c) strcat(a,c)
    #define strcpy_s(a,b,c) strcpy(a,c)
    #define printf_s(...) printf(__VA_ARGS__)
    #define fprintf_s(...) fprintf(__VA_ARGS__)
#endif
#include <limits.h>
#include <string.h>
#include <OpenCL/cl.h>
#define MAX_CODE_SIZE (1000000)

void clErrCheck(cl_int err, char* msg) {
    if (err != CL_SUCCESS) {
        printf_s("Fatal CL Error %d when trying to execute %s\n", err, msg);
        exit(EXIT_FAILURE);
    }
}

void getExecutableDir(char *dir) {
    #ifdef _WIN32
        // Windows specific code
        if (GetModuleFileName(NULL, dir, MAX_PATH) != 0) {
            char* last_slash = strrchr(dir, '\\');
            if (last_slash != NULL) {
                *last_slash = '\0';
            }
        } else {
            fprintf(stderr, "Error: Unable to get the current working directory\n");
        }
    #elif __linux__
        // Linux specific code
        ssize_t len = readlink("/proc/self/exe", dir, (size_t)(MAX_PATH - 1));
        if (len != -1) {
            dir[len] = '\0';
            char* last_slash = strrchr(dir, '/');
            if (last_slash != NULL) {
                *last_slash = '\0';
            }
        } else {
            fprintf(stderr, "Error: Unable to get the current working directory\n");
        }
    #elif __APPLE__
        // macOS specific code
        uint32_t size = (uint32_t)(MAX_PATH);
        if (_NSGetExecutablePath(dir, &size) == 0) {
            char *real_path = realpath(dir, NULL); // Resolve symbolic links
            if (real_path != NULL) {
                strcpy(dir, real_path);
                free(real_path); // Free memory allocated by realpath
                char* last_slash = strrchr(dir, '/');
                if (last_slash != NULL) {
                    *last_slash = '\0';
                }
            } else {
                fprintf(stderr, "Error: Unable to resolve executable path\n");
            }
        } else {
            fprintf(stderr, "Error: Buffer size too small for executable path\n");
        }
    #else
        #error Platform not supported
    #endif
}