#include <math.h>
#include <cstdio>
#include "nrutil.h"

static float sqrarg;
#define POWELL_SQR(a) (sqrarg=(a),sqrarg*sqrarg)

void powell(float p[], float **xi, int n, float ftol, int *iter, float *fret,
            float (*func)(float *), int maxIter)
{
	int i,ibig,j;
	float t,fptt,fp,del;
	float *pt,*ptt,*xit;
	void linmin(float p[], float xi[], int n, float *fret, 
               float (*func)(float *));
	/*
	pt=vector(1,n);
	ptt=vector(1,n);
	xit=vector(1,n);
	*/

	pt = new float[n + 1];
	ptt = new float[n + 1];
	xit = new float[n + 1];

	*fret=(*func)(p);
	for (j=1;j<=n;j++) pt[j]=p[j];
	for (*iter=1;;(*iter)++) {
      printf("ITERATION = %i\n", *iter);
		fp=(*fret);
		ibig=0;
		del=0.0f;
		for (i=1;i<=n;i++) {
 			for (j=1;j<=n;j++) xit[j]=xi[j][i];
			fptt=(*fret);
			linmin(p,xit,n,fret,func);
			if ((float)fabs(fptt-(*fret)) > del) {
				del=(float)fabs(fptt-(*fret));
				ibig=i;
			}
		}
		printf("fret = %f\n", *fret);
		if (2.0f*(float)fabs(fp-(*fret)) <= ftol*((float)fabs(fp)+(float)fabs(*fret))) {
			delete[] pt;
			delete[] xit;
			delete[] ptt;
			/*free_vector(xit,1,n);
			free_vector(ptt,1,n);
			free_vector(pt,1,n);*/
			return;
		}
#if 1
		if (*iter == maxIter) 
      {/*
			free_vector(xit,1,n);
			free_vector(ptt,1,n);
			free_vector(pt,1,n);*/
			delete[] xit;
			delete[] ptt;
			delete[] pt;
			return;
      }
#else
		if (*iter == maxIter) nrerror("Too many iterations in routine POWELL");
#endif

		for (j=1;j<=n;j++) {
			ptt[j]=2.0f*p[j]-pt[j];
			xit[j]=p[j]-pt[j];
			pt[j]=p[j];
		}
		fptt=(*func)(ptt);
		if (fptt < fp) {
			t=2.0f*(fp-2.0f*(*fret)+fptt)*POWELL_SQR(fp-(*fret)-del)-del*POWELL_SQR(fp-fptt);
			if (t < 0.0) {
				linmin(p,xit,n,fret,func);
				for (j=1;j<=n;j++) xi[j][ibig]=xit[j];
			}
		}
	}
}

#undef SQR
