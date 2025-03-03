/*******************************************************************************
  Tokenise String Buffer To Argc and Argv Style Format
  Brian Khuu 2017
*******************************************************************************/
#include <stdio.h>  // printf()
#include <ctype.h>  // isprint()
#include <string.h> // strtok()

/**-----------------------------------------------------------------------------
  @brief Tokenise a string buffer into argc and argv format

  Tokenise string buffer to argc and argv form via strtok_r()
  Warning: Using strtok_r will modify the string buffer

  Returns: Number of tokens extracted

------------------------------------------------------------------------------*/
int tokenise_to_argc_argv(
        char     *buffer,     ///< In/Out : Modifiable String Buffer To Tokenise
        int      *argc,       ///< Out    : Argument Count
        char     *argv[],     ///< Out    : Argument String Vector Array
        const int argv_length,///< In     : Maximum Count For `*argv[]`
        void (*print)         ///< In     : Print results
            (char* const* argv, int argc)
      )
{ /* Tokenise string buffer into argc and argv format (req: string.h) */
  int i = 0;
  argv[0] = strtok(buffer, " ");
  for (i = 1; i < argv_length; i++)
  { /* Fill argv via strtok_r() */
    if ( NULL == (argv[i] = strtok(NULL, " ")) ) break;
  }
  *argc = i;
  if (print)
    print(argv, *argc);
  return i; // Argument Count
}