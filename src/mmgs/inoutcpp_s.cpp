/* =============================================================================
**  This file is part of the mmg software package for the tetrahedral
**  mesh modification.
**  Copyright (c) Bx INP/CNRS/Inria/UBordeaux/UPMC, 2004-
**
**  mmg is free software: you can redistribute it and/or modify it
**  under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  mmg is distributed in the hope that it will be useful, but WITHOUT
**  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
**  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
**  License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License and of the GNU General Public License along with mmg (in
**  files COPYING.LESSER and COPYING). If not, see
**  <http://www.gnu.org/licenses/>. Please read their terms carefully and
**  use this copy of the mmg distribution only if you accept them.
** =============================================================================
*/

/**
 * \brief Input / Output Functions that needs cpp features.
 * \author Charles Dapogny (UPMC)
 * \author Cécile Dobrzynski (Bx INP/Inria/UBordeaux)
 * \author Pascal Frey (UPMC)
 * \author Algiane Froehly (Inria/UBordeaux)
 * \version 5
 * \copyright GNU Lesser General Public License.
 */

#include "vtkparser.hpp"
#include "libmmgs.h"
#include "libmmgs_private.h"

#ifdef USE_VTK

static int MMGS_loadVtkMesh_part2 ( MMG5_pMesh mesh,MMG5_pSol *sol,
                                    vtkDataSet **dataset, int8_t ptMeditRef,
                                    int8_t eltMeditRef,MMG5_int nsols,
                                    int8_t metricData, int8_t lsData ) {
  int ier;

  if ( !MMGS_zaldy(mesh) ) {
    return 0;
  }

  mesh->ne = mesh->nprism = 0;

  if ( !mesh->nt ) {
    fprintf(stderr,"  ** MISSING DATA.\n");
    fprintf(stderr," Check that your mesh contains triangles.\n");
    fprintf(stderr," Exit program.\n");
    return -1;
  }

  if (mesh->npmax < mesh->np || mesh->ntmax < mesh->nt ) {
    return -1;
  }

  ier = MMG5_loadVtkMesh_part2(mesh,sol,dataset,ptMeditRef,eltMeditRef,nsols,metricData,lsData);

  if ( ier < 1 ) {
    fprintf(stderr,"  ** ERROR WHEN PARSING THE INPUT FILE\n");
    return  ier;
  }

  return ier;
}

#endif

int MMGS_loadVtpMesh(MMG5_pMesh mesh,MMG5_pSol met,MMG5_pSol sol,const char *filename) {

#ifndef USE_VTK

  fprintf(stderr,"  ** VTK library not founded. Unavailable file format.\n");
  return -1;

#else
  int         ier;
  int         nsols;
  int8_t      ptMeditRef,eltMeditRef,metricData,lsData;
  vtkDataSet  *dataset;
  MMG5_pSol   allSol[2];

  mesh->dim = 3;

  ier = MMG5_loadVtpMesh_part1(mesh,filename,&dataset,&ptMeditRef,&eltMeditRef,
                               &nsols,&metricData,&lsData);
  if ( ier < 1 ) return ier;

  /* Check data fields */
  if ( nsols > (metricData+lsData) ) {
    fprintf(stderr,"Error: %d UNEXPECTED DATA FIELD(S)\n",nsols-metricData-lsData);
    return -1;
  }

  // Mesh alloc and transfer of the mesh from dataset toward the MMG5 Mesh Sol
  allSol[0] = met;
  allSol[1] = sol;
  ier = MMGS_loadVtkMesh_part2(mesh,allSol,&dataset,ptMeditRef,eltMeditRef,nsols,metricData,lsData);
  if ( ier < 1 ) {
    fprintf(stderr,"  ** ERROR WHEN PARSING THE INPUT FILE\n");
    return  ier;
  }

  if ( sol ) {
    /* Check the metric type */
    ier = MMG5_chkMetricType(mesh,&sol->type,&sol->entities,NULL);
  }

  return ier;
#endif
}

int MMGS_loadVtpMesh_and_allData(MMG5_pMesh mesh,MMG5_pSol *sol,const char *filename) {

#ifndef USE_VTK

  fprintf(stderr,"  ** VTK library not founded. Unavailable file format.\n");
  return -1;

#else
  int         ier;
  int         nsols;
  int8_t      ptMeditRef,eltMeditRef,metricData,lsData;
  vtkDataSet  *dataset;
  MMG5_pSol   allSol[2];

  mesh->dim = 3;

  ier = MMG5_loadVtpMesh_part1(mesh,filename,&dataset,&ptMeditRef,&eltMeditRef,
                               &nsols,&metricData,&lsData);
  if ( ier < 1 ) return ier;

  mesh->nsols = nsols;
  if ( *sol )  MMG5_DEL_MEM(mesh,*sol);
  MMG5_ADD_MEM(mesh,nsols*sizeof(MMG5_Sol),"solutions array",
                printf("  Exit program.\n");
                return -1);
  MMG5_SAFE_CALLOC(*sol,nsols,MMG5_Sol,return -1);

  // Mesh alloc and transfer of the mesh from dataset toward the MMG5 Mesh Sol
  allSol[0] = NULL;
  allSol[1] = *sol;
  ier = MMGS_loadVtkMesh_part2(mesh,allSol,&dataset,ptMeditRef,eltMeditRef,nsols,metricData,0);

  return ier;
#endif
}

