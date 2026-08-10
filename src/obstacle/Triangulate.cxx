/* bzflag
 * Copyright (c) 1993-2025 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


// top dog
#include "common.h"

// implementation header
#include "Triangulate.h"

// system headers
#include <string.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// triangulation parameters
static glm::vec3 Normal; // FIXME, uNormal, vNormal;
static const glm::vec3* const *Verts = nullptr;
static int Count = 0;
static int* WorkSet = NULL;


static void vec3norm(glm::vec3& v)
{
    const float len = glm::length(v);
    if (len < 1.0e-6f)
    {
        v = glm::vec3(0.0f);
        return;
    }
    else
    {
        const float scale = 1.0f / len;
        v *= scale;
    }
}


static inline void makeNormal()
{
    // Newell method
    glm::vec3 normal(0.0f);
    for (int i = 0; i < Count; i++)
    {
        glm::vec3 v0 = *Verts[i];
        glm::vec3 v1 = *Verts[(i + 1) % Count];

        normal.x += ((v0.y - v1.y) * (v0.z + v1.z));
        normal.y += ((v0.z - v1.z) * (v0.x + v1.x));
        normal.z += ((v0.x - v1.x) * (v0.y + v1.y));
    }

    // normalize
    vec3norm(normal);

    Normal = normal;
}


static inline bool isConvex(int w0, int w1, int w2)
{
    // caution: faces can fold around the normal
    const int v0 = WorkSet[w0];
    const int v1 = WorkSet[w1];
    const int v2 = WorkSet[w2];
    const glm::vec3& p0 = *Verts[v0];
    const glm::vec3& p1 = *Verts[v1];
    const glm::vec3& p2 = *Verts[v2];
    glm::vec3 e0 = p1 - p0;
    glm::vec3 e1 = p2 - p1;
    if (glm::dot(glm::cross(e0, e1), Normal) <= 0.0f)
        return false;
    return true;
}


static inline bool isFaceClear(int w0, int w1, int w2)
{
    int i;
    const int v0 = WorkSet[w0];
    const int v1 = WorkSet[w1];
    const int v2 = WorkSet[w2];

    const glm::vec3& vert0 = *Verts[v0];
    const glm::vec3& vert1 = *Verts[v1];
    const glm::vec3& vert2 = *Verts[v2];
    // setup the edges
    glm::vec3 edges[3];
    edges[0] = vert1 - vert0;
    edges[1] = vert2 - vert1;
    edges[2] = vert0 - vert2;

    // get the triangle normal
    glm::vec3 normal = glm::cross(edges[0], edges[1]);

    // setup the planes
    glm::vec4 planes[3];
    glm::vec3 p0 = glm::cross(edges[0], normal);
    glm::vec3 p1 = glm::cross(edges[1], normal);
    glm::vec3 p2 = glm::cross(edges[2], normal);
    planes[0] = glm::vec4(p0, -glm::dot(p0, vert0));
    planes[1] = glm::vec4(p1, -glm::dot(p1, vert1));
    planes[2] = glm::vec4(p2, -glm::dot(p2, vert2));

    for (int w = 0; w < Count; w++)
    {
        if ((w == w0) || (w == w1) || (w == w2))
        {
            continue; // FIXME: lazy
        }
        const int v = WorkSet[w];
        const glm::vec3& vertV = *Verts[v];

        for (i = 0; i < 3; i++)
        {
            const float dist = glm::dot(glm::vec3(planes[i]), vertV) + planes[i].w;
            if (dist > 0.0f)
            {
                break; // this point is clear
            }
        }
        if (i == 3)
            return false;
    }
    return true;
}


static inline float getDot(int w0, int w1, int w2)
{
    const int v0 = WorkSet[w0];
    const int v1 = WorkSet[w1];
    const int v2 = WorkSet[w2];
    glm::vec3 e0 = *Verts[v1] - *Verts[v0];
    glm::vec3 e1 = *Verts[v2] - *Verts[v1];

    vec3norm(e0);
    vec3norm(e1);
    return glm::dot(e0, e1);
}


std::vector<TriIndices> triangulateFace(int count, const glm::vec3* const* verts)
{
    std::vector<TriIndices> tris;

    Verts = verts;
    Count = count;
    WorkSet = new int[Count];
    for (int i = 0; i < Count; i++)
        WorkSet[i] = i;
    makeNormal();

    int best = 0;
    bool left = false;
    bool first = true;
    float score = 0.0f;

    while (Count >= 3)
    {
        bool convex = false;
        bool faceClear = false;

        int offset;
        if (best == Count)
            offset = Count - 1;
        else
            offset = (best % Count);

        // stripping pattern
        if (left)
            offset = (offset + (Count - 1)) % Count;
        left = !left;

        // find the best triangle
        for (int w = offset; w < offset + (Count - 2); w++)
        {
            const int w0 = (w + 0) % Count;
            const int w1 = (w + 1) % Count;
            const int w2 = (w + 2) % Count;

            const bool convex2 = isConvex(w0, w1, w2);
            if (convex && !convex2)
                continue;

            const bool faceClear2 = isFaceClear(w0, w1, w2);
            if ((faceClear && !faceClear2) && (convex || !convex2))
                continue;

            if (first)
            {
                const float score2 = 2.0f - getDot(w0, w1, w2);
                if ((score2 < score) &&
                        (convex || !convex2) && (faceClear || !faceClear2))
                    continue;
                else
                    score = score2;
            }

            best = w0;
            if (convex && faceClear)
                break;
            convex = convex2;
            faceClear = faceClear2;
        }

        first = false;

        // add the triangle
        TriIndices ti;
        ti.indices[0] = WorkSet[(best + 0) % Count];
        ti.indices[1] = WorkSet[(best + 1) % Count];
        ti.indices[2] = WorkSet[(best + 2) % Count];
        tris.push_back(ti);

        // remove the middle vertex
        const int m = (best + 1) % Count;
        memmove(WorkSet + m, WorkSet + m + 1, (Count - m - 1) * sizeof(int));
        Count--;
    }

    delete[] WorkSet;

    return tris;
}


// Local variables: ***
// mode: C++ ***
// tab-width: 4***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
