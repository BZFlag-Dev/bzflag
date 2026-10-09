/* bzflag
 * Copyright (c) 1993-2024 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "ServerAuth.h"

/* system headers */
#include <cstring>

/* common implementation headers */
#include "TextUtils.h"
#include "ErrorHandler.h"

ServerAuth::ServerAuth()
{
}

ServerAuth::~ServerAuth()
{
}

void ServerAuth::requestToken(StartupInfo *info)
{
    startupInfo = info;

    std::string url = info->listServerURL;

    std::string msg = "action=GETTOKEN&callsign=";
    msg     += TextUtils::url_encode(info->callsign);
    msg     += "&password=";
    msg     += TextUtils::url_encode(info->password);
    if (info->serverName[0] != '\0')
    {
        msg += "&nameport=";
        msg += info->serverName;
        msg += ':';
        msg += std::to_string(info->serverPort);
    }
    setPostMode(msg);
    setURL(url);
    addHandle();
}

void ServerAuth::finalization(char *_data, unsigned int length, bool good)
{
    if (good)
    {
        char *base = (char *)_data;
        char *endS = base + length;
        const char tokenIdentifier[]   = "TOKEN: ";
        const char noTokenIdentifier[] = "NOTOK: ";
        const char errorIdentifier[]   = "ERROR: ";
        const char noticeIdentifier[]  = "NOTICE: ";

        while (base < endS)
        {
            // find next newline
            char* scan = base;
            while (scan < endS && *scan != '\n')
                scan++;

            // if no newline then no more complete replies
            if (scan >= endS)
                break;
            *scan++ = '\0';

            // look for TOKEN: and save token if found also look for NOTOK:
            // and record "badtoken" into the token string and print an
            // error
            if (strncmp(base, tokenIdentifier, strlen(tokenIdentifier)) == 0)
            {
                strncpy(startupInfo->token, (char *)(base + strlen(tokenIdentifier)),
                        TokenLen - 1);
                startupInfo->token[TokenLen - 1] = '\0';
#ifdef DEBUG
                std::vector<std::string> args;
                args.push_back(startupInfo->token);
                printError("got token: {1}", &args);
#endif
            }
            else if (!strncmp(base, noTokenIdentifier,
                              strlen(noTokenIdentifier)))
            {
                printError("ERROR: did not get token:");
                printError(base);
                strcpy(startupInfo->token, "badtoken\0");
            }
            else if (!strncmp(base, errorIdentifier, strlen(errorIdentifier)))
            {
                printError(base);
                strcpy(startupInfo->token, "badtoken\0");
            }
            else if (!strncmp(base, noticeIdentifier, strlen(noticeIdentifier)))
                printError(base);

            // next reply
            base = scan;
        }
    }
    else
        strcpy(startupInfo->token, "badtoken\0");
}