int MMGS_loadVtkMesh(MMG5_pMesh mesh,MMG5_pSol met,MMG5_pSol sol,const char *filename) {

#ifndef USE_VTK

  fprintf(stderr,"  ** VTK library not founded. Unavailable file format.\n");
  return -1;

#else
  int         ier;
  int         nsols;
  int8_t      ptMeditRef,eltMeditRef,metricData,lsData;
  vtkDataSet  *dataset;
  MMG5_pSol   allSol[2];

  mesh->dim = 3;

  ier = MMG5_loadVtkMesh_part1(mesh,filename,&dataset,&ptMeditRef,&eltMeditRef,
                               &nsols,&metricData,&lsData);
  if ( ier < 1 ) return ier;

  /* Check data fields */
  if ( nsols > (metricData+lsData) ) {
    fprintf(stderr,"Error: %d UNEXPECTED DATA FIELD(S)\n",nsols-metricData-lsData);
    return -1;
  }

  // Mesh alloc and transfer of the mesh from dataset toward the MMG5 Mesh Sol
  allSol[0] = met;
  allSol[1] = sol;
  ier = MMGS_loadVtkMesh_part2(mesh,allSol,&dataset,ptMeditRef,eltMeditRef,nsols,metricData,lsData);
  if ( ier < 1 ) {
    fprintf(stderr,"  ** ERROR WHEN PARSING THE INPUT FILE\n");
    return  ier;
  }

  if ( sol ) {
    /* Check the metric type */
    ier = MMG5_chkMetricType(mesh,&sol->type,&sol->entities,NULL);
  }

  return ier;
#endif
}

int MMGS_loadVtkMesh_and_allData(MMG5_pMesh mesh,MMG5_pSol *sol,const char *filename) {

#ifndef USE_VTK

  fprintf(stderr,"  ** VTK library not founded. Unavailable file format.\n");
  return -1;

#else
  int         ier;
  int         nsols;
  int8_t      ptMeditRef,eltMeditRef,metricData,lsData;
  vtkDataSet *dataset;
  MMG5_pSol   allSol[2];

  mesh->dim = 3;

  ier = MMG5_loadVtkMesh_part1(mesh,filename,&dataset,&ptMeditRef,&eltMeditRef,
                               &nsols,&metricData,&lsData);
  if ( ier < 1 ) return ier;

  mesh->nsols = nsols;
  if ( *sol )  MMG5_DEL_MEM(mesh,*sol);
  MMG5_ADD_MEM(mesh,nsols*sizeof(MMG5_Sol),"solutions array",
                printf("  Exit program.\n");
                return -1);
  MMG5_SAFE_CALLOC(*sol,nsols,MMG5_Sol,return -1);

  // Mesh alloc and transfer of the mesh from dataset toward the MMG5 Mesh Sol
  allSol[0] = NULL;
  allSol[1] = *sol;
  ier = MMGS_loadVtkMesh_part2(mesh,allSol,&dataset,ptMeditRef,eltMeditRef,nsols,metricData,0);

  return ier;
#endif
}

int MMGS_loadVtuMesh(MMG5_pMesh mesh,MMG5_pSol met,MMG5_pSol sol,const char *filename) {

#ifndef USE_VTK

  fprintf(stderr,"  ** VTK library not founded. Unavailable file format.\n");
  return -1;

#else
  int         ier;
  int         nsols;
  int8_t      ptMeditRef,eltMeditRef,metricData,lsData;
  vtkDataSet  *dataset;
  MMG5_pSol   allSol[2];

  mesh->dim = 3;

  ier = MMG5_loadVtuMesh_part1(mesh,filename,&dataset,&ptMeditRef,&eltMeditRef,
                               &nsols,&metricData,&lsData);
  if ( ier < 1 ) return ier;

  /* Check data fields */
  if ( nsols > (metricData+lsData) ) {
    fprintf(stderr,"Error: %d UNEXPECTED DATA FIELD(S)\n",nsols-metricData-lsData);
    return -1;
  }

  // Mesh alloc and transfer of the mesh from dataset toward the MMG5 Mesh Sol
  allSol[0] = met;
  allSol[1] = sol;
  ier = MMGS_loadVtkMesh_part2(mesh,allSol,&dataset,ptMeditRef,eltMeditRef,nsols,metricData,lsData);
  if ( ier < 1 ) {
    fprintf(stderr,"  ** ERROR WHEN PARSING THE INPUT FILE\n");
    return  ier;
  }

  if ( sol ) {
    /* Check the metric type */
    ier = MMG5_chkMetricType(mesh,&sol->type,&sol->entities,NULL);
  }

  return ier;
#endif
}

int MMGS_loadVtuMesh_and_allData(MMG5_pMesh mesh,MMG5_pSol *sol,const char *filename) {

#ifndef USE_VTK

  fprintf(stderr,"  ** VTK library not founded. Unavailable file format.\n");
  return -1;

#else
  int         ier;
  int         nsols;
  int8_t      ptMeditRef,eltMeditRef,metricData,lsData;
  vtkDataSet  *dataset;
  MMG5_pSol   allSol[2];

  mesh->dim = 3;

  ier = MMG5_loadVtuMesh_part1(mesh,filename,&dataset,&ptMeditRef,&eltMeditRef,
                               &nsols,&metricData,&lsData);
  if ( ier < 1 ) return ier;

  mesh->nsols = nsols;
  if ( *sol )  MMG5_DEL_MEM(mesh,*sol);
  MMG5_ADD_MEM(mesh,nsols*sizeof(MMG5_Sol),"solutions array",
                printf("  Exit program.\n");
                return -1);
  MMG5_SAFE_CALLOC(*sol,nsols,MMG5_Sol,return -1);

  // Mesh alloc and transfer of the mesh from dataset toward the MMG5 Mesh Sol
  allSol[0] = NULL;
  allSol[1] = *sol;
  ier = MMGS_loadVtkMesh_part2(mesh,allSol,&dataset,ptMeditRef,eltMeditRef,nsols,metricData,0);

  return ier;
#endif
}

int MMGS_saveVtuMesh(MMG5_pMesh mesh,MMG5_pSol sol,const char *filename) {

#ifndef USE_VTK

  fprintf(stderr,"  ** VTK library not found. Unavailable file format.\n");
  return -1;

#else

  return MMG5_saveVtkMesh<vtkUnstructuredGrid,vtkXMLUnstructuredGridWriter,
                          vtkXMLPUnstructuredGridWriter>(mesh,&sol,filename,1,1);

#endif
}

int MMGS_saveVtuMesh_and_allData(MMG5_pMesh mesh,MMG5_pSol *sol,const char *filename) {

  MMG5_pSol  allSol[2];

#ifndef USE_VTK

  fprintf(stderr,"  ** VTK library not found. Unavailable file format.\n");
  return -1;

#else

  allSol[0] = NULL;
  allSol[1] = *sol;

  return MMG5_saveVtkMesh<vtkUnstructuredGrid,vtkXMLUnstructuredGridWriter,
                          vtkXMLPUnstructuredGridWriter>(mesh,allSol,filename,0,1);

#endif
}

int MMGS_saveVtkMesh(MMG5_pMesh mesh,MMG5_pSol sol,const char *filename) {

#ifndef USE_VTK

  fprintf(stderr,"  ** VTK library not found. Unavailable file format.\n");
  return -1;

#else

  return MMG5_saveVtkMesh<vtkUnstructuredGrid,vtkDataSetWriter,
                          vtkPDataSetWriter>(mesh,&sol,filename,1,0);

#endif
}

int MMGS_saveVtkMesh_and_allData(MMG5_pMesh mesh,MMG5_pSol *sol,const char *filename) {

  MMG5_pSol  allSol[2];

#ifndef USE_VTK

  fprintf(stderr,"  ** VTK library not found. Unavailable file format.\n");
  return -1;

#else

  allSol[0] = NULL;
  allSol[1] = *sol;

  return MMG5_saveVtkMesh<vtkUnstructuredGrid,vtkDataSetWriter,
                          vtkPDataSetWriter>(mesh,allSol,filename,0,0);

#endif
}

int MMGS_saveVtpMesh(MMG5_pMesh mesh,MMG5_pSol sol,const char *filename) {

#ifndef USE_VTK

  fprintf(stderr,"  ** VTK library not found. Unavailable file format.\n");
  return -1;

#else

  return MMG5_saveVtkMesh<vtkPolyData,vtkXMLPolyDataWriter,
                          vtkXMLPPolyDataWriter>(mesh,&sol,filename,1,1);

#endif
}

int MMGS_saveVtpMesh_and_allData(MMG5_pMesh mesh,MMG5_pSol *sol,const char *filename) {

  MMG5_pSol  allSol[2];

#ifndef USE_VTK

  fprintf(stderr,"  ** VTK library not found. Unavailable file format.\n");
  return -1;

#else

  allSol[0] = NULL;
  allSol[1] = *sol;

  return MMG5_saveVtkMesh<vtkPolyData,vtkXMLPolyDataWriter,
                          vtkXMLPPolyDataWriter>(mesh,allSol,filename,0,1);

#endif
}
