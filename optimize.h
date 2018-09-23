#pragma once
#include "geometry.h"
#include "ppc.h"
#include <opencv2/core.hpp>
// optimizer functions

float errorFunction(float params[3], bool debug = false);
float powellError(float *p);
float stitchingError(const PPC * refPPC, cv::Mat &refIm, const PPC * objPPC, cv::Mat &objIm);
float optimize(float(*energy)(float[3]), float x[3], const PPC *refPPC, const cv::Mat &refImage, const cv::Mat &objImage);

