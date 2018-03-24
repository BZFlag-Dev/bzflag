/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "VBO_Handler.h"

// System headers
#include <iostream>
#include <algorithm>
#ifdef _WANT_BACKTRACE
#include <execinfo.h>
#endif

#include "VBO_Manager.h"

VBO_Handler::VBO_Handler(int vboSize_) :
    vboSize(vboSize_)
{
    vboManager.registerHandler(this);
    freeVBOList.push_back({0, vboSize});
}

VBO_Handler::~VBO_Handler()
{
    vboManager.unregisterHandler(this);
}

void trace_and_abort()
{
#ifdef _WANT_BACKTRACE
    void *array[10];
    size_t size;
    char **strings;
    size_t i;
    size = backtrace (array, 10);
    strings = backtrace_symbols (array, size);
    printf ("Obtained %zd stack frames.\n", size);
    for (i = 0; i < size; i++)
        printf ("%s\n", strings[i]);
    free (strings);
#endif
    abort();
}

int VBO_Handler::vboAlloc(int Vsize)
{
    MemElement memElement;

    // First check if there is a chunk that fit precisely
    auto it = std::find_if(freeVBOList.begin(), freeVBOList.end(),
                           [&](auto& item)
    {
        return item.Vsize == Vsize;
    });
    if (it == freeVBOList.end())
        // If not check if there is a chunk big enough
        it = std::find_if(freeVBOList.begin(), freeVBOList.end(),
                          [&](auto& item)
    {
        return item.Vsize > Vsize;
    });
    if (it == freeVBOList.end())
    {
        // Bad, not enough memory (or too fragmented)
        std::cout << vboName() << " requested " << Vsize << " vertexes" << std::endl;
        trace_and_abort ();
    }

    // Push a new element in the alloc list
    memElement.Vsize    = Vsize;
    memElement.vboIndex = it->vboIndex;
    alloVBOList.push_back(memElement);

    // Reduse the size of the old chunk
    it->Vsize    -= Vsize;
    it->vboIndex += Vsize;;
    // If nothing more drop from the Free List
    if (!it->Vsize)
        freeVBOList.erase(it);

    // return the address of the chunk
    return memElement.vboIndex;
}

void VBO_Handler::vboFree(int vboIndex)
{
    MemElement memElement;

    // Check if we allocated that index
    auto it = std::find_if(alloVBOList.begin(), alloVBOList.end(),
                           [&](auto& item)
    {
        return (item.vboIndex == vboIndex);
    });

    if (it == alloVBOList.end())
    {
        if (vboIndex < 0)
            return;
        // Bad, that index was never allocated
        std::cout << vboName() << " deallocated " << vboIndex << " never allocated" << std::endl;
        trace_and_abort ();
    }

    // Save the chunk and drop from the allocated list
    memElement.vboIndex = vboIndex;
    memElement.Vsize    = it->Vsize;
    alloVBOList.erase(it);

    // Check in the free list for a contiguous previous chunk
    it = std::find_if(freeVBOList.begin(), freeVBOList.end(),
                      [&](auto& item)
    {
        return (item.vboIndex + item.Vsize == memElement.vboIndex);
    });
    if (it != freeVBOList.end())
    {
        memElement.vboIndex = it->vboIndex;
        memElement.Vsize   += it->Vsize;
        freeVBOList.erase(it);
    }

    // Check in the free list for a contiguous successor chunk
    it = std::find_if(freeVBOList.begin(), freeVBOList.end(),
                      [&](auto& item)
    {
        return (item.vboIndex == memElement.vboIndex + memElement.Vsize);
    });
    if (it != freeVBOList.end())
    {
        memElement.Vsize   += it->Vsize;
        freeVBOList.erase(it);
    }
    freeVBOList.push_back(memElement);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
