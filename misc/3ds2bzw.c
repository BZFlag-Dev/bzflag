//
// 3DS2BZW
//
// Author:  Dave Rodgers  (bzaka: trepan)
// Date:    Aug 26, 2004
//
// Utility program to convert 3DS model files
// into BZFlag 1.12 tetrahedron patterns.
//
// To use the program, you might type this:
//
//   ./3ds2bzw model.3ds > model.bzw
//
// NOTE:  this program requires lib3ds to
//        compile. to compile the program,
//        use something like this:
//
//        gcc -O3 -W -Wall -o 3ds2bzw 3ds2bzw.c -l3ds -lm
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <lib3ds/file.h>
#include <lib3ds/camera.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/vector.h>
#include <lib3ds/light.h>


static void printNode (Lib3dsNode *node);

static Lib3dsFile* File3DS = NULL;
static int UseDiffuse = 0;

static double NormalDir = +1.0;


//////////////////////////////////////////////////////////////////////////////

int main (int argc, char **argv)
{
  Lib3dsNode* node;
  const char* execname = argv[0];

  while (argc > 1) {
    if (strcmp ("-i", argv[1]) == 0) {
      NormalDir = -1.0;
      argc--;
      argv++;
      continue;
    }
    else if (strcmp ("-d", argv[1]) == 0) {
      UseDiffuse = 1;
      argc--;
      argv++;
      continue;
    }
    else {
      break;
    }
  }

  if (argc != 2) {
    printf ("\n");
    printf ("usage:  %s [-i] [-d] <filename>\n", execname);
    printf ("  -i : invert the plane normals\n");
    printf ("  -d : use diffuse colors instead of ambient\n");
    printf ("\n");
    return 1;
  }

  File3DS = lib3ds_file_load (argv[1]);
  if (File3DS == NULL) {
    printf ("Problems loading file\n");
    return 1;
  }

  lib3ds_file_eval (File3DS, 0.0f /* the frame time */);

  for (node = File3DS->nodes; node != NULL; node = node->next) {
    printNode (node);
  }

  lib3ds_file_free (File3DS);

  return 0;
}

//////////////////////////////////////////////////////////////////////////////

static void printNode (Lib3dsNode *node)
{
  Lib3dsNode* child;
  for (child = node->childs; child != NULL; child = child->next) {
    printNode (child);
  }

  if (node->type == LIB3DS_OBJECT_NODE) {

    if (strcmp(node->name,"$$$DUMMY") == 0) {
      return;
    }

    if (!node->user.d) {
      node->user.d = 1;
      Lib3dsMesh* mesh = lib3ds_file_mesh_by_name (File3DS, node->name);
      if (mesh == NULL) {
        return;
      }

      Lib3dsVector* normalL = (Lib3dsVector *)
        malloc (3 * sizeof (Lib3dsVector) * mesh->faces);
      lib3ds_mesh_calculate_normals (mesh, normalL);

      int f;
      for (f = 0; f < (int)mesh->faces; f++) {

        Lib3dsFace* face = &mesh->faceL[f];
        Lib3dsMaterial* mat = NULL;

        const float* vertices[3];
        vertices[0] = mesh->pointL[face->points[0]].pos;
        vertices[1] = mesh->pointL[face->points[1]].pos;
        vertices[2] = mesh->pointL[face->points[2]].pos;

        double edges[2][3];
        edges[0][0] = (double) (vertices[0][0] - vertices[1][0]);
        edges[0][1] = (double) (vertices[0][1] - vertices[1][1]);
        edges[0][2] = (double) (vertices[0][2] - vertices[1][2]);
        edges[1][0] = (double) (vertices[1][0] - vertices[2][0]);
        edges[1][1] = (double) (vertices[1][1] - vertices[2][1]);
        edges[1][2] = (double) (vertices[1][2] - vertices[2][2]);

        double cross[3];
        cross[0] = (edges[0][1] * edges[1][2]) - (edges[0][2] * edges[1][1]);
        cross[1] = (edges[0][2] * edges[1][0]) - (edges[0][0] * edges[1][2]);
        cross[2] = (edges[0][0] * edges[1][1]) - (edges[0][1] * edges[1][0]);

        double length = (cross[0] * cross[0]) +
                        (cross[1] * cross[1]) +
                        (cross[2] * cross[2]);

        length = sqrt (length);
        if (length < 0.000001) {
          fprintf (stderr, "Ditched face: length = %f\n", length);
          continue;
        }

        // normalize
        cross[0] = cross[0] / length;
        cross[1] = cross[1] / length;
        cross[2] = cross[2] / length;

        double center[3] = { 0.0f, 0.0f, 0.0f };

        printf ("\n");
        printf ("tetra\n");

        if (face->material[0]) {
          mat = lib3ds_file_material_by_name(File3DS, face->material);

          if (mat != NULL) {
            if (UseDiffuse) {
              printf ("  color %i %i %i %i # diffuse\n",
                       (int)(mat->diffuse[0] * 255.5f),
                       (int)(mat->diffuse[1] * 255.5f),
                       (int)(mat->diffuse[2] * 255.5f),
                       (int)(mat->diffuse[3] * 255.5f));
            } else {
              printf ("  color %i %i %i %i # ambient\n",
                       (int)(mat->ambient[0] * 255.5f),
                       (int)(mat->ambient[1] * 255.5f),
                       (int)(mat->ambient[2] * 255.5f),
                       (int)(mat->ambient[3] * 255.5f));
            }
          }
        }

        int v;
        for (v = 0; v < 3; v++) {
          const float* vertex = vertices[v];
          printf ("  vertex %f %f %f\n", vertex[0], vertex[1], vertex[2]);
          center[0] = center[0] + vertex[0];
          center[1] = center[1] + vertex[1];
          center[2] = center[2] + vertex[2];
        }

        center[0] = center[0] / 3.0;
        center[1] = center[1] / 3.0;
        center[2] = center[2] / 3.0;

        double maxLength = -1.0e38;
        double minLength = +1.0e38;
        for (v = 0; v < 3; v++) {
          double outwards[3];
          outwards[0] = vertices[v][0] - center[0];
          outwards[1] = vertices[v][1] - center[1];
          outwards[2] = vertices[v][2] - center[2];

          double tmpLength = (outwards[0] * outwards[0]) +
                             (outwards[1] * outwards[1]) +
                             (outwards[2] * outwards[2]);
          tmpLength = sqrt (tmpLength);

          if (tmpLength > maxLength) {
            maxLength = tmpLength;
          }
          if (tmpLength < minLength) {
            minLength = tmpLength;
          }
        }

        double fourthLength;
        if (maxLength > (0.1 * minLength)) {
          fourthLength = minLength;
        } else {
          fourthLength = maxLength;
        }
        fourthLength = fourthLength * (0.1 * NormalDir);

        double fourth[3];
        fourth[0] = center[0] + (cross[0] * fourthLength);
        fourth[1] = center[1] + (cross[1] * fourthLength);
        fourth[2] = center[2] + (cross[2] * fourthLength);
        printf ("  vertex %f %f %f\n", fourth[0], fourth[1], fourth[2]);

        printf ("  visible 0 0 0 1\n");
        printf ("end\n");
      }
      free (normalL);
    }
  }
  return;
}

//////////////////////////////////////////////////////////////////////////////


