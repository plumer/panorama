#include "optimize.h"
#include "powell/powell.h"
#include <omp.h>
#include <atomic>
const PPC * refCamera = nullptr;
cv::Mat refImage, objImage;

float errorFunction(float powell_params[3], bool debug) {
	static int count = 0;
	count++;
	// params[0]: pan
	// params[1]: tilt
	// params[2]: roll

	PPC testCamera = *refCamera;
	testCamera.Pan(powell_params[0]);
	testCamera.Tilt(powell_params[1]);
	testCamera.Roll(powell_params[2]);

	return stitchingError(refCamera, refImage, &testCamera, objImage);
}

float powellError(float *powell_params) {
	return errorFunction(powell_params + 1);
}

float stitchingError(const PPC * refPPC, cv::Mat & refIm, const PPC * objPPC, cv::Mat & objIm) {
	Matrix3f MRef{ refPPC->a, refPPC->b, refPPC->c };
	Matrix3f MObj{ objPPC->a, objPPC->b, objPPC->c };
	Matrix3f M = MRef.inverted()*MObj;

	static int count = 0;
	count++;

	assert(objIm.channels() == 3);
	assert(objIm.depth() == CV_32F);
	assert(refIm.channels() == 3);
	assert(refIm.depth() == CV_32F);

	// for every pixel in object image _uv1_
	//   find its coordinate, _uv0_ in reference image
	//   if _uv0_ is out of boundary
	//     continue
	//   else
	//     get the pixel _uv0_ in reference image, possibly using bilinear interpolation
	//     calculate color difference
	//     make 1 contribution to number of pixels counted
	std::atomic_int pCount = 0;
	std::atomic_int64_t sumDiffInt = 0;
#pragma omp parallel for
	for (int r = 0; r < objIm.rows; ++r) {
		for (int c = 0; c < objIm.cols; ++c) {
			Vector3f uv1{ float(c) + 0.5f, float(r) + 0.5f, 1.0f };
			Vector3f uv0 = M * uv1;
			uv0 /= uv0.z;
			if (uv0.x < 0 || uv0.x > refIm.cols - 1 || uv0.y < 0 || uv0.y > refIm.rows - 1)
				continue;

			cv::Vec3f refColor = refIm.at<cv::Vec3f>(uv0.y, uv0.x);
			if (false) {
				cv::Point2i ulC(uv0.x, uv0.y), urC(uv0.x + 0.5, uv0.y);
				cv::Point2i llC(uv0.x, uv0.y + 0.5), lrC(uv0.x + 0.5, uv0.y + 0.5);
				cv::Vec3f ul = refIm.at<cv::Vec3f>(ulC);
				cv::Vec3f ur = refIm.at<cv::Vec3f>(urC);
				cv::Vec3f ll = refIm.at<cv::Vec3f>(llC);
				cv::Vec3f lr = refIm.at<cv::Vec3f>(lrC);
				float dx = uv0.x - ulC.x, dy = uv0.y - ulC.y;
				cv::Vec3f u = ul * (1 - dx) + ur * dx;
				cv::Vec3f l = ll * (1 - dx) + lr*dx;
				refColor = u * (1 - dy) + l * dy;
			}
			cv::Vec3f objColor = objIm.at<cv::Vec3f>(r, c);
			//Vector3f diffColor{ float(refColor[0]) - objColor[0], float(refColor[1]) - objColor[1], float(refColor[2]) - objColor[2] };
			cv::Vec3f cDiff = objColor - refColor;
			float squareDiff = cDiff.dot(cDiff);
			int squareDiffInt = squareDiff * 65535;

			pCount++;
			sumDiffInt += squareDiffInt;
		}
	}
	assert(sumDiffInt >= 0);
	double sumDiff = sumDiffInt / 65535.0;

	//std::cout << "count = " << count << ", error = " << sumDiff / pCount <<  std::endl;

	return float(sumDiff) / pCount;
}

float optimize(float(*energy)(float[3]), float x[3], const PPC *refPPC, const cv::Mat &refIm, const cv::Mat &objIm) {

	// setup the cameras and image references
	if (refPPC == nullptr) return -1.0f;
	refCamera = refPPC;
	refImage = refIm;
	objImage = objIm;

	// prepare workspace for powell.
	int n = 3;
	float **xi = new float *[n + 1];
	for (int i = 0; i < n + 1; ++i) {
		xi[i] = new float[n + 1];
		for (int j = 0; j < n + 1; ++j) xi[i][j] = 0.0f;
		xi[i][i] = 3.0f;
	}

	// I don't know why powell uses subscript from 1 to n.
	float *p = new float[n + 1];
	for (int i = 0; i < n; ++i) {
		p[i + 1] = x[i];
	}

	int iter = 1;
	float fret = 30000.f;
	powell(p, xi, n, 0.0001, &iter, &fret, energy, 30);

	// copy the solution back.
	for (int i = 0; i < n; ++i) {
		x[i] = p[i + 1];
	}

	// clear up the space.
	for (int i = 0; i < n + 1; ++i) {
		delete[] xi[i];
	}
	delete[] xi;
	delete[] p;
	return fret;
}