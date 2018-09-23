#include <stdio.h>
#include "nrutil.h"

extern int ncom;	/* defined in LINMIN */
extern float *pcom,*xicom,(*nrfunc)(float *);

float f1dim(float x)
{
	int j;
	float f,*xt;

	//xt=vector(1,ncom);
	xt = new float[ncom + 1];
	for (j=1;j<=ncom;j++) xt[j]=pcom[j]+x*xicom[j];
	f=(*nrfunc)(xt);
	//free_vector(xt,1,ncom);
	delete[] xt;
	return f;
}
