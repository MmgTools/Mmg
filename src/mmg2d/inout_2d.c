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
#include "mmg2d.h"

/* read mesh data */
int MMG2D_loadMesh(MMG5_pMesh mesh,const char *filename) {
  FILE        *inm;
  MMG5_pPoint       ppt;
  MMG5_pEdge        ped;
  MMG5_pTria        pt;
  MMG5_pQuad        pq1;
  float             fc;
  long         posnp,posnt,posncor,posned,posnq,posreq,posreqed,posntreq,posnqreq;
  int          k,ref,tmp,ncor,norient,nreq,ntreq,nreqed,bin,iswp,nqreq,nref;
  double       air,dtmp;
  int          i,bdim,binch,bpos;
  char         *ptr,*data;
  char         chaine[MMG5_FILESTR_LGTH],strskip[MMG5_FILESTR_LGTH];

  posnp = posnt = posncor = posned = posnq = posreq = posreqed = posntreq = posnqreq = 0;
  ncor = nreq = nreqed = ntreq = nqreq = 0;
  bin = 0;
  iswp = 0;
  mesh->np = mesh->nt = mesh->na = mesh->xp = 0;
  nref = 0;

  MMG5_SAFE_CALLOC(data,strlen(filename)+7,char,return 0);
  strcpy(data,filename);
  ptr = strstr(data,".mesh");
  if ( !ptr ) {
    strcat(data,".meshb");
    if (!(inm = fopen(data,"rb")) ) {
      ptr  = strstr(data,".mesh");
      *ptr = '\0';
      strcat(data,".mesh");
      if (!(inm = fopen(data,"rb")) ) {
        MMG5_SAFE_FREE(data);
        return 0;
      }
    }
    else  bin = 1;
  }
  else {
    ptr = strstr(data,".meshb");

    if ( ptr )  bin = 1;

    if( !(inm = fopen(data,"rb")) ) {
      MMG5_SAFE_FREE(data);
      return 0;
    }
  }
  if ( mesh->info.imprim >= 0 )
    fprintf(stdout,"  %%%% %s OPENED\n",data);
  MMG5_SAFE_FREE(data);

  if (!bin) {
    strcpy(chaine,"D");
    while(fscanf(inm,"%127s",&chaine[0])!=EOF && strncmp(chaine,"End",strlen("End")) ) {
      if ( chaine[0] == '#' ) {
        fgets(strskip,MMG5_FILESTR_LGTH,inm);
        continue;
      }

      if(!strncmp(chaine,"MeshVersionFormatted",strlen("MeshVersionFormatted"))) {
        MMG_FSCANF(inm,"%d",&mesh->ver);
        continue;
      }
      else if(!strncmp(chaine,"Dimension",strlen("Dimension"))) {
        MMG_FSCANF(inm,"%d",&mesh->dim);
        if(mesh->info.nreg==2) {
          if(mesh->dim!=3) {
            fprintf(stdout,"WRONG USE OF -msh \n");
            return 0;
          }
          mesh->dim = 2;
        }
        if(mesh->dim!=2) {
          fprintf(stdout,"BAD DIMENSION : %d\n",mesh->dim);
          return 0;
        }
        continue;
      }
      else if(!strncmp(chaine,"Vertices",strlen("Vertices"))) {
        MMG_FSCANF(inm,"%d",&mesh->np);
        posnp = ftell(inm);
        continue;
      }
      else if(!strncmp(chaine,"Triangles",strlen("Triangles"))) {
        MMG_FSCANF(inm,"%d",&mesh->nt);
        posnt = ftell(inm);
        continue;
      }
      else if(!strncmp(chaine,"Quadrilaterals",strlen("Quadrilaterals"))) {
        fscanf(inm,"%d",&mesh->nquad);
        posnq = ftell(inm);
        continue;
      }
      else if(!strncmp(chaine,"RequiredQuadrilaterals",strlen("RequiredQuadrilaterals"))) {
        MMG_FSCANF(inm,"%d",&nqreq);
        posnqreq = ftell(inm);
        continue;
      }
      else if(!strncmp(chaine,"Corners",strlen("Corners"))) {
        MMG_FSCANF(inm,"%d",&ncor);
        posncor = ftell(inm);
        continue;
      }
      else if(!strncmp(chaine,"RequiredVertices",strlen("RequiredVertices"))) {
        MMG_FSCANF(inm,"%d",&nreq);
        posreq = ftell(inm);
        continue;
      }
      else if(!strncmp(chaine,"Edges",strlen("Edges"))) {
        MMG_FSCANF(inm,"%d",&mesh->na);
        posned = ftell(inm);
        continue;
      }
      else if(!strncmp(chaine,"RequiredEdges",strlen("RequiredEdges"))) {
        MMG_FSCANF(inm,"%d",&nreqed);
        posreqed = ftell(inm);
        continue;
      }
      else if(!strncmp(chaine,"RequiredTriangles",strlen("RequiredTriangles"))) {
        MMG_FSCANF(inm,"%d",&ntreq);
        posntreq = ftell(inm);
        continue;
      }
    }
  }
  else {
    bdim = 0;
    MMG_FREAD(&mesh->ver,MMG5_SW,1,inm);
    iswp=0;
    if(mesh->ver==16777216)
      iswp=1;
    else if(mesh->ver!=1) {
      fprintf(stdout,"BAD FILE ENCODING\n");
    }
    MMG_FREAD(&mesh->ver,MMG5_SW,1,inm);
    if(iswp) mesh->ver = MMG5_swapbin(mesh->ver);
    while(fread(&binch,MMG5_SW,1,inm)!=0 && binch!=54 ) {
      if(iswp) binch=MMG5_swapbin(binch);
      if(binch==54) break;
      if(!bdim && binch==3) {  //Dimension
        MMG_FREAD(&bdim,MMG5_SW,1,inm);  //NulPos=>20
        if(iswp) bdim=MMG5_swapbin(bdim);
        MMG_FREAD(&bdim,MMG5_SW,1,inm);
        if(iswp) bdim=MMG5_swapbin(bdim);
        mesh->dim = bdim;
        if(bdim!=2) {
          fprintf(stdout,"BAD MESH DIMENSION : %d\n",mesh->dim);
          return 0;
        }
        continue;
      } else if(!mesh->np && binch==4) {  //Vertices
        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
        if(iswp) bpos=MMG5_swapbin(bpos);
        MMG_FREAD(&mesh->np,MMG5_SW,1,inm);
        if(iswp) mesh->np=MMG5_swapbin(mesh->np);
        posnp = ftell(inm);
        rewind(inm);
        fseek(inm,bpos,SEEK_SET);
        continue;
      }  else if(!mesh->nt && binch==6) {//MMG5_Triangles
        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
        if(iswp) bpos=MMG5_swapbin(bpos);
        MMG_FREAD(&mesh->nt,MMG5_SW,1,inm);
        if(iswp) mesh->nt=MMG5_swapbin(mesh->nt);
        posnt = ftell(inm);
        rewind(inm);
        fseek(inm,bpos,SEEK_SET);
        continue;
      }
      else if(binch==17) {  //RequiredTriangles
        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
        if(iswp) bpos=MMG5_swapbin(bpos);
        MMG_FREAD(&ntreq,MMG5_SW,1,inm);
        if(iswp) ntreq=MMG5_swapbin(ntreq);
        posntreq = ftell(inm);
        rewind(inm);
        fseek(inm,bpos,SEEK_SET);
        continue;
      } else if(!mesh->nquad && binch==7) {//Quadrilaterals
        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
        if(iswp) bpos=MMG5_swapbin(bpos);
        MMG_FREAD(&mesh->nquad,MMG5_SW,1,inm);
        if(iswp) mesh->nquad=MMG5_swapbin(mesh->nquad);
        posnq = ftell(inm);
        rewind(inm);
        fseek(inm,bpos,SEEK_SET);
        continue;
      } else if(binch==18) {  //RequiredQuadrilaterals
        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
        if(iswp) bpos=MMG5_swapbin(bpos);
        MMG_FREAD(&nqreq,MMG5_SW,1,inm);
        if(iswp) nqreq=MMG5_swapbin(nqreq);
        posnqreq = ftell(inm);
        rewind(inm);
        fseek(inm,bpos,SEEK_SET);
        continue;
      } else if(!ncor && binch==13) {
        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
        if(iswp) bpos=MMG5_swapbin(bpos);
        MMG_FREAD(&ncor,MMG5_SW,1,inm);
        if(iswp) ncor=MMG5_swapbin(ncor);
        posncor = ftell(inm);
        rewind(inm);
        fseek(inm,bpos,SEEK_SET);
        continue;
      } else if(!mesh->na && binch==5) { //Edges
        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
        if(iswp) bpos=MMG5_swapbin(bpos);
        MMG_FREAD(&mesh->na,MMG5_SW,1,inm);
        if(iswp) mesh->na=MMG5_swapbin(mesh->na);
        posned = ftell(inm);
        rewind(inm);
        fseek(inm,bpos,SEEK_SET);
        continue;
      } else if(!nreqed && binch==16) { //RequiredEdges
        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
        if(iswp) bpos=MMG5_swapbin(bpos);
        MMG_FREAD(&nreqed,MMG5_SW,1,inm);
        if(iswp) nreqed=MMG5_swapbin(nreqed);
        posreqed = ftell(inm);
        rewind(inm);
        fseek(inm,bpos,SEEK_SET);
        continue;
      } else if(!nreq && binch==15) { //RequiredVertices
        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
        if(iswp) bpos=MMG5_swapbin(bpos);
        MMG_FREAD(&nreq,MMG5_SW,1,inm);
        if(iswp) nreq=MMG5_swapbin(nreq);
        posreq = ftell(inm);
        rewind(inm);
        fseek(inm,bpos,SEEK_SET);
        continue;
      } else {
        //printf("on traite ? %d\n",binch);
        MMG_FREAD(&bpos,MMG5_SW,1,inm); //NulPos
        if(iswp) bpos=MMG5_swapbin(bpos);
        //printf("on avance... Nulpos %d\n",bpos);
        rewind(inm);
        fseek(inm,bpos,SEEK_SET);
      }
    }

  }

  if ( abs(mesh->info.imprim) > 5 )
    fprintf(stdout,"  -- READING DATA FILE %s\n",data);

  if ( !mesh->np  ) {
    fprintf(stdout,"  ** MISSING DATA : no point\n");
    return -1;
  }

  mesh->npi  = mesh->np;
  mesh->nai  = mesh->na;
  mesh->nti  = mesh->nt;
  if ( !mesh->np ) {
    fprintf(stdout,"  ** MISSING DATA\n");
    return -1;
  }

  /* Memory allocation */
  if ( !MMG2D_zaldy(mesh) )  return 0;

  /* Read vertices */
  rewind(inm);
  fseek(inm,posnp,SEEK_SET);
  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if (mesh->ver < 2) { /*float*/
      if (!bin) {
        if(mesh->info.nreg==2) {
          for (i=0 ; i<3 ; i++) {
            MMG_FSCANF(inm,"%f",&fc);
            if(i==2) break;
            ppt->c[i] = (double) fc;
          }
        } else {
          for (i=0 ; i<2 ; i++) {
            MMG_FSCANF(inm,"%f",&fc);
            ppt->c[i] = (double) fc;
          }
        }
        MMG_FSCANF(inm,"%d",&ppt->ref);
      } else {
        if(mesh->info.nreg==2) {
          fprintf(stderr,"  ## Warning: %s: binary not available with"
                  " -msh option.\n",__func__);
          return 0;
        }
        for (i=0 ; i<2 ; i++) {
          MMG_FREAD(&fc,MMG5_SW,1,inm);
          if(iswp) fc=MMG5_swapf(fc);
          ppt->c[i] = (double) fc;
        }
        MMG_FREAD(&ppt->ref,MMG5_SW,1,inm);
        if(iswp) ppt->ref=MMG5_swapbin(ppt->ref);
      }
    } else {
      if (!bin) {
        if(mesh->info.nreg==2) {
          MMG_FSCANF(inm,"%lf %lf %lf %d",&ppt->c[0],&ppt->c[1],&dtmp,&ppt->ref);
        } else {
          MMG_FSCANF(inm,"%lf %lf %d",&ppt->c[0],&ppt->c[1],&ppt->ref);
        }
      }
      else {
        for (i=0 ; i<2 ; i++) {
          MMG_FREAD(&ppt->c[i],MMG5_SD,1,inm);
          if(iswp) ppt->c[i]=MMG5_swapd(ppt->c[i]);
        }
        MMG_FREAD(&ppt->ref,MMG5_SW,1,inm);
        if(iswp) ppt->ref=MMG5_swapbin(ppt->ref);
      }
    }
    if ( ppt->ref < 0 ) {
      ppt->ref = -ppt->ref;
      ++nref;
    }
    ppt->tag = MG_NUL;
  }

  /* Read edges */
  rewind(inm);
  fseek(inm,posned,SEEK_SET);
  for (k=1; k<=mesh->na; k++) {
    ped = &mesh->edge[k];
    if (!bin) {
      MMG_FSCANF(inm,"%d %d %d",&ped->a,&ped->b,&ped->ref);
    }
    else {
      MMG_FREAD(&ped->a,MMG5_SW,1,inm);
      if(iswp) ped->a=MMG5_swapbin(ped->a);
      MMG_FREAD(&ped->b,MMG5_SW,1,inm);
      if(iswp) ped->b=MMG5_swapbin(ped->b);
      MMG_FREAD(&ped->ref,MMG5_SW,1,inm);
      if(iswp) ped->ref=MMG5_swapbin(ped->ref);
    }
    if ( ped->ref < 0 ) {
      ped->ref = -ped->ref;
      ++nref;
    }
    ped->tag |= MG_REF+MG_BDY;
  }

  /* Read triangles */
  if ( mesh->nt ) {
    rewind(inm);
    fseek(inm,posnt,SEEK_SET);
    norient = 0;
    for (k=1; k<=mesh->nt; k++) {
      pt = &mesh->tria[k];
      if (!bin) {
        MMG_FSCANF(inm,"%d %d %d %d",&pt->v[0],&pt->v[1],&pt->v[2],&pt->ref);
      }
      else {
        for (i=0 ; i<3 ; i++) {
          MMG_FREAD(&pt->v[i],MMG5_SW,1,inm);
          if(iswp) pt->v[i]=MMG5_swapbin(pt->v[i]);
        }
        MMG_FREAD(&pt->ref,MMG5_SW,1,inm);
        if(iswp) pt->ref=MMG5_swapbin(pt->ref);
      }
      for (i=0; i<3; i++) {
        ppt = &mesh->point[ pt->v[i] ];
        ppt->tag &= ~MG_NUL;
      }
      for(i=0 ; i<3 ; i++)
        pt->edg[i] = 0;

      /* Get positive ref */
      if ( pt->ref < 0 ) {
        pt->ref = -pt->ref;
        ++nref;
      }

      /* Check orientation */
      air = MMG2D_quickarea(mesh->point[pt->v[0]].c,mesh->point[pt->v[1]].c,
                           mesh->point[pt->v[2]].c);
      if(air < 0) {
        norient++;
        tmp = pt->v[2];
        pt->v[2] = pt->v[1];
        pt->v[1] = tmp;
      }
    }
    if ( norient ) {
      fprintf(stdout,"\n     $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ \n");
      fprintf(stdout,"         BAD ORIENTATION : vol < 0 -- %8d element(s) reoriented\n",norient);
      fprintf(stdout,"     $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ \n\n");
    }

    if ( ntreq ) {
      rewind(inm);
      fseek(inm,posntreq,SEEK_SET);
      for (k=1; k<=ntreq; k++) {
        if(!bin) {
          MMG_FSCANF(inm,"%d",&i);
        }
        else {
          MMG_FREAD(&i,MMG5_SW,1,inm);
          if(iswp) i=MMG5_swapbin(i);
        }
        if ( i>mesh->nt ) {
          fprintf(stderr,"\n  ## Warning: %s: required triangle number %8d"
                  " ignored.\n",__func__,i);
        } else {
          pt = &mesh->tria[i];
          pt->tag[0] |= MG_REQ;
          pt->tag[1] |= MG_REQ;
          pt->tag[2] |= MG_REQ;
        }
      }
    }
  }
  else {
    for (k=1; k<=mesh->np; k++) {
      ppt = &mesh->point[ k ];
      ppt->tag &= ~MG_NUL;
    }
  }

  /* read mesh quadrilaterals */
  if ( mesh->nquad ) {
    rewind(inm);
    fseek(inm,posnq,SEEK_SET);

    for (k=1; k<=mesh->nquad; k++) {
      pq1 = &mesh->quadra[k];
      if (!bin) {
        MMG_FSCANF(inm,"%d %d %d %d %d",&pq1->v[0],&pq1->v[1],&pq1->v[2],
               &pq1->v[3],&pq1->ref);
      }
      else {
        for (i=0 ; i<4 ; i++) {
          MMG_FREAD(&pq1->v[i],MMG5_SW,1,inm);
          if(iswp) pq1->v[i]=MMG5_swapbin(pq1->v[i]);
        }
        MMG_FREAD(&pq1->ref,MMG5_SW,1,inm);
        if(iswp) pq1->ref=MMG5_swapbin(pq1->ref);
      }
      for (i=0; i<4; i++) {
        ppt = &mesh->point[ pq1->v[i] ];
        ppt->tag &= ~MG_NUL;
      }

      if ( pq1->ref < 0 ) {
        pq1->ref = -pq1->ref;
        ++nref;
      }
    }
    /* get required quadrilaterals */
    if(nqreq) {
      rewind(inm);
      fseek(inm,posnqreq,SEEK_SET);
      for (k=1; k<=nqreq; k++) {
        if(!bin) {
          MMG_FSCANF(inm,"%d",&i);
        }
        else {
          MMG_FREAD(&i,MMG5_SW,1,inm);
          if(iswp) i=MMG5_swapbin(i);
        }
        if ( i>mesh->nquad ) {
          fprintf(stderr,"\n  ## Warning: %s: required quadrilaterals number"
                  " %8d ignored.\n",__func__,i);
        } else {
          pq1 = &mesh->quadra[i];
          pq1->tag[0] |= MG_REQ;
          pq1->tag[1] |= MG_REQ;
          pq1->tag[2] |= MG_REQ;
          pq1->tag[3] |= MG_REQ;
        }
      }
    }
  }

  /* Read corners */
  if ( ncor ) {
    rewind(inm);
    fseek(inm,posncor,SEEK_SET);
    for (k=1; k<=ncor; k++) {
      if (!bin) {
        MMG_FSCANF(inm,"%d",&ref);
      }
      else {
        MMG_FREAD(&ref,MMG5_SW,1,inm);
        if(iswp) ref=MMG5_swapbin(ref);
      }
      ppt = &mesh->point[ref];
      ppt->tag |= MG_CRN;
    }
  }

  /* Read required vertices*/
  if (nreq) {
    rewind(inm);
    fseek(inm,posreq,SEEK_SET);
    for (k=1; k<=nreq; k++) {
      if (!bin) {
        MMG_FSCANF(inm,"%d",&ref);
      }
      else {
        MMG_FREAD(&ref,MMG5_SW,1,inm);
        if(iswp) ref=MMG5_swapbin(ref);
      }
      ppt = &mesh->point[ref];
      ppt->tag |= MG_REQ;
    }
  }

  /* read required edges*/
  if (nreqed) {
    rewind(inm);
    fseek(inm,posreqed,SEEK_SET);
    for (k=1; k<=nreqed; k++) {
      if (!bin) {
        MMG_FSCANF(inm,"%d",&ref);
      }
      else {
        MMG_FREAD(&ref,MMG5_SW,1,inm);
        if(iswp) ref=MMG5_swapbin(ref);
      }
      ped = &mesh->edge[ref];
      ped->tag |= MG_REQ;
    }
  }

  fclose(inm);

  /*maill periodique : remettre toutes les coord entre 0 et 1*/
  if(mesh->info.renum==-10) {
    if ( mesh->info.imprim > 4 || mesh->info.ddebug )
      printf("  ## Periodic mesh: %d points %d triangles\n",mesh->np,mesh->nt);
    for (k=1; k<=mesh->np; k++) {
      ppt = &mesh->point[k];
      while (ppt->c[0] > 1 + 5e-3) {
        ppt->c[0] -= 1;
      }
      while (ppt->c[0] < 0 - 5e-3) {
        ppt->c[0] += 1;
      }
      while (ppt->c[1] > 1 + 5e-3) {
        ppt->c[1] -= 1;
      }
      while (ppt->c[1] < 0 - 5e-3) {
        ppt->c[1] += 1;
      }
    }
  }

  if ( nref ) {
    fprintf(stdout,"\n     $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ \n");
    fprintf(stdout,"         WARNING : %d entities with unexpected refs (ref< 0).\n",nref);
    fprintf(stdout,"                   We take their absolute values.\n");
    fprintf(stdout,"     $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ \n\n");
  }

  if ( abs(mesh->info.imprim) > 3 ) {
    fprintf(stdout,"     NUMBER OF VERTICES       %8d  CORNERS    %6d\n",mesh->np,ncor);
    fprintf(stdout,"     NUMBER OF TRIANGLES      %8d\n",mesh->nt);
    if ( mesh->nquad )
      fprintf(stdout,"     NUMBER OF QUADRILATERALS %8d\n",mesh->nquad);

    if ( mesh->na )
      fprintf(stdout,"     NUMBER OF EDGES          %8d\n",mesh->na);

    if ( nreq || nreqed || ntreq || nqreq ) {
      fprintf(stdout,"     NUMBER OF REQUIRED ENTITIES: \n");
      if ( nreq )
        fprintf(stdout,"                  VERTICES       %8d \n",nreq);
      if ( nreqed )
        fprintf(stdout,"                  EDGES          %8d \n",nreqed);
      if ( ntreq )
        fprintf(stdout,"                  TRIANGLES      %8d \n",ntreq);
      if ( nqreq )
        fprintf(stdout,"                  QUADRILATERALS %8d \n",nqreq);
    }
    if(ncor) fprintf(stdout,"     NUMBER OF CORNERS        %8d \n",ncor);
  }

  return 1;
}

