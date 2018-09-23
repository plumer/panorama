#include <stdio.h>
#include "nrutil.h"

#define TOL 2.0e-4f

int ncom=0;	/* defining declarations */
float *pcom=0,*xicom=0,(*nrfunc)(float *);

void linmin(float p[], float xi[], int n, float *fret, float (*func)(float *))
{
	int j;
	float xx,xmin,fx,fb,fa,bx,ax;
	float brent(float, float, float, float (*f)(float), float, float *);
	float f1dim(float);
	void mnbrak(float *ax,float *bx,float *cx,float *fa, float *fb, float *fc, float (*func)(float));

	ncom=n;
	//pcom=vector(1,n);
	//xicom=vector(1,n);
	pcom = new float[n + 1];
	xicom = new float[n + 1];
	nrfunc=func;
	for (j=1;j<=n;j++) {
		pcom[j]=p[j];
		xicom[j]=xi[j];
	}
	ax=0.0;
	xx=1.0;
	bx=2.0;
	mnbrak(&ax,&xx,&bx,&fa,&fx,&fb,f1dim);
	*fret=brent(ax,xx,bx,f1dim,TOL,&xmin);
	for (j=1;j<=n;j++) {
		xi[j] *= xmin;
		p[j] += xi[j];
	}
	//free_vector(xicom,1,n);
	//free_vector(pcom,1,n);
	delete[] pcom;
	delete[] xicom;
}

#undef TOL
