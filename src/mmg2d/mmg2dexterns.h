#include "mmgexterns.h"
#include "libmmg2d_private.h"

extern int    (*MMG2D_defsiz)(MMG5_pMesh ,MMG5_pSol );
extern int    (*MMG2D_intmet)(MMG5_pMesh ,MMG5_pSol ,int ,int8_t ,int ,double );
extern double (*MMG2D_lencurv)(MMG5_pMesh ,MMG5_pSol ,int ,int );
extern int    (*MMG2D_gradsizreq)(MMG5_pMesh ,MMG5_pSol );
extern double (*MMG2D_caltri)(MMG5_pMesh ,MMG5_pSol ,MMG5_pTria );
extern int    (*MMG2D_gradsiz)(MMG5_pMesh ,MMG5_pSol );
extern int    (*MMG2D_resetRef)(MMG5_pMesh);
extern int    (*MMG2D_setref)(MMG5_pMesh,MMG5_pSol);
extern int    (*MMG2D_snapval)(MMG5_pMesh,MMG5_pSol);
