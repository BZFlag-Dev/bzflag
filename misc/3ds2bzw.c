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
#include <lib3ds/mesh.h>
#include <lib3ds/vector.h>
#include <lib3ds/light.h>


static Lib3dsFile* File3DS = NULL;
static int UseDiffuse = 0;

static double NormalDir = +1.0;


//////////////////////////////////////////////////////////////////////////////

int main (int argc, char **argv)
{
  Lib3dsMesh* mesh;
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

  // load the file
  File3DS = lib3ds_file_load (argv[1]);
  if (File3DS == NULL) {
    printf ("Problems loading file\n");
    return 1;
  }

  // evaluate the first frame
  lib3ds_file_eval (File3DS, 0.0f /* the frame time */);
  
  // dump all of the meshes
  for (mesh = File3DS->meshes; mesh != NULL; mesh = mesh->next) {
    // FIXME - Lib3dsMatrix* matrix = &mesh->matrix;
    // comments on statistics
    unsigned int i;
    printf ("mesh  # %s\n", mesh->name);
    printf ("# vertices:  %i\n", (int)mesh->points);
    printf ("# normals:   %i\n", (int)mesh->faces * 3);
    printf ("# texcoords: %i\n", (int)mesh->texels);
    printf ("# faces:     %i\n", (int)mesh->faces);

    // vertices
    for (i = 0; i < mesh->points; i++) {
      Lib3dsPoint* point = &(mesh->pointL[i]);
      printf ("  vertex %f %f %f  # %i\n", 
              point->pos[0], point->pos[1], point->pos[2], i);
    }

    // normals  (cheat for now, flat normals)
    Lib3dsVector* normals = (Lib3dsVector*)
      malloc (3 * mesh->faces * sizeof(Lib3dsVector));
    lib3ds_mesh_calculate_normals (mesh, normals);
    for (i = 0; i < (mesh->faces * 3); i++) {
      printf ("  normal %f %f %f  # %i\n",
              normals[i][0], normals[i][1], normals[i][2], i);
    }
    free (normals);

    // texcoords
    for (i = 0; i < mesh->texels; i++) {
      Lib3dsTexel* texel = &(mesh->texelL[i]);
      printf ("  texcoord %f %f  # %i\n", *texel[0], *texel[1], i);
    }

    // faces
    for (i = 0; i < mesh->faces; i++) {
      Lib3dsFace* face = &(mesh->faceL[i]);
      Lib3dsWord* points = face->points;
      printf ("  face  # material = %s\n", face->material);
      printf ("    vertices %i %i %i\n", points[0], points[1], points[2]);
      printf ("    normals %i %i %i\n", (i * 3) + 0, (i * 3) + 1, (i * 3) + 2);
      Lib3dsMaterial* mat =
        lib3ds_file_material_by_name(File3DS, face->material);
      if (mat) {
        if (mesh->texels != 0) {
          printf ("    texture %s\n", mat->texture1_map.name);
          printf ("    texcoords %i %i %i\n", points[0], points[1], points[2]);

          // BZ isn't ready for these, yet... 
          // printf ("  #texture %s\n", material->texture2_map.name);
          // printf ("  #texture %s\n", material->texture1_mask.name);
          // printf ("  #texture %s\n", material->texture2_mask.name);
          
        }
        printf ("    ambient %f %f %f %f\n", mat->ambient[0],
                mat->ambient[1], mat->ambient[2], mat->ambient[3]);
        printf ("    diffuse %f %f %f %f\n", mat->diffuse[0],
                mat->diffuse[1], mat->diffuse[2], mat->diffuse[3]);
        printf ("    specular %f %f %f %f\n", mat->specular[0],
                mat->specular[1], mat->specular[2], mat->specular[3]);
        printf ("    shininess %f\n", mat->shininess);
      }
      printf ("  endface\n");
    }

    printf ("end  # %s\n\n", mesh->name); 
  }

  lib3ds_file_free (File3DS);

  return 0;
}

//////////////////////////////////////////////////////////////////////////////