/**
 * \param mesh pointer toward the mesh structure.
 * \return 0 if failed, 1 otherwise.
 *
 * Check mesh data for a Msh mesh : mark the vertices as used if no triangles in
 * the mesh (mesh generation) and check that all z-componants are 0.
 *
 */
int MMG2D_2dMeshCheck(MMG5_pMesh mesh) {
  MMG5_pPoint ppt;
  double      z;
  int         k;

  if (!mesh->nt) {
    for (k=1; k<=mesh->np; k++) {
      ppt = &mesh->point[ k ];
      ppt->tag &= ~MG_NUL;
    }
  }

  z = 0.;
  for ( k=1; k<=mesh->np; ++k ) {
    ppt = &mesh->point[k];
    if ( !MG_VOK(ppt) ) continue;

    z += fabs(ppt->c[2]);
  }
  if ( z > MMG5_EPSOK ) {
    fprintf(stderr,"\n  ## Error: %s: Input mesh must be a two-dimensional mesh.\n",
            __func__);
    return 0;
  }
  return 1;
}

int MMG2D_loadMshMesh(MMG5_pMesh mesh,MMG5_pSol sol,const char *filename) {
  FILE*       inm;
  long        posNodes,posElts,*posNodeData;
  int         ier;
  int         bin,iswp,nelts,nsols;

  mesh->dim = 2;

  ier = MMG5_loadMshMesh_part1(mesh,filename,&inm,
                               &posNodes,&posElts,&posNodeData,
                               &bin,&iswp,&nelts,&nsols);
  if ( ier < 1 )  return (ier);

  if ( nsols>1 ) {
    fprintf(stderr,"SEVERAL SOLUTION => IGNORED: %d\n",nsols);
    nsols = 0;
  }

  if ( !MMG2D_zaldy(mesh) ) {
    fclose(inm);
    MMG5_SAFE_FREE(posNodeData);
    return 0;
  }

  if ( mesh->ne || mesh->nprism ) {
    fprintf(stderr,"\n  ## Error: %s: Input mesh must be a two-dimensional mesh.\n",
            __func__);
    fclose(inm);
    MMG5_SAFE_FREE(posNodeData);
    return -1;
  }
  if ( !mesh->nt )
      fprintf(stdout,"  ** WARNING NO GIVEN TRIANGLE\n");

  if (mesh->npmax < mesh->np || mesh->ntmax < mesh->nt ) {
    fclose(inm);
    MMG5_SAFE_FREE(posNodeData);
    return -1;
  }

  ier = MMG5_loadMshMesh_part2( mesh,&sol,&inm,
                                posNodes,posElts,posNodeData,
                                bin,iswp,nelts,nsols);

  MMG5_SAFE_FREE(posNodeData);
  if ( ier < 1 ) return  ier;

  /* Check the metric type */
  ier = MMG5_chkMetricType(mesh,&sol->type,inm);
  if ( ier <1 ) {
    fprintf(stderr,"  ** ERROR WHEN PARSING THE INPUT FILE\n");
    return ier;
  }

  /* Mark all points as used in case of mesh generation and check the
   * z-componant */
  if ( !MMG2D_2dMeshCheck(mesh) ) return -1;

  return 1;
}

int MMG2D_loadMshMesh_and_allData(MMG5_pMesh mesh,MMG5_pSol *sol,const char *filename) {
  FILE*       inm;
  long        posNodes,posElts,*posNodeData;
  int         ier;
  int         bin,iswp,nelts,nsols;

  mesh->dim = 2;

  ier = MMG5_loadMshMesh_part1(mesh,filename,&inm,
                               &posNodes,&posElts,&posNodeData,
                               &bin,&iswp,&nelts,&nsols);
  if ( ier < 1 )  return (ier);

  mesh->nsols = nsols;
  if ( *sol )  MMG5_DEL_MEM(mesh,*sol);

  MMG5_ADD_MEM(mesh,nsols*sizeof(MMG5_Sol),"solutions array",
                printf("  Exit program.\n"); fclose(inm);
                MMG5_SAFE_FREE(posNodeData);
                return -1);
  MMG5_SAFE_CALLOC(*sol,nsols,MMG5_Sol,return -1);

  if ( !MMG2D_zaldy(mesh) ) {
    fclose(inm);
    MMG5_SAFE_FREE(posNodeData);
    return 0;
  }

  if ( mesh->ne || mesh->nprism ) {
    fprintf(stderr,"\n  ## Error: %s: Input mesh must be a two-dimensional mesh.\n",
            __func__);
    fclose(inm);
    MMG5_SAFE_FREE(posNodeData);
    return -1;
  }
  if ( !mesh->nt )
      fprintf(stdout,"  ** WARNING NO GIVEN TRIANGLE\n");

  if (mesh->npmax < mesh->np || mesh->ntmax < mesh->nt ) {
    fclose(inm);
    MMG5_SAFE_FREE(posNodeData);
    return -1;
  }

  ier = MMG5_loadMshMesh_part2( mesh,sol,&inm,
                                posNodes,posElts,posNodeData,
                                bin,iswp,nelts,nsols);

  MMG5_SAFE_FREE(posNodeData);
  if ( ier < 1 ) {
    fprintf(stderr,"  ** ERROR WHEN PARSING THE INPUT FILE\n");
    return  ier;
  }

  /* Mark all points as used in case of mesh generation and check the
   * z-componant */
  if ( !MMG2D_2dMeshCheck(mesh) ) return -1;

  return 1;
}

/**
 * \param sol pointer toward an allocatable sol structure.
 * \param inm pointer toward the solution file
 * \param bin 1 if binary file
 * \param iswp Endianess
 * \param index of the readed solution
 *
 * \return 1 if success, -1 if fail
 *
 * Read the solution value for vertex of index pos in floating precision.
 *
 */
static inline
int MMG2D_readFloatSol(MMG5_pSol sol,FILE *inm,int bin,int iswp,int pos) {
  float       fbuf;
  int         i;

  for (i=0; i<sol->size; i++) {
    if ( !bin ) {
      MMG_FSCANF(inm,"%f",&fbuf);
      sol->m[sol->size*pos+i] = (double)fbuf;
    }
    else {
      MMG_FREAD(&fbuf,MMG5_SW,1,inm);
      if ( iswp ) fbuf=MMG5_swapf(fbuf);
      sol->m[sol->size*pos+i] = (double)fbuf;
    }
  }
  return 1;
}

/**
 * \param sol pointer toward an allocatable sol structure.
 * \param inm pointer toward the solution file
 * \param bin 1 if binary file
 * \param iswp Endianess
 * \param index of the readed solution
 *
 * \return 1 if success, -1 if fail
 *
 * Read the solution value for vertex of index pos in double precision.
 *
 */
static inline
int MMG2D_readDoubleSol(MMG5_pSol sol,FILE *inm,int bin,int iswp,int pos) {
  double       dbuf;
  int          i;

  for (i=0; i<sol->size; i++) {
    if ( !bin ) {
      MMG_FSCANF(inm,"%lf",&dbuf);
      sol->m[sol->size*pos+i] = (double)dbuf;
    }
    else {
      MMG_FREAD(&dbuf,MMG5_SD,1,inm);
      if ( iswp ) dbuf=MMG5_swapf(dbuf);
      sol->m[sol->size*pos+i] = (double)dbuf;
    }
  }
  return 1;
}

/**
 * \param mesh pointer toward the mesh structure.
 * \param met pointer toward the sol structure.
 * \param filename name of file.
 * \return -1 data invalid or we fail, 0 no file, 1 ok.
 *
 * Load metric field.
 *
 */
int MMG2D_loadSol(MMG5_pMesh mesh,MMG5_pSol sol,const char *filename) {
  FILE       *inm;
  long        posnp;
  int         iswp,ier,dim,meshDim;
  int         k,ver,bin,np,nsols,*type;

  /** Read the file header */
  meshDim = 2;
  if ( mesh->info.nreg == 2 ) {
    /* -msh mode */
    meshDim = 3;
  }
  ier =  MMG5_loadSolHeader(filename,meshDim,&inm,&ver,&bin,&iswp,&np,&dim,&nsols,
                             &type,&posnp,mesh->info.imprim);

  /* correction for the -msh mode */
  sol->dim = 2;

  if ( ier < 1 ) return ier;

  if ( nsols!=1 ) {
    fprintf(stderr,"SEVERAL SOLUTION => IGNORED: %d\n",nsols);
    fclose(inm);
    MMG5_SAFE_FREE(type);
    return -1;
  }

  if ( mesh->np != np ) {
    fprintf(stderr,"  ** MISMATCHES DATA: THE NUMBER OF VERTICES IN "
            "THE MESH (%d) DIFFERS FROM THE NUMBER OF VERTICES IN "
            "THE SOLUTION (%d) \n",mesh->np,np);
    fclose(inm);
    MMG5_SAFE_FREE(type);
    return -1;
  }

  ier = MMG5_chkMetricType(mesh,type,inm);
  if ( ier <1 ) return ier;

  /* Allocate and store the header informations for each solution */
  if ( !MMG2D_Set_solSize(mesh,sol,MMG5_Vertex,mesh->np,type[0]) ) {
    fclose(inm);
    MMG5_SAFE_FREE(type);
    return -1;
  }
  /* For binary file, we read the verson inside the file */
  if ( ver ) sol->ver = ver;

  MMG5_SAFE_FREE(type);

  /* Read mesh solutions */
  rewind(inm);
  fseek(inm,posnp,SEEK_SET);

  if ( sol->ver == 1 ) {
    /* Simple precision */
    for (k=1; k<=sol->np; k++) {
      if ( MMG2D_readFloatSol(sol,inm,bin,iswp,k) < 0 ) return -1;
    }
  }
  else {
    for (k=1; k<=sol->np; k++) {
      /* Double precision */
      if ( MMG2D_readDoubleSol(sol,inm,bin,iswp,k) < 0 ) return -1;
    }
  }

  fclose(inm);

  /* stats */
  MMG5_printMetStats(mesh,sol);

  return 1;
}

/**
 * \param mesh pointer toward the mesh structure.
 * \param sol pointer toward an allocatable sol structure.
 * \param filename name of file.
 * \return -1 data invalid or we fail, 0 no file, 1 ok.
 *
 * Load a medit solution file containing 1 or more solutions.
 *
 */
int MMG2D_loadAllSols(MMG5_pMesh mesh,MMG5_pSol *sol, const char *filename) {
  MMG5_pSol   psl;
  FILE       *inm;
  long        posnp;
  int         iswp,ier,dim,meshDim;
  int         j,k,ver,bin,np,nsols,*type;
  char        data[16];
  static char mmgWarn = 0;

  /** Read the file header */
  meshDim = 2;
  if ( mesh->info.nreg == 2 ) {
    /* -msh mode */
    meshDim = 3;
  }

  ier =  MMG5_loadSolHeader(filename,meshDim,&inm,&ver,&bin,&iswp,&np,&dim,&nsols,
                            &type,&posnp,mesh->info.imprim);
  if ( ier < 1 ) return ier;

  if ( mesh->np != np ) {
    fprintf(stderr,"  ** MISMATCHES DATA: THE NUMBER OF VERTICES IN "
            "THE MESH (%d) DIFFERS FROM THE NUMBER OF VERTICES IN "
            "THE SOLUTION (%d) \n",mesh->np,np);
    fclose(inm);
    MMG5_SAFE_FREE(type);
    return -1;
  }

  /** Sol tab allocation */
  mesh->nsols = nsols;

  if ( nsols > MMG5_NSOLS_MAX ) {
    fprintf(stderr,"\n  ## Error: %s: unexpected number of data (%d).\n",
            __func__,nsols);
    MMG5_SAFE_FREE(type);
    fclose(inm);
    return -1;
  }

  if ( *sol )  MMG5_DEL_MEM(mesh,*sol);

  MMG5_ADD_MEM(mesh,nsols*sizeof(MMG5_Sol),"solutions array",
                printf("  Exit program.\n"); fclose(inm);
                MMG5_SAFE_FREE(type);
                return -1);
  MMG5_SAFE_CALLOC(*sol,nsols,MMG5_Sol,return -1);

  for ( j=0; j<nsols; ++j) {
    psl = *sol+j;

    /* Give an arbitrary name to the solution because the Medit format has non
     * name field */
    sprintf(data,"sol_%d",j);
    if ( !MMG2D_Set_inputSolName(mesh,psl,data) ) {
      if ( !mmgWarn ) {
        mmgWarn = 1;
        fprintf(stderr,"\n  ## Warning: %s: unable to set solution name for"
                " at least 1 solution.\n",__func__);
      }
    }

    /* Allocate and store the header informations for each solution */
    if ( !MMG2D_Set_solSize(mesh,psl,MMG5_Vertex,mesh->np,type[j]) ) {
      MMG5_SAFE_FREE(type);
      fclose(inm);
      return -1;
    }
    psl->dim = 2;
    /* For binary file, we read the verson inside the file */
    if ( ver ) psl->ver = ver;
  }
  MMG5_SAFE_FREE(type);

  /* read mesh solutions */
  rewind(inm);
  fseek(inm,posnp,SEEK_SET);

  if ( (*sol)[0].ver == 1 ) {
    /* Simple precision */
    for (k=1; k<=mesh->np; k++) {
      for ( j=0; j<nsols; ++j ) {
        psl = *sol+j;
        if ( MMG2D_readFloatSol(psl,inm,bin,iswp,k) < 0 ) return -1;
      }
    }
  }
  else {
    /* Double precision */
    for (k=1; k<=mesh->np; k++) {
      for ( j=0; j<nsols; ++j ) {
        psl = *sol+j;
        if ( MMG2D_readDoubleSol(psl,inm,bin,iswp,k) < 0 ) return -1;
      }
    }
  }
  fclose(inm);

  /* stats */
  MMG5_printSolStats(mesh,sol);

  return 1;
}

int MMG2D_saveMesh(MMG5_pMesh mesh,const char *filename) {
  FILE*             inm;
  MMG5_pPoint       ppt;
  MMG5_pEdge        ped;
  MMG5_pTria        pt;
  MMG5_pQuad        pq;
  double            dblb;
  int               k,ne,np,nc,nreq,nereq,nedreq,nq,nqreq,ref,ntang;
  int               bin, binch, bpos;
  char              *ptr,*data,chaine[MMG5_FILESTR_LGTH];

  mesh->ver = 2;
  bin = 0;

  /* Name of file */
  MMG5_SAFE_CALLOC(data,strlen(filename)+7,char,return 0);
  strcpy(data,filename);
  ptr = strstr(data,".mesh");
  if ( !ptr ) {
    strcat(data,".meshb");
    if( !(inm = fopen(data,"wb")) ) {
      ptr  = strstr(data,".mesh");
      *ptr = '\0';
      strcat(data,".mesh");
      if( !(inm = fopen(data,"wb")) ) {
        MMG5_SAFE_FREE(data);
        return 0;
      }
    }
    else {
      bin = 1;
    }
  }
  else {
    ptr = strstr(data,".meshb");
    if( ptr )  bin = 1;
    if( !(inm = fopen(data,"wb")) ) {
      fprintf(stderr,"  ** UNABLE TO OPEN %s.\n",data);
      MMG5_SAFE_FREE(data);
      return 0;
    }
  }

  if ( mesh->info.imprim >= 0 )
    fprintf(stdout,"  %%%% %s OPENED\n",data);
  MMG5_SAFE_FREE(data);

  /* Write header */
  binch=0; bpos=10;
  if ( !bin ) {
    strcpy(&chaine[0],"MeshVersionFormatted 2\n");
    fprintf(inm,"%s",chaine);
    if(mesh->info.nreg) {
      strcpy(&chaine[0],"\n\nDimension 3\n");
    }
    else {
      strcpy(&chaine[0],"\n\nDimension 2\n");
    }
    fprintf(inm,"%s ",chaine);
  }
  else
  {
    binch = 1; //MeshVersionFormatted
    fwrite(&binch,MMG5_SW,1,inm);
    binch = 2; //version
    fwrite(&binch,MMG5_SW,1,inm);
    binch = 3; //Dimension
    fwrite(&binch,MMG5_SW,1,inm);
    bpos = 20; //Pos
    fwrite(&bpos,MMG5_SW,1,inm);
    if(mesh->info.nreg) binch = 3; //Dimension
    else binch = 2;
    fwrite(&binch,MMG5_SW,1,inm);
  }

  /* Write vertices */
  np = 0;
  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( MG_VOK(ppt) )  np++;
    ppt->tmp = np;
  }

  if ( !bin ) {
    strcpy(&chaine[0],"\n\nVertices\n");
    fprintf(inm,"%s",chaine);
    fprintf(inm,"%d\n",np);
  }
  else {
    binch = 4; //Vertices
    fwrite(&binch,MMG5_SW,1,inm);
    if ( mesh->info.nreg )
      bpos += 12+(1+3*mesh->ver)*4*np; //NullPos
    else
      bpos += 12+(1+2*mesh->ver)*4*np; //NullPos

    fwrite(&bpos,MMG5_SW,1,inm);
    fwrite(&np,MMG5_SW,1,inm);
  }
  fflush(inm);

  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( MG_VOK(ppt) ) {
      ref = ppt->ref;
      if ( mesh->info.nreg ) {
        if ( !bin )
          fprintf(inm,"%.15lg %.15lg 0 %d\n",ppt->c[0],ppt->c[1],ref);
        else {
          dblb = 0.;
          fwrite((unsigned char*)&ppt->c[0],MMG5_SD,1,inm);
          fwrite((unsigned char*)&ppt->c[1],MMG5_SD,1,inm);
          fwrite((unsigned char*)&dblb,MMG5_SD,1,inm);
          fwrite((unsigned char*)&ref,MMG5_SW,1,inm);
        }
      }
      else {
        if ( !bin ) {
          fprintf(inm,"%.15lg %.15lg %d\n",ppt->c[0],ppt->c[1],ref);
          fflush(inm);
        }
        else {
          fwrite(&ppt->c[0],MMG5_SD,1,inm);
          fwrite(&ppt->c[1],MMG5_SD,1,inm);
          fwrite(&ref,MMG5_SW,1,inm);
        }
      }
    }
  }

  /* Print corners */
  nc = 0;
  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( MG_VOK(ppt) && (ppt->tag & MG_CRN) )  nc++;
  }

  if ( nc ) {
    if ( !bin ) {
      strcpy(&chaine[0],"\n\nCorners\n");
      fprintf(inm,"%s",chaine);
      fprintf(inm,"%d\n",nc);
    }
    else
    {
      binch = 13; //
      fwrite(&binch,MMG5_SW,1,inm);
      bpos += 12+4*nc; //NullPos
      fwrite(&bpos,MMG5_SW,1,inm);
      fwrite(&nc,MMG5_SW,1,inm);
    }

    for (k=1; k<=mesh->np; k++) {
      ppt = &mesh->point[k];
      if ( MG_VOK(ppt) && (ppt->tag & MG_CRN) ) {
        if(!bin) {
          fprintf(inm,"%d\n",ppt->tmp);
        }
        else {
          fwrite(&ppt->tmp,MMG5_SW,1,inm);
        }
      }
    }
  }

  /* Required vertex */
  nreq = 0;
  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( MG_VOK(ppt) ) {
      if ( mesh->info.nosurf && (ppt->tag & MG_NOSURF) ) continue;
      if ( ppt->tag & MG_REQ )  nreq++;
    }
  }
  if ( nreq ) {
    if ( !bin ) {
      strcpy(&chaine[0],"\n\nRequiredVertices\n");
      fprintf(inm,"%s",chaine);
      fprintf(inm,"%d\n",nreq);
    }
    else {
      binch = 15; //
      fwrite(&binch,MMG5_SW,1,inm);
      bpos += 12+4*nreq; //NullPos
      fwrite(&bpos,MMG5_SW,1,inm);
      fwrite(&nreq,MMG5_SW,1,inm);
    }
    for (k=1; k<=mesh->np; k++) {
      ppt = &mesh->point[k];
      if ( MG_VOK(ppt) ) {
        if ( mesh->info.nosurf && ( ppt->tag & MG_NOSURF )) continue;
        if ((ppt->tag & MG_REQ)
            /*&& ( (ppt->tag & MG_BDY) || (ppt->tag & MG_SD) ) */ ) {
          if(!bin)
            fprintf(inm,"%d\n",ppt->tmp);
          else
            fwrite(&ppt->tmp,MMG5_SW,1,inm);
        }
      }
    }
  }

  /* edges */
  nedreq = 0;
  if ( mesh->na ) {
    if(!bin) {
      strcpy(&chaine[0],"\n\nEdges\n");
      fprintf(inm,"%s",chaine);
      fprintf(inm,"%d\n",mesh->na);
    }
    else {
      binch = 5; //Edges
      fwrite(&binch,MMG5_SW,1,inm);
      bpos += 12 + 3*4*mesh->na;//Pos
      fwrite(&bpos,MMG5_SW,1,inm);
      fwrite(&mesh->na,MMG5_SW,1,inm);
    }
    for (k=1; k<=mesh->na; k++) {
      ped = &mesh->edge[k];
      if(!bin)
        fprintf(inm,"%d %d %d\n",mesh->point[ped->a].tmp,mesh->point[ped->b].tmp,ped->ref);
      else
      {
        fwrite(&mesh->point[ped->a].tmp,MMG5_SW,1,inm);
        fwrite(&mesh->point[ped->b].tmp,MMG5_SW,1,inm);
        fwrite(&ped->ref,MMG5_SW,1,inm);
      }
      if ( ped->tag & MG_REQ ) nedreq++;
    }

    if ( nedreq ) {
      if(!bin) {
        strcpy(&chaine[0],"\n\nRequiredEdges\n");
        fprintf(inm,"%s",chaine);
        fprintf(inm,"%d\n",nedreq);
      } else {
        binch = 16; //RequiredEdges
        fwrite(&binch,MMG5_SW,1,inm);
        bpos += 12 + 4*nedreq;//Pos
        fwrite(&bpos,MMG5_SW,1,inm);
        fwrite(&nedreq,MMG5_SW,1,inm);
      }
      ne = 0;
      for (k=1; k<=mesh->na; k++) {
        ne++;
        if (  mesh->edge[k].tag & MG_REQ ) {
          if(!bin) {
            fprintf(inm,"%d\n",ne);
          } else {
            fwrite(&ne,MMG5_SW,1,inm);
          }
        }
      }
    }
  }

  /* elements */
  ne    = 0;
  nereq = 0;
  for (k=1; k<=mesh->nt; k++) {
    pt = &mesh->tria[k];
    if ( !MG_EOK(pt) ) continue;
    ne++;
    if ( (pt->tag[0] & MG_REQ) && (pt->tag[1] & MG_REQ) && pt->tag[2] & MG_REQ )
      ++nereq;
  }

  if ( ne ) {
    if ( !bin ) {
      strcpy(&chaine[0],"\n\nTriangles\n");
      fprintf(inm,"%s",chaine);
      fprintf(inm,"%d\n",ne);
    }
    else {
      binch = 6; //Triangles
      fwrite(&binch,MMG5_SW,1,inm);
      bpos += 12+16*ne; //Pos
      fwrite(&bpos,MMG5_SW,1,inm);
      fwrite(&ne,MMG5_SW,1,inm);
    }
    for (k=1; k<=mesh->nt; k++) {
      pt = &mesh->tria[k];
      if ( MG_EOK(pt) ) {
        ref = pt->ref;
        if ( !bin ) {
          fprintf(inm,"%d %d %d %d\n",mesh->point[pt->v[0]].tmp,
                  mesh->point[pt->v[1]].tmp,
                  mesh->point[pt->v[2]].tmp,ref);
        }
        else {
          fwrite(&mesh->point[pt->v[0]].tmp,MMG5_SW,1,inm);
          fwrite(&mesh->point[pt->v[1]].tmp,MMG5_SW,1,inm);
          fwrite(&mesh->point[pt->v[2]].tmp,MMG5_SW,1,inm);
          fwrite(&ref,MMG5_SW,1,inm);
        }
      }
    }
    if ( nereq ) {
      if(!bin) {
        strcpy(&chaine[0],"\n\nRequiredTriangles\n");
        fprintf(inm,"%s",chaine);
        fprintf(inm,"%d\n",nereq);
      } else {
        binch = 17; //ReqTriangles
        fwrite(&binch,MMG5_SW,1,inm);
        bpos += 12+4*nereq; //Pos
        fwrite(&bpos,MMG5_SW,1,inm);
        fwrite(&nereq,MMG5_SW,1,inm);
      }
      ne = 0;
      for (k=1; k<=mesh->nt; k++) {
        pt = &mesh->tria[k];
        if ( !MG_EOK(pt) )  continue;
        ++ne;
        if ( (pt->tag[0] & MG_REQ) && (pt->tag[1] & MG_REQ)
             && pt->tag[2] & MG_REQ ) {
          if(!bin) {
            fprintf(inm,"%d\n",ne);
          } else {
            fwrite(&ne,MMG5_SW,1,inm);
          }
        }
      }
    }
  }

  /* quad + required quad */
  nq = nqreq = 0;

  if ( mesh->nquad ) {

    for (k=1; k<=mesh->nquad; k++) {
      pq = &mesh->quadra[k];
      if ( !MG_EOK(pq) ) {
        continue;
      }
      nq++;
      if ( pq->tag[0] & MG_REQ && pq->tag[1] & MG_REQ &&
           pq->tag[2] & MG_REQ && pq->tag[3] & MG_REQ ) {
        nqreq++;
      }
    }
  }

  if ( nq ) {
    if(!bin) {
      strcpy(&chaine[0],"\n\nQuadrilaterals\n");
      fprintf(inm,"%s",chaine);
      fprintf(inm,"%d\n",nq);
    } else {
      binch = 7; //Quadrilaterals
      fwrite(&binch,MMG5_SW,1,inm);
      bpos += 12+20*nq; //Pos
      fwrite(&bpos,MMG5_SW,1,inm);
      fwrite(&nq,MMG5_SW,1,inm);
    }
    for (k=1; k<=mesh->nquad; k++) {
      pq = &mesh->quadra[k];
      if ( !MG_EOK(pq) ) continue;

      if(!bin) {
        fprintf(inm,"%d %d %d %d %d\n",mesh->point[pq->v[0]].tmp,
                mesh->point[pq->v[1]].tmp,mesh->point[pq->v[2]].tmp,
                mesh->point[pq->v[3]].tmp, pq->ref);
      } else {
        fwrite(&mesh->point[pq->v[0]].tmp,MMG5_SW,1,inm);
        fwrite(&mesh->point[pq->v[1]].tmp,MMG5_SW,1,inm);
        fwrite(&mesh->point[pq->v[2]].tmp,MMG5_SW,1,inm);
        fwrite(&mesh->point[pq->v[3]].tmp,MMG5_SW,1,inm);
        fwrite(&pq->ref,MMG5_SW,1,inm);
      }
      if ( pq->tag[0] & MG_REQ && pq->tag[1] & MG_REQ &&
           pq->tag[2] & MG_REQ && pq->tag[3] & MG_REQ ) {
        nqreq++;
      }
    }
    if ( nqreq ) {
      if(!bin) {
        strcpy(&chaine[0],"\n\nRequiredQuadrilaterals\n");
        fprintf(inm,"%s",chaine);
        fprintf(inm,"%d\n",nqreq);
      } else {
        binch = 18; //ReqQuad
        fwrite(&binch,MMG5_SW,1,inm);
        bpos += 12+4*nqreq; //Pos
        fwrite(&bpos,MMG5_SW,1,inm);
        fwrite(&nqreq,MMG5_SW,1,inm);
      }
      for (k=0; k<=mesh->nquad; k++) {
        pq = &mesh->quadra[k];
        if ( (pq->tag[0] & MG_REQ) && (pq->tag[1] & MG_REQ)
             && pq->tag[2] & MG_REQ && pq->tag[3] & MG_REQ ) {
          if(!bin) {
            fprintf(inm,"%d\n",k);
          } else {
            fwrite(&k,MMG5_SW,1,inm);
          }
        }
      }
    }
  }

  /*savetangent*/
  ntang=0;
  for(k=1 ; k<=mesh->np ; k++) {
    ppt = &mesh->point[k];
    if ( MG_VOK(ppt) ) {
      if(!(ppt->tag & MG_BDY)) continue;
      if(ppt->tag & MG_CRN) continue;
      ntang++;
    }
  }


  /* Remark: here we save the tangents but there is a bug in medit (it crashes
   * if it try to read tangents without normals. It is easy to patch, in
   * zaldy1.c, seek the " if ( mesh->ntg )" field and replace
   * "assert(mesh->extra->n);" by "assert(mesh->extra->t);").
   * To not have to modify medit, here we save the tangents as if it were normals. */
  /* if ( ntang ) { */
  /*   if ( !bin ) { */
  /*     strcpy(&chaine[0],"\n\nNormals\n"); //be careful it is tangent!! */
  /*     fprintf(inm,"%s",chaine); */
  /*     fprintf(inm,"%d\n",ntang); */
  /*   } */
  /*   else */
  /*   { */
  /*     binch = 60; //normals */
  /*     fwrite(&binch,MMG5_SW,1,inm); */
  /*     if ( mesh->info.nreg ) */
  /*       bpos += 12+(3*mesh->ver)*4*ntang; //Pos */
  /*     else */
  /*       bpos += 12+(2*mesh->ver)*4*ntang; //Pos */
  /*     fwrite(&bpos,MMG5_SW,1,inm); */
  /*     fwrite(&ntang,MMG5_SW,1,inm); */
  /*   } */

  /*   for(k=1 ; k<=mesh->np ; k++) { */
  /*     ppt = &mesh->point[k]; */
  /*     if(!MG_VOK(ppt)) continue; */
  /*     if(!(ppt->tag & MG_BDY)) continue; */
  /*     if(ppt->tag & MG_CRN) continue; */
  /*     if(mesh->info.nreg) { */
  /*       if ( !bin ) */
  /*         fprintf(inm,"%lf %lf %lf\n",ppt->n[0],ppt->n[1],0.e0); */
  /*       else { */
  /*         dblb = 0; */
  /*         fwrite((unsigned char*)&ppt->n[0],MMG5_SD,1,inm); */
  /*         fwrite((unsigned char*)&ppt->n[1],MMG5_SD,1,inm); */
  /*         fwrite(&dblb,MMG5_SD,1,inm); */
  /*       } */
  /*     } */
  /*     else */
  /*     { */
  /*       if ( !bin ) */
  /*         fprintf(inm,"%lf %lf \n",ppt->n[0],ppt->n[1]); */
  /*       else { */
  /*         fwrite((unsigned char*)&ppt->n[0],MMG5_SD,1,inm); */
  /*         fwrite((unsigned char*)&ppt->n[1],MMG5_SD,1,inm); */
  /*       } */
  /*     } */
  /*   } */

  /*   if ( !bin ) { */
  /*     strcpy(&chaine[0],"\n\nNormalAtVertices\n"); */
  /*     fprintf(inm,"%s",chaine); */
  /*     fprintf(inm,"%d\n",ntang); */
  /*   } */

  /*   else { */
  /*     binch = 20; //normalatvertices */
  /*     fwrite(&binch,MMG5_SW,1,inm); */
  /*     bpos += 12 + 2*4*ntang;//Pos */
  /*     fwrite(&bpos,MMG5_SW,1,inm); */
  /*     fwrite(&ntang,MMG5_SW,1,inm); */
  /*   } */
  /*   nn=1; */
  /*   for(k=1 ; k<=mesh->np ; k++) { */
  /*     ppt = &mesh->point[k]; */
  /*     if ( !MG_VOK(ppt) ) continue; */
  /*     if(!(ppt->tag & MG_BDY)) continue; */
  /*     if(ppt->tag & MG_CRN) continue; */

  /*     if(!bin) { */
  /*       fprintf(inm,"%d %d\n",ppt->tmp,nn++); */
  /*     } */
  /*     else { */
  /*       fwrite(&ppt->tmp,MMG5_SW,1,inm); */
  /*       ++nn; */
  /*       fwrite(&nn,MMG5_SW,1,inm); */
  /*     } */
  /*   } */
  /* } */

  if(!bin) {
    strcpy(&chaine[0],"\n\nEnd\n");
    fprintf(inm,"%s",chaine);
  }
  else {
    binch = 54; //End
    fwrite(&binch,MMG5_SW,1,inm);
  }

  if ( abs(mesh->info.imprim) > 4 ) {
    fprintf(stdout,"     NUMBER OF VERTICES       %8d   CORNERS   %8d"
            "   REQUIRED %8d\n",np,nc,nreq);

    if ( mesh->na )
      fprintf(stdout,"     NUMBER OF EDGES          %8d   REQUIRED  %8d\n",mesh->na,nedreq);
    if ( mesh->nt )
      fprintf(stdout,"     NUMBER OF TRIANGLES      %8d   REQUIRED  %8d\n",
              mesh->nt, nereq);
    if ( nq )
      fprintf(stdout,"     NUMBER OF QUADRILATERALS %8d   REQUIRED  %8d\n",nq,nqreq);
  }

  fclose(inm);

  return 1;
}

int MMG2D_saveMshMesh(MMG5_pMesh mesh,MMG5_pSol sol,const char *filename) {
  return MMG5_saveMshMesh(mesh,&sol,filename,1);
}

int MMG2D_saveMshMesh_and_allData(MMG5_pMesh mesh,MMG5_pSol *sol,const char *filename) {
  return MMG5_saveMshMesh(mesh,sol,filename,0);
}

/**
 * \param sol pointer toward an allocatable sol structure.
 * \param inm pointer toward the solution file
 * \param bin 1 if binary file
 * \param index of the writted solution
 *
 * Write the solution value for vertex of index pos in double precision.
 *
 */
static inline
void MMG2D_writeDoubleSol(MMG5_pSol sol,FILE *inm,int bin,int pos) {
  int          i,isol;

  isol = pos * sol->size;

  if ( !bin ) {
    for (i=0; i<sol->size; i++)
      fprintf(inm," %.15lg",sol->m[isol + i]);
  }
  else {
    for (i=0; i<sol->size; i++)
      fwrite(&sol->m[isol + i],MMG5_SD,1,inm);
  }
}


/**
 * \param mesh pointer toward the mesh structure.
 * \param met pointer toward the sol structure.
 * \param filename name of file.
 * \return 0 if failed, 1 otherwise.
 *
 * Write isotropic or anisotropic metric.
 *
 */
int MMG2D_saveSol(MMG5_pMesh mesh,MMG5_pSol sol,const char *filename) {
  FILE*        inm;
  MMG5_pPoint  ppt;
  int          k,ier;
  int          binch,bin,dim;

  if ( !sol->np )  return 1;


  if ( !(sol->np || sol->m) ) {
    fprintf(stderr,"\n  ## Warning: %s: no metric data to save.\n",__func__);
    return 1;
  }


  sol->ver = 2;

  if ( sol->dim==2 && mesh->info.nreg ) {
    dim = 3;
  }
  else {
    dim = 2;
  }

  ier = MMG5_saveSolHeader( mesh,filename,&inm,sol->ver,&bin,mesh->np,dim,
                            1,&sol->type,&sol->size);

  if ( ier < 1 ) return ier;

  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( !MG_VOK(ppt) )  continue;

    MMG2D_writeDoubleSol(sol,inm,bin,k);
    fprintf(inm,"\n");
  }

  /* End file */
  if ( !bin ) {
    fprintf(inm,"\n\nEnd\n");
  }
  else {
    binch = 54; //End
    fwrite(&binch,MMG5_SW,1,inm);
  }
  fclose(inm);

  return 1;
}

/**
 * \param mesh pointer toward the mesh structure.
 * \param sol pointer toward the solutions array.
 * \param filename name of file.
 * \return 0 if failed, 1 otherwise.
 *
 * Write 1 or more solutions.
 *
 */
int MMG2D_saveAllSols(MMG5_pMesh mesh,MMG5_pSol *sol,const char *filename) {
  FILE*        inm;
  MMG5_pPoint  ppt;
  MMG5_pSol    psl;
  int          j,k,ier;
  int          binch,bin;
  int          *type,*size;


  if ( !(*sol)[0].np )  return 1;

  MMG5_SAFE_CALLOC(type,mesh->nsols,int,return 0);
  MMG5_SAFE_CALLOC(size,mesh->nsols,int,MMG5_SAFE_FREE(type);return 0);

  for (k=0; k<mesh->nsols; ++k ) {
    (*sol)[k].ver = 2;
    type[k]     = (*sol)[k].type;
    size[k]     = (*sol)[k].size;
  }

  ier = MMG5_saveSolHeader( mesh,filename,&inm,(*sol)[0].ver,&bin,mesh->np,
                            (*sol)[0].dim,mesh->nsols,type,size);

  MMG5_SAFE_FREE(type);
  MMG5_SAFE_FREE(size);

  if ( ier < 1 ) return ier;

  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( !MG_VOK(ppt) )  continue;

    for ( j=0; j<mesh->nsols; ++j ) {
      psl = *sol + j;
      MMG2D_writeDoubleSol(psl,inm,bin,k);
    }
    fprintf(inm,"\n");
  }

  /* End file */
  if ( !bin ) {
    fprintf(inm,"\n\nEnd\n");
  }
  else {
    binch = 54; //End
    fwrite(&binch,MMG5_SW,1,inm);
  }
  fclose(inm);

  return 1;
}

/* Custom version of Savemesh for debugging purpose */
int MMG2D_savemesh_db(MMG5_pMesh mesh,char *filename,char pack) {
  MMG5_pTria         pt;
  MMG5_pEdge         pa;
  MMG5_pPoint        ppt,p0,p1,p2;
  int                k,np,nt,nc;
  FILE               *out;

  out = fopen(filename,"w");

  np = nt = 0;
  /* Write Header */
  fprintf(out,"MeshVersionFormatted %d\n\nDimension %d\n\n",1,2);

  /* Print vertices */
  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( pack && MG_VOK(ppt) ) {
      np++;
      ppt->tmp = np;
    }
    else if ( !pack ) {
      np++;
    }
  }

  fprintf(out,"Vertices\n %d\n\n",np);
  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( ( pack && MG_VOK(ppt) ) || !pack )
      fprintf(out,"%f %f %d\n",ppt->c[0],ppt->c[1],ppt->ref);
  }

  /* Print Triangles */
  for (k=1; k<=mesh->nt; k++) {
    pt = &mesh->tria[k];
    if ( MG_EOK(pt) ) nt++;
  }

  fprintf(out,"Triangles\n %d\n\n",nt);
  for (k=1; k<=mesh->nt; k++) {
    pt = &mesh->tria[k];
    if ( MG_EOK(pt) ) {
      p0 = &mesh->point[pt->v[0]];
      p1 = &mesh->point[pt->v[1]];
      p2 = &mesh->point[pt->v[2]];
      if ( pack ) {
        fprintf(out,"%d %d %d %d\n",p0->tmp,p1->tmp,p2->tmp,pt->ref);
      }
      else {
        fprintf(out,"%d %d %d %d\n",pt->v[0],pt->v[1],pt->v[2],pt->ref);
      }
    }
  }

  /* Print Edges */
  if ( mesh->na ) {
    fprintf(out,"Edges\n %d\n\n",mesh->na);
    for (k=1; k<=mesh->na; k++) {
      pa = &mesh->edge[k];
      p1 = &mesh->point[pa->a];
      p2 = &mesh->point[pa->b];
      if ( pack ) fprintf(out,"%d %d %d\n",p1->tmp,p2->tmp,pa->ref);
      else        fprintf(out,"%d %d %d\n",pa->a,pa->b,pa->ref);
    }
  }

  /* Print corners */
  nc = 0;
  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( MG_VOK(ppt) && ppt->tag & MG_CRN ) nc++;
  }

  if ( nc ) {
    fprintf(out,"Corners\n %d\n\n",nc);
    for (k=1; k<=mesh->np; k++) {
      ppt = &mesh->point[k];
      if ( MG_VOK(ppt) && ppt->tag & MG_CRN ) {
        if ( pack )   fprintf(out,"%d\n",ppt->tmp);
        else          fprintf(out,"%d\n",k);
      }
    }
  }

  /* End keyword */
  fprintf(out,"End\n");

  fclose(out);

  return 1;
}

/* Custom version of Savemet for debugging purpose */
int MMG2D_savemet_db(MMG5_pMesh mesh,MMG5_pSol met,char *filename,char pack) {
  MMG5_pPoint        ppt;
  int                k,np;
  char               *ptr,typ=0,*data;
  FILE               *out;

  if ( met->size == 1 ) typ =1;
  else if ( met->size == 3 ) typ = 3;

  MMG5_SAFE_CALLOC(data,strlen(filename)+6,char,return 0);
  strcpy(data,filename);
  ptr = strstr(data,".mesh");
  if ( ptr )
    *ptr = '\0';

  strcat(data,".sol");
  out = fopen(data,"w");

  MMG5_SAFE_FREE(data);

  np = 0;
  for (k=1; k<=mesh->np; k++)
    mesh->point[k].tmp = 0;

  /* Write Header */
  fprintf(out,"MeshVersionFormatted %d\n\nDimension %d\n\n",1,2);

  /* Print vertices */
  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( pack && MG_VOK(ppt) ) {
      np++;
      ppt->tmp = np;
    }
    else if ( !pack ) {
      np++;
      ppt->tmp = np;
    }
  }

  fprintf(out,"SolAtVertices\n %d\n%d %d\n\n",np,1,typ);
  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( ( pack && MG_VOK(ppt) ) || !pack ) {
      if ( met->size == 1 )
        fprintf(out,"%f\n",met->m[k]);
      else if ( met->size == 3 )
        fprintf(out,"%f %f %f\n",met->m[3*k+0],met->m[3*k+1],met->m[3*k+2]);
    }
  }

  /* End keyword */
  fprintf(out,"End\n");

  fclose(out);

  return 1;
}

/* Save normal vector field for debugging purpose */
int MMG2D_savenor_db(MMG5_pMesh mesh,char *filename,char pack) {
  MMG5_pPoint        ppt;
  int                k,np;
  char               *ptr,*data;
  FILE               *out;

  MMG5_SAFE_CALLOC(data,strlen(filename)+6,char,return 0);
  strcpy(data,filename);
  ptr = strstr(data,".mesh");
  if ( ptr )
    *ptr = '\0';

  strcat(data,".nor.sol");
  out = fopen(data,"w");

  MMG5_SAFE_FREE(data);

  np = 0;
  for (k=1; k<=mesh->np; k++)
    mesh->point[k].tmp = 0;

  /* Write Header */
  fprintf(out,"MeshVersionFormatted %d\n\nDimension %d\n\n",1,2);

  /* Pack vertices or not for writing */
  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( pack && MG_VOK(ppt) ) {
      np++;
      ppt->tmp = np;
    }
    else if ( !pack ) {
      np++;
      ppt->tmp = np;
    }
  }

  fprintf(out,"SolAtVertices\n %d\n%d %d\n\n",np,1,2);
  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( ( pack && MG_VOK(ppt) ) || !pack ) {
      if ( MG_EDG(ppt->tag) && ! MG_SIN(ppt->tag) ) fprintf(out,"%f %f\n",ppt->n[0],ppt->n[1]);
      else fprintf(out,"%f %f\n",0.0,0.0);
    }
  }

  /* End keyword */
  fprintf(out,"End\n");

  fclose(out);

  return 1;
}

/* Save displacement field for debugging purpose */
int MMG2D_savedisp_db(MMG5_pMesh mesh,MMG5_pSol disp,char *filename,char pack) {
  MMG5_pPoint        ppt;
  int                k,np;
  char               *ptr,*data;
  FILE               *out;

  MMG5_SAFE_CALLOC(data,strlen(filename)+6,char,return 0);
  strcpy(data,filename);
  ptr = strstr(data,".sol");
  if ( ptr )
    *ptr = '\0';

  strcat(data,".disp.sol");
  out = fopen(data,"w");
  MMG5_SAFE_FREE(data);

  np = 0;
  for (k=1; k<=mesh->np; k++)
    mesh->point[k].tmp = 0;

  /* Write Header */
  fprintf(out,"MeshVersionFormatted %d\n\nDimension %d\n\n",1,2);

  /* Pack vertices or not for writing */
  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( pack && MG_VOK(ppt) ) {
      np++;
      ppt->tmp = np;
    }
    else if ( !pack ) {
      np++;
      ppt->tmp = np;
    }
  }

  fprintf(out,"SolAtVertices\n %d\n%d %d\n\n",np,1,2);
  for (k=1; k<=mesh->np; k++) {
    ppt = &mesh->point[k];
    if ( ( pack && MG_VOK(ppt) ) || !pack )
      fprintf(out,"%f %f\n",disp->m[2*(k-1)+1],disp->m[2*(k-1)+2]);
  }

  /* End keyword */
  fprintf(out,"End\n");

  fclose(out);

  return 1;
}

static inline
int MMG2D_saveEle(MMG5_pMesh mesh,const char *filename) {
  FILE*             inm;
  MMG5_pTria        pt;
  int               k,i,ne;
  char              *ptr,*data;

  if ( !mesh->nt ) {
    return 1;
  }

  if ( (!filename) || !(*filename) ) {
    filename = mesh->nameout;
  }
  if ( (!filename) || !(*filename) ) {
    printf("\n  ## Error: %s: unable to save a file without a valid filename\n.",
           __func__);
    return 0;
  }

  /* Name of file */
  MMG5_SAFE_CALLOC(data,strlen(filename)+5,char,return 0);
  strcpy(data,filename);
  ptr = strstr(data,".node");
  if ( ptr ) {
    *ptr = '\0';
  }

  /* Add .node ext  */
  strcat(data,".ele");
  if( !(inm = fopen(data,"wb")) ) {
    fprintf(stderr,"  ** UNABLE TO OPEN %s.\n",data);
    MMG5_SAFE_FREE(data);
    return 0;
  }

  fprintf(stdout,"  %%%% %s OPENED\n",data);
  MMG5_SAFE_FREE(data);

  ne    = 0;
  for (k=1; k<=mesh->nt; k++) {
    pt = &mesh->tria[k];
    if ( !MG_EOK(pt) ) continue;
    ne++;
  }

  /* Save elt number, node number per elt, 1 bdy marker */
  fprintf(inm, "%d %d %d\n\n",ne,mesh->dim+1,1);

  ne = 0;
  for ( k=1; k<=mesh->nt; ++k ) {
    pt = &mesh->tria[k];
    if ( MG_EOK(pt) ) {
      /* Save elt idx */
      fprintf(inm, "%d ",++ne);

      /* Save connectivity */
      for ( i=0; i<=mesh->dim; ++i ) {
        fprintf(inm, "%d ",mesh->point[pt->v[i]].tmp);
      }

      /* Save bdy marker */
      fprintf(inm, "%d\n",pt->ref);
    }
  }
  fprintf(stdout,"     NUMBER OF ELEMENT       %8d\n",ne);

  fclose(inm);

  return 1;
}

static inline
int MMG2D_saveNeigh(MMG5_pMesh mesh,const char *filename) {
  FILE*             inm;
  MMG5_pTria        pt;
  int               k,i,ne,idx;
  char              *ptr,*data;

  if ( !mesh->nt ) {
    return 1;
  }

  if ( (!filename) || !(*filename) ) {
    filename = mesh->nameout;
  }
  if ( (!filename) || !(*filename) ) {
    printf("\n  ## Error: %s: unable to save a file without a valid filename\n.",
           __func__);
    return 0;
  }

  /* Name of file */
  MMG5_SAFE_CALLOC(data,strlen(filename)+7,char,return 0);
  strcpy(data,filename);
  ptr = strstr(data,".node");
  if ( ptr ) {
    *ptr = '\0';
  }

  /* Add .node ext  */
  strcat(data,".neigh");
  if( !(inm = fopen(data,"wb")) ) {
    fprintf(stderr,"  ** UNABLE TO OPEN %s.\n",data);
    MMG5_SAFE_FREE(data);
    return 0;
  }

  fprintf(stdout,"  %%%% %s OPENED\n",data);
  MMG5_SAFE_FREE(data);

  if ( ! mesh->adja ) {
    if ( !MMG2D_hashTria(mesh) ) {
      printf("\n  ## Error: %s: unable to compute triangle adjacencies\n.",__func__);
      return 0;
    }
  }

  ne    = 0;
  for (k=1; k<=mesh->nt; k++) {
    pt = &mesh->tria[k];
    if ( !MG_EOK(pt) ) continue;
    ne++;
  }

  /* Save elt number, number of neighbors per elt */
  fprintf(inm, "%d %d\n\n",ne,mesh->dim+1);

  ne = 0;
  for ( k=1; k<=mesh->nt; ++k ) {
    pt = &mesh->tria[k];
    if ( MG_EOK(pt) ) {
      /* Save elt idx */
      fprintf(inm, "%d ",++ne);

      /* Save neighbors */
      for ( i=1; i<=mesh->dim+1; ++i ) {
        /* The triangle conventions is that no neighbors <=> -1 */
        idx = ( mesh->adja[3*(k-1)+i] > 0 ) ? mesh->adja[3*(k-1)+i]/3 : -1;
        fprintf(inm, "%d ",idx);
      }
      /* Save bdy marker */
      fprintf(inm, "\n");
    }
  }

  fclose(inm);

  return 1;
}

int MMG2D_saveTetgenMesh(MMG5_pMesh mesh,const char *filename) {
  MMG5_pPoint ppt;
  MMG5_pEdge  ped;
  int k;

  /** Hack to impose edge reference over its nodes extremities */
  /* Reset reference */
  for ( k=1; k<=mesh->np; ++k ) {
    ppt = &mesh->point[k];
    if ( MG_VOK(ppt) ) {
      ppt->ref  = 0;
      ppt->flag = 0;
    }
  }

  for ( k=1; k<=mesh->na; ++k ) {
    ped = &mesh->edge[k];

    ppt = &mesh->point[ped->a];
    assert ( MG_VOK(ppt) );
    if ( !ppt->flag ) {
      /* Point is seen for the first time */
      ppt->ref  = ped->ref;
      ppt->flag = 1;
    }
    else {
      if ( ped->ref != ppt->ref ) {
        /* Point belongs to two edges with different references */
        ppt->ref = 0;
      }
    }
  }

  if ( !MMG5_saveNode(mesh,filename) ) {
    return 0;
  }

  if ( !MMG2D_saveEle(mesh,filename) ) {
    return 0;
  }

  if ( !MMG5_saveEdge(mesh,filename) ) {
    return 0;
  }

  if ( !MMG2D_saveNeigh(mesh,filename) ) {
    return 0;
  }

  return 1;
}
