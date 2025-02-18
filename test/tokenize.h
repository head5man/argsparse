#pragma once

#if defined( __cplusplus )
extern "C"
{
#endif

int tokenise_to_argc_argv(
    char     *buffer,     ///< In/Out : Modifiable String Buffer To Tokenise
    int      *argc,       ///< Out    : Argument Count
    char     *argv[],     ///< Out    : Argument String Vector Array
    const int argv_length,///< In     : Maximum Count For `*argv[]`
    void (*print)         ///< In     : Print results
        (char* const* argv, int argc));

#if defined( __cplusplus )
}
#endif