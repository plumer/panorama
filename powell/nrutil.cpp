#include <stdio.h>
#include <stdlib.h>

#include "nrutil.h"

void nrerror(char error_text[])
{
	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	exit(1);
}


float *vector(int nl,int nh)
{
	float *v;
	
	v=(float *)malloc((unsigned) (nh-nl+1)*sizeof(float));
	if (!v) nrerror("allocation failure in vector()");
	return v-nl;
}


void free_vector(float *v,int nl,int nh)
{
	free((char *) (v+nl));
}


float **matrix(int nrl, int nrh, int ncl, int nch)
{
	int i;
	float **m;

	m = (float **) malloc((unsigned) (nrh-nrl+1)*sizeof(float*));
	if (!m) nrerror("allocation failure 1 in matrix()");
	m -= nrl;
	for (i=nrl;i<=nrh;i++) {
		m[i] = (float *) malloc((unsigned) (nch-ncl+1)*sizeof(float));
		if (!m[i]) nrerror("allocation failure 2 in matrix()");
	 	m[i] -= ncl;
	}
	return m;
}


void free_matrix(float **m, int nrl, int nrh, int ncl, int nch)
{
	int i;

	for(i=nrh;i>=nrl;i--) free((char*) (m[i] + ncl));
	free ((char*) (m + nrl));
}

