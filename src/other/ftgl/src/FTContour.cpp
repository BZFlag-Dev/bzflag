/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 *               2008 Sam Hocevar <sam@zoy.org>
 *               2008 Éric Beets <ericbeets@free.fr>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include "FTContour.h"

#include <math.h>

static const float BEZIER_STEP_SIZE = 0.2f;


void FTContour::AddPoint(FTPoint point)
{
    if(pointList.empty() || (point != pointList[pointList.size() - 1]
                              && point != pointList[0]))
    {
        pointList.push_back(point);
    }
}


void FTContour::AddOutsetPoint(FTPoint point)
{
    outsetPointList.push_back(point);
}


void FTContour::AddFrontPoint(FTPoint point)
{
    frontPointList.push_back(point);
}


void FTContour::AddBackPoint(FTPoint point)
{
    backPointList.push_back(point);
}


void FTContour::evaluateQuadraticCurve(FTPoint A, FTPoint B, FTPoint C)
{
    for(unsigned int i = 0; i <= (1.0f / BEZIER_STEP_SIZE); i++)
    {
        float t = static_cast<float>(i) * BEZIER_STEP_SIZE;

        FTPoint U = (1.0f - t) * A + t * B;
        FTPoint V = (1.0f - t) * B + t * C;

        AddPoint((1.0f - t) * U + t * V);
    }
}

void FTContour::evaluateCubicCurve(FTPoint A, FTPoint B, FTPoint C, FTPoint D)
{
    for(unsigned int i = 0; i <= (1.0f / BEZIER_STEP_SIZE); i++)
    {
        float t = static_cast<float>(i) * BEZIER_STEP_SIZE;

        FTPoint U = (1.0f - t) * A + t * B;
        FTPoint V = (1.0f - t) * B + t * C;
        FTPoint W = (1.0f - t) * C + t * D;

        FTPoint M = (1.0f - t) * U + t * V;
        FTPoint N = (1.0f - t) * V + t * W;

        AddPoint((1.0f - t) * M + t * N);
    }
}

FTGL_DOUBLE FTContour::NormVector(const FTPoint &v)
{
    return sqrt(v.X() * v.X() + v.Y() * v.Y());
}

void FTContour::RotationMatrix(const FTPoint &a, const FTPoint &b, FTGL_DOUBLE *matRot, FTGL_DOUBLE *invRot)
{
    FTPoint abVect(b.X() - a.X(), b.Y() - a.Y(), 0);
    FTGL_DOUBLE abNorm = NormVector(abVect);
    invRot[0] = matRot[0] = -abVect.X() / abNorm;
    invRot[2] = matRot[1] = -abVect.Y() / abNorm;
    invRot[1] = matRot[2] =  abVect.Y() / abNorm;
    invRot[3] = matRot[3] = -abVect.X() / abNorm;
}

void FTContour::MultMatrixVect(FTGL_DOUBLE *mat, FTPoint &v)
{
    FTPoint res;
    res.X(v.X() * mat[0] + v.Y() * mat[1]);
    res.Y(v.X() * mat[2] + v.Y() * mat[3]);
    v.X(res.X());
    v.Y(res.Y());
}

void FTContour::ComputeBisec(FTPoint &v)
{
    FTGL_DOUBLE sgn = -64.0;
    if((v.Y() / NormVector(v)) < 0)
        sgn = 64.0;
    v.X(sgn * sqrt((NormVector(v) - v.X()) / (NormVector(v) + v.X())));
    v.Y(64.0);
}

FTPoint FTContour::ComputeOutsetPoint(FTPoint a, FTPoint b, FTPoint c)
{
    FTGL_DOUBLE mat[4], inv[4];
    /* Build the rotation matrix from 'ab' vector */
    RotationMatrix(b, a, mat, inv);
    /* 'h' is the second vector 'bc' */
    FTPoint h = c - b;
    /* Apply the rotation to the second vector 'bc' */
    MultMatrixVect(mat, h);
    /* Compute the vector bisecting 'bh' */
    ComputeBisec(h);
    /* Apply the inverted rotation matrix to 'bh' */
    MultMatrixVect(inv, h);
    return h;
}

void FTContour::outsetContour()
{
    size_t size = PointCount();
    FTPoint vOutset;
    for(unsigned int pointIndex = 0; pointIndex < size; ++pointIndex)
    {
        int prev = (pointIndex%size + size - 1) % size;
        int cur = pointIndex%size;
        int next = (pointIndex%size + 1) % size;
        /* Build the outset shape with d = 1.0f */
        vOutset = ComputeOutsetPoint(Point(prev), Point(cur), Point(next));
        AddOutsetPoint(vOutset);
    }
}

FTContour::FTContour(FT_Vector* contour, char* tags, unsigned int n)
{
    for(unsigned int i = 0; i < n; ++ i)
    {
        if(tags[i] == FT_Curve_Tag_On || n < 2)
        {
            AddPoint(FTPoint(contour[i]));
            continue;
        }

        FTPoint cur(contour[i]);
        FTPoint prev = (pointList.size() == 0 || i == 0)
                       ? FTPoint(contour[n - 1])
                       : pointList[pointList.size() - 1];
        FTPoint next = (i == n - 1)
                       ? (pointList.size() == 0)
                         ? FTPoint(contour[0])
                         : pointList[0]
                       : FTPoint(contour[i + 1]);

        if(tags[i] == FT_Curve_Tag_Conic)
        {
            while(tags[(i == n - 1) ? 0 : i + 1] == FT_Curve_Tag_Conic)
            {
                next = (cur + next) * 0.5f;

                evaluateQuadraticCurve(prev, cur, next);
                ++i;

                prev = next;
                cur = FTPoint(contour[i]);
                next = (i == n - 1)
                       ? pointList[0]
                       : FTPoint(contour[i + 1]);
            }

            evaluateQuadraticCurve(prev, cur, next);
            continue;
        }

        if(tags[i] == FT_Curve_Tag_Cubic)
        {
            FTPoint next2 = (i == n - 2)
                             ? pointList[0]
                             : FTPoint(contour[i + 2]);
            evaluateCubicCurve(prev, cur, next, next2);
            ++i;
            continue;
        }
    }

    /* Create (or not) front outset and/or back outset */
    outsetContour();
}

void FTContour::buildFrontOutset(float outset)
{
    for(size_t i = 0; i < PointCount(); ++i)
    {
        FTPoint point = FTPoint(Point(i).X() + Outset(i).X() * outset,
                                Point(i).Y() + Outset(i).Y() * outset,
                                0);
       AddFrontPoint(point);
    }
}
void FTContour::buildBackOutset(float outset)
{
    for(size_t i = 0; i < PointCount(); ++i)
    {
        FTPoint point = FTPoint(Point(i).X() + Outset(i).X() * outset,
                                Point(i).Y() + Outset(i).Y() * outset,
                                0);
       AddBackPoint(point);
    }
}

