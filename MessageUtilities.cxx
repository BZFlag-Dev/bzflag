#include "MessageUtilities.h"

template <>
bool MessageUtilities::parse(const char *string, bool &dest)
{
    if (strcasecmp(string, "true") == 0)
        dest = true;
    else if (strcasecmp(string, "false") == 0)
        dest = false;
    else
        return false;

    return true;
}
template <>
bool MessageUtilities::parse(const char *string, float &dest)
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
