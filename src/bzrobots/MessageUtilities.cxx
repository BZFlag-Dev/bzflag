#include "MessageUtilities.h"

bool MessageUtilities::parseFloat(char *string, float &dest)
{
    char *endptr;
    dest = strtof(string, &endptr);
    if (endptr == string)
        return false;

    /* We don't want NaN no matter what - it's of no use in this scenario.
     * (And strtof will allow the string "NAN" as NaN) */
    if (isnan(dest))
        dest = 0.0f;

    return true;
}
  
messageParseStatus MessageUtilities::parseSingleFloat(char **arguments, int count, float &dest)
{
  if (count != 1)
    return InvalidArgumentCount;
  if (!parseFloat(arguments[0], dest))
    return InvalidArguments;

  return ParseOk;
}
