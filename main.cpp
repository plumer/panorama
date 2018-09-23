#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"
#include "geometry.h"
#include "quaternion.h"
#include "ppc.h"
#include "optimize.h"

#include "utilities/shader.h"


#include <iostream>
#include <fstream>
#include <memory>
#include <utility>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

// glfw callbacks
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode);
void mouseButton_callback(GLFWwindow * window, int button, int action, int);
void mouseMove_callback(GLFWwindow * window, double xpos, double ypos);
void framebuffer_callback(GLFWwindow * window, int width, int height);
void mouseScroll_callback(GLFWwindow * window, double xOffset, double yOffset);

void testMath();

int WINDOW_WIDTH = 1024, WINDOW_HEIGHT = 768;

// scene objects: camera
PPC * viewCamera; //
std::unique_ptr<PPC> paintCamera;
std::vector< std::unique_ptr<PPC> > cameras;
PPC oldCamera{ WINDOW_WIDTH, WINDOW_HEIGHT, 90.0f };
glm::mat4 projTrans, viewTrans, modelTrans;
const cv::Vec3f BGCOLOR(0.01f, 0.01f, 0.01f);

// textures
GLuint midCubeTex, leftCubeTex, rightCubeTex, botTex;
constexpr unsigned int CUBEMAP_SIZE = 1024;
cv::Mat midCubeIm, leftCubeIm, rightCubeIm;
std::vector<cv::Mat> images;
void drawImageOnCanvas(const PPC * viewPPC, cv::Mat & canvas, const PPC * refPPC, cv::Mat & refImage, float imGain = 1.0f);

// scene objects: geometry objects
GLuint rectVAO;
void setupVAO();

// User Interface
static bool keys[1024] = { false };
static bool LMB = false, RMB = false;
static glm::vec2 mouseLoc;



// Image gain

// computes how many pixels in im0 are overlapping with im1 (given size)
// and computes the average color of the overlapping area in im0.
std::pair<int, cv::Vec3f> ComputeImageOverlap(const PPC *ppc0, const cv::Mat &im0, const PPC * ppc1, cv::Size size1);

#define SCENE 4

#define DO_OPTIMIZE

// ImGui Variables
namespace imv {
	float errorTR;
	float errorBR;
	float HDRmax = 1.0f;
	bool shouldAdjustHDRRange = false;
	bool manualHDR = false;
};

void autoAdjustHDRRange();

// Texture Mapping
bool sendCVMatToGLTex(cv::Mat m, GLuint tex, bool normalized = false);

int main() {

	static_assert(sizeof(Vector3f) == sizeof(float) * 3, "Vector3f should take 12 bytes");
	static_assert(sizeof(Matrix3f) == sizeof(float) * 9, "Matrix3f should take 36 bytes");

	testMath();
	//return 0;
	// ! GLFW initialization
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// set up the camera
	viewCamera = new PPC{ WINDOW_WIDTH, WINDOW_HEIGHT, 90.0f };
	//viewCamera->PositionAndOrient(Point3f{ 140.0, 140.0, 120.0 }, Point3f::origin(), Vector3f::YBASE);

	// Create window and set context
	GLFWwindow * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Projective Texture Mapping", nullptr, nullptr);
	if (nullptr == window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	ImGui_ImplGlfwGL3_Init(window, true);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouseButton_callback);
	glfwSetCursorPosCallback(window, mouseMove_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_callback);
	glfwSetScrollCallback(window, mouseScroll_callback);

	ImGuiIO & io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("fonts/Roboto-Regular.ttf", 18.f);
	io.Fonts->AddFontDefault();

	glEnable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);

	framebuffer_callback(window, WINDOW_WIDTH, WINDOW_HEIGHT);

	// preparing geometry for rendering
	setupVAO();

	// Set up shaders
	Shader program("shaders/vs.glsl", "shaders/fs.glsl");
	program.compile().link();
	Shader axesProgram("shaders/axes.vs.glsl", "shaders/axes.fs.glsl");
	axesProgram.compile().link();
	Shader objectProgram("shaders/object.vs.glsl", "shaders/object.fs.glsl");
	objectProgram.compile().link();


	// load images
#if SCENE == 0
	const char * prefix = "images/lab/";
	const char * filenames[] = {
		"images/lab/left.tif",
		"images/lab/mid.tif",
		"images/lab/right.tif"
	};
#elif SCENE == 1
	const char * prefix = "images/home/";
	const char * filenames[] = {
		"images/home/right.tif",
		"images/home/top.tif",
		"images/home/bottom.tif"
	};
#elif SCENE == 2
	const char * prefix = "images/lab2/";
	const char * filenames[] = {
		"images/lab2/right.tif",
		"images/lab2/mid.tif",
		"images/lab2/left.tif",
	};
#elif SCENE == 3
	const char * prefix = "images/home2/";
	const char *filenames[] = {
		//"images/home2/left2.tif",
		//"images/home2/left1.tif",
		"images/home2/right1.tif",
		"images/home2/right2.tif",
	};
#elif SCENE == 4
	const char *prefix = "images/rec/";
	const char * filenames[] = {
		"images/rec/right.tif",
		"images/rec/mid.tif",
		"images/rec/left.tif",
	};
#endif
	constexpr unsigned int nImages = sizeof(filenames) / sizeof(filenames[0]);
	float cParams[nImages][3] = {
		{0.0f, 0.0f, 0.0f},
#if SCENE == 0
		{ 8.0259,3.34651,-1.92106 },
		{ 21.5631,-0.00617976,-0.00617976 },
#elif SCENE == 1
		{ -23.4526f,-10.8265f,-0.474968f },
		{ -26.0f,  11.68f, 0.3162f },
#elif SCENE == 2
		{ -15.0f, 0.0f, 0.0f },
		{ -15.0f, 0.0f, 0.0f },
#elif SCENE == 3
		//{ 15.0f, 0.0f, 0.0f },
		//{ 10.0f, 0.0f, 0.0f },
		{ 15.0f, 0.0f, 0.0f },
#elif SCENE == 4
		{ -24.0f, 0.0f, 0.0f },
		{ -24.0f, 0.0f, 0.0f }
#endif
	};
	images.resize(nImages);
	for (int i = 0; i < nImages; ++i) {
		cv::Mat tmp = cv::imread(filenames[i]);
		tmp.convertTo(images[i], CV_32FC3, 1.0 / 255.0);
		if (i != 0) {
			// naive assumption : all images have the same dimension
			// therefore the aspect ratios are all the same.
			assert(images[i].cols == images[0].cols);
			assert(images[i].rows == images[0].rows);
		}

	}
	midCubeIm = cv::Mat(CUBEMAP_SIZE, CUBEMAP_SIZE, CV_32FC3);
	leftCubeIm = cv::Mat(CUBEMAP_SIZE, CUBEMAP_SIZE, CV_32FC3);
	rightCubeIm = cv::Mat(CUBEMAP_SIZE, CUBEMAP_SIZE, CV_32FC3);
	for (int r = 0; r < midCubeIm.rows; ++r) {
		for (int c = 0; c < midCubeIm.cols; ++c) {
			midCubeIm.at<cv::Vec3f>(r, c) = BGCOLOR;
			leftCubeIm.at<cv::Vec3f>(r, c) = BGCOLOR;
			rightCubeIm.at<cv::Vec3f>(r, c) = BGCOLOR;
		}
	}

	//cv::imshow("right", images[0]);
	//cv::imshow("bottom", images[2]);
	//cv::waitKey(0);

	assert(images[0].channels() == 3);
	assert(images[0].depth() == CV_32F);



	cameras.resize(nImages);

	// horizontal field-of-view of Nexus 5X is 67.9f.
	// by default, the aspect ratio is 4:3 or 16:9. 4:3 mostly.
	float hfov = 67.9f;
	if (images[0].rows * 3 == images[0].cols * 4) {
		// cols : rows = width : height = 3:4, so the camera was rotated.
		hfov *= 0.75f;
	}
	else if (images[0].rows * 4 == images[0].cols * 3) {
		// nothing happens
	}
	else {
		std::cerr << "not supported aspect ratio: " << float(images[0].cols) / images[0].rows << std::endl;
		std::cin.get();
		std::exit(1);
	}

	float imGains[nImages] = { 0.0f };
	// >>>>>>>>>>>>>>>>>>>>>>>>> Find relative camera locations >>>>>>>>>>>>>>>>>>>>>>>>

	for (int round = 0; round < 3; ++round) {
		cameras[0].reset(new PPC{ images[0].cols, images[0].rows, hfov });

		// let's assume that the i-th image overlaps with the (i-1)-th image for sure.
		for (int i = 1; i < nImages; ++i) {
#ifdef DO_OPTIMIZE
			optimize(powellError, cParams[i], cameras[i - 1].get(), images[i - 1], images[i]);
#endif
			std::cout << cParams[i][0] << "," << cParams[i][1] << ',' << cParams[i][2] << std::endl;
			cameras[i].reset(new PPC{ *cameras[i - 1] });
			cameras[i]->Pan(cParams[i][0]);
			cameras[i]->Tilt(cParams[i][1]);
			cameras[i]->Roll(cParams[i][2]);
			std::cout << "image #" << i << ": after optimization: error = " << stitchingError(cameras[i - 1].get(), images[i - 1], cameras[i].get(), images[i]) << std::endl;
		}

		paintCamera = std::make_unique<PPC>(CUBEMAP_SIZE, CUBEMAP_SIZE, 90.0f);
		{
			Vector3f x0 = cameras[0]->a, y0 = -cameras[0]->b, z0 = -cameras[0]->GetVD();
			x0.normalize(); y0.normalize(); z0.normalize();
			Vector3f xn = cameras.back()->a, yn = -cameras.back()->b, zn = -cameras.back()->GetVD();
			xn.normalize(); yn.normalize(); zn.normalize();
			Quaternion q0; q0.fromRotMatrix(Matrix3f{ x0, y0, z0 });
			Quaternion qn; qn.fromRotMatrix(Matrix3f{ xn, yn, zn });
			Quaternion qm = Quaternion::slerp(q0, qn, 0.5);
			Matrix3f mm = qm.toRotMatrix();
			viewCamera->PositionAndOrient(viewCamera->C, viewCamera->C - mm.columns[2], mm.columns[1]);
			paintCamera->PositionAndOrient(paintCamera->C, paintCamera->C - mm.columns[2], mm.columns[1]);
		}
		// <<<<<<<<<<<<<<<<<<<<<<<<< Find relative camera locations <<<<<<<<<<<<<<<<<<<<<<<<


		// >>>>>>>>>>>>>>>>>>>>>> Compute number of overlapping pixels >>>>>>>>>>>>>>>>>>>>>

		// NOverlapPixels[i][j] denotes how many pixels overlapped for image _i_ and _j_.
		int NOverlapPixels[nImages][nImages] = { 0 };

		// I[i][j] denotes the average pixel intensity for image _i_, over the region overlapping with image _j_.
		cv::Vec3f AverageI[nImages][nImages];

		const cv::Vec3f L(0.0722, 0.7152, 0.2126);

		for (int i = 0; i < nImages; ++i) {
			for (int j = 0; j < nImages; ++j) if (j != i) {
				std::tie(NOverlapPixels[i][j], AverageI[i][j]) =
					ComputeImageOverlap(cameras[i].get(), images[i], cameras[j].get(), images[j].size());
				float luminance = AverageI[i][j].dot(L);
			}
		}

		std::cout << "\n";
		for (int i = 0; i < nImages; ++i) {
			for (int j = 0; j < nImages; ++j) {
				float luminance = AverageI[i][j].dot(L);
				printf("%d - %5.2f, ", NOverlapPixels[i][j], luminance);
			}
			std::cout << '\n';
		}

		// adjust the image brightness.
		// comparing images between pairs i-1 and i.
		// if AverageI(im_{i-1}, im_{i}) = p and AverageI(im_{i}, im_{i-1}) = q, 
		// then that means the ratio of luminance of im1 to im0 is q/p.
		// the luminance of im_i should be scaled by p/q.

		std::cout << "\n";
		//const int REF_I = 0;
		
		imGains[0] = 1.0f;
		for (int i = 1; i < nImages; ++i) {
			float p = AverageI[i - 1][i].dot(L);
			float q = AverageI[i][i - 1].dot(L);
			imGains[i] = imGains[i - 1] * (p / q);
			printf("Image %d has luminance %f, image %d has luminance %f\n", i - 1, p, i, q);
			printf("Image %d, '%s', is adjusted by a factor of %f.\n", i, filenames[i], imGains[i]);
			images[i] *= imGains[i];
		}

		// <<<<<<<<<<<<<<<<<<<<<< Compute number of overlapping pixels <<<<<<<<<<<<<<<<<<<<<
	}

	// what's the error now?
	for (int i = 1; i < nImages; ++i) {
		std::cout << "image #" << i << ": after optimization: error = " << stitchingError(cameras[i-1].get(), images[i-1], cameras[i].get(), images[i]) << std::endl;
	}

	// Generate cube map
	{
		for (int i = 0; i < nImages; ++i) {
			drawImageOnCanvas(paintCamera.get(), midCubeIm, cameras[i].get(), images[i], imGains[i]);
		}
		paintCamera->Pan(-90.0f);
		for (int i = 0; i < nImages; ++i) {
			drawImageOnCanvas(paintCamera.get(), leftCubeIm, cameras[i].get(), images[i], imGains[i]);
		}
		paintCamera->Pan(180.0f);
		for (int i = 0; i < nImages; ++i) {
			drawImageOnCanvas(paintCamera.get(), rightCubeIm, cameras[i].get(), images[i], imGains[i]);
		}
	}
	std::string stitchedImageFN = prefix;
	if (stitchedImageFN.back() != '/') stitchedImageFN += '/';
	cv::imwrite(stitchedImageFN + "stitched.png", midCubeIm);
	
	// compute brightest pixel
	//{
	//	std::vector < cv::Mat> channels;
	//	cv::split(midCubeIm, channels);
	//	double max = 0.0f; double min = 1.0f;
	//	for (int i = 0; i < channels.size(); ++i) {
	//		cv::minMaxLoc(channels[0], &min, &max);
	//		if (max > imv::HDRmax)
	//			imv::HDRmax = max;
	//	}
	//}

	glGenTextures(1, &midCubeTex);
	glGenTextures(1, &leftCubeTex);
	glGenTextures(1, &rightCubeTex);
	sendCVMatToGLTex(midCubeIm, midCubeTex);
	sendCVMatToGLTex(leftCubeIm, leftCubeTex);
	sendCVMatToGLTex(rightCubeIm, rightCubeTex);


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		glClearColor(0.0, 0.0, 0.0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		{
			ImGui::Begin("Control Panel");
			ImGui::TextWrapped("Press ESC to quit the program.");
			//ImGui::TextWrapped("Use WASD or arrow keys to move the camera.");
			ImGui::TextWrapped("Use Q and E keys to roll the camera.");
			ImGui::TextWrapped("Click and drag the mouse to move the view direction.");
			//ImGui::TextWrapped("Right-click the mouse to look at origin.");
			if (ImGui::Button("Calculate error")) {
				imv::errorTR = stitchingError(cameras[0].get(), images[0], cameras[1].get(), images[1]);
				imv::errorBR = stitchingError(cameras[1].get(), images[1], cameras[2].get(), images[2]);
				imv::errorTR = std::sqrt(imv::errorTR);
				imv::errorBR = std::sqrt(imv::errorBR);
			}
			ImGui::Text("Error is measured in grayscale 0-255.");
			ImGui::Text("Error (mid against right) = %.3f", imv::errorTR);
			ImGui::Text("Error (left against mid) = %.3f", imv::errorBR);
			ImGui::Checkbox("Manual HDR", &imv::manualHDR);
			if (imv::manualHDR) {
				ImGui::SliderFloat("HDRmax", &imv::HDRmax, 0.4f, 1.4f);
			}
			else {
				ImGui::Text("HDR max = %.3f", imv::HDRmax);
			}
			ImGui::End();
		}
		projTrans = viewCamera->GetProjectionTrans(0.1f, 100.0f);
		viewTrans = viewCamera->GetViewTrans();
		modelTrans = glm::mat4();
		Matrix3f abc;// { projCamera->a, projCamera->b, projCamera->c };
		//viewTrans = glm::lookAt(glm::vec3{ -1.0f, 0.0f, 4.0f }, glm::vec3{ 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f });
		program.use();
		program.uniformMat4("proj", projTrans);
		program.uniformMat4("view", viewTrans);
		program.uniformMat4("model", modelTrans);
		program.uniform1f("HDRmax", imv::HDRmax);

		glBindVertexArray(rectVAO);
		glBindTexture(GL_TEXTURE_2D, midCubeTex);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// left side
		modelTrans = glm::rotate(glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
		program.uniformMat4("model", modelTrans);
		glBindTexture(GL_TEXTURE_2D, leftCubeTex);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		modelTrans = glm::rotate(glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
		program.uniformMat4("model", modelTrans);
		glBindTexture(GL_TEXTURE_2D, rightCubeTex);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		if (imv::shouldAdjustHDRRange && imv::manualHDR == false) {
			imv::shouldAdjustHDRRange = false;
			autoAdjustHDRRange();
		}

		ImGui::Render();
		glfwSwapBuffers(window);
	}

	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	return 0;

}

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	else if (key >= 0 && key < 1024 && action == GLFW_PRESS) {
		keys[key] = true;
	}
	else if (key >= 0 && key < 1024 && action == GLFW_RELEASE) {
		keys[key] = false;
	}

	float step = 2.0;
	if (keys[GLFW_KEY_Q]) {
		viewCamera->Roll(-0.5);
	}
	else if (keys[GLFW_KEY_E]) {
		viewCamera->Roll( 0.5);
	}

	ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mode);
}

void mouseButton_callback(GLFWwindow * window, int button, int action, int mode) {
	
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			LMB = true;
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			mouseLoc.x = xpos, mouseLoc.y = ypos;
			oldCamera = *viewCamera;
		}
		else if (action == GLFW_RELEASE) {
			LMB = false;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) RMB = true;
		else if (action == GLFW_RELEASE) {
			RMB = false;
			imv::shouldAdjustHDRRange = true;
		}
	}
	ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mode);
}

void mouseMove_callback(GLFWwindow * window, double xpos, double ypos) {
	if (LMB && ImGui::GetIO().WantCaptureMouse == false) {
		float dx = xpos - mouseLoc.x, dy = ypos - mouseLoc.y;
		*viewCamera = oldCamera;
		viewCamera->Tilt(dy*0.3f);
		viewCamera->Pan(dx*0.3f);
		//std::cout << "view trans updated" << std:: endl;
		imv::shouldAdjustHDRRange = true;
	}
}

void framebuffer_callback(GLFWwindow * window, int width, int height) {
	WINDOW_WIDTH = width;
	WINDOW_HEIGHT = height;
	glViewport(0, 0, width, height);
	projTrans = viewCamera->GetProjectionTrans(0.1f, 1000.0f);

}

void mouseScroll_callback(GLFWwindow * window, double xOffset, double yOffset) {
	if (ImGui::GetIO().WantCaptureMouse == false) {
		//viewCamera->Translate(viewCamera->GetVD() * yOffset * 2.0);
		viewCamera->c += viewCamera->GetVD() * yOffset * 10.0;
		imv::shouldAdjustHDRRange = true;
	}
	ImGui_ImplGlfwGL3_ScrollCallback(window, xOffset, yOffset);
}

void testMath() {
	Point3f origin{ 1.0, 2.0, 1.0 };
	Point3f target{ 5.0, 4.0, 6.0 };
	//Matrix3f rot = Matrix3f::rotate(Vector3f{ 0.5f, 0.6f, 1.0f }, 50.0f);
	Matrix3f rot;
	Quaternion q; q.fromRotMatrix(rot);
	return;
}

void drawImageOnCanvas(const PPC * viewPPC, cv::Mat & canvas, const PPC * objPPC, cv::Mat & objImage, float imGain) {
	Matrix3f Mview{ viewPPC->a, viewPPC->b, viewPPC->c };
	Matrix3f MObj{ objPPC->a, objPPC->b, objPPC->c };

	assert(objImage.channels() == 3);
	assert(objImage.depth() == CV_32F);

	// MObj * uvobj*w = Mview*uvview
	Matrix3f M = MObj.inverted()*Mview;
	for (int u = 0; u < canvas.cols; ++u) {
		for (int v = 0; v < canvas.rows; ++v) {
			Vector3f uvView{ u + 0.5f, v + 0.5f, 1.0f };
			Vector3f uvObj = M * uvView;
			if (uvObj.z < 0)
				continue;
			uvObj /= uvObj.z;
			if (uvObj.x < 0 || uvObj.x > objImage.cols - 1 || uvObj.y < 0 || uvObj.y > objImage.rows - 1)
				continue;

			cv::Vec3f objColor;
			objColor = objImage.at<cv::Vec3f>(uvObj.y, uvObj.x);// * imGain;

			cv::Vec3f canvasColor = canvas.at<cv::Vec3f>(v, u);
			if (canvasColor == BGCOLOR) {
				canvas.at<cv::Vec3f>(v, u) = objColor;
			}
			else {
				// for uvObj, how far is it from the center to the current pixel?
				float d = std::abs(uvObj.x - objImage.cols / 2) / (objImage.cols/2);
				// for now d is in [0, 1]. 0 means at the center, 1 means at the border.
				// d would be the weight for the existing color.
				cv::Vec3f blendedColor = canvasColor*d + objColor*(1-d);
				canvas.at<cv::Vec3f>(v, u) = blendedColor;
			}
		}
	}
}

void setupVAO() {
	float vertices[] = {
		-10.0,  10.0, -10.0,	0.0, 1.0,
		-10.0, -10.0, -10.0, 0.0, 0.0,
			10.0, -10.0, -10.0,  1.0, 0.0,
		-10.0,  10.0, -10.0,	0.0, 1.0,
			10.0, -10.0, -10.0,	1.0, 0.0,
			10.0,  10.0, -10.0,	1.0, 1.0
	};
	glGenVertexArrays(1, &rectVAO);
	GLuint cubeVBO;
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}


std::pair<int, cv::Vec3f> ComputeImageOverlap(const PPC *ppc0, const cv::Mat &im0, const PPC *ppc1, cv::Size size1)
{
	// Compute the number of pixels in im0 that is overlapping im1.
	// Compute the average color of those pixels.
	assert(im0.channels() == 3);
	assert(im0.depth() == CV_32F);

	Matrix3f M0{ ppc0->a, ppc0->b, ppc0->c };
	Matrix3f M1{ ppc1->a, ppc1->b, ppc1->c };
	// M0*uv0 = M1*uv1
	// uv1 = M1^{-1}*M0.
	Matrix3f M = M1.inverted()*M0;

	int pCount = 0;

	// for every pixel in image0, uv0
	//   find its coordinate, uv1 in im1
	//   if uv1 is in boundary
	//     increment the count
	//     tally up the pixel color

	pCount = 0;
	cv::Vec3f sum(0.0, 0.0f, 0.0);

	for (int r = 0; r < im0.rows; ++r) {
		for (int c = 0; c < im0.cols; ++c) {
			Vector3f uv0{ float(c) + 0.5f, float(r) + 0.5f, 1.0f };
			Vector3f uv1 = M*uv0;
			uv1 /= uv1.z;
			if (uv1.x >= 0 && uv1.x < size1.width && uv1.y >= 0 && uv1.y < size1.height) {
				pCount++;
				cv::Vec3f color = im0.at<cv::Vec3f>(r, c);
				sum += color;
			}
		}
	}

	return std::make_pair(pCount, sum / pCount);
}

bool sendCVMatToGLTex(cv::Mat mat, GLuint tex, bool normalize) {
	// TODO: we need to flip the image before sending the data to the texture.
	// Image in OpenCV starts at the top (matrix indexing) but image in OpenGL starts at the bottom (deCartesian coordinate).
	cv::Mat matCopy;
	cv::flip(mat, matCopy, 0);
	if (normalize) {
		double max;
		cv::minMaxLoc(matCopy, nullptr, &max);
		matCopy /= max;
	}
	glBindTexture(GL_TEXTURE_2D, tex);
	GLenum inputDataType;			// char? float? int?
	GLenum inputDataFormat;			// how many channels?
	switch (matCopy.depth()) {
	case CV_8U: inputDataType = GL_UNSIGNED_BYTE; break;
	case CV_8S: inputDataType = GL_BYTE; break;
	case CV_32F: inputDataType = GL_FLOAT; break;
	default: std::cout << "Unknown depth: " << matCopy.depth() << std::endl; return false;
	}

	switch (matCopy.channels())
	{
	case 1: inputDataFormat = GL_RED; break;
	case 2: inputDataFormat = GL_RG; break;
	case 3: inputDataFormat = GL_BGR; break;
	case 4: inputDataFormat = GL_BGRA; break;
	default: std::cout << "Unexpected number of channels: " << matCopy.channels() << std::endl; return false;
	}

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, matCopy.size().width, matCopy.size().height, 0, inputDataFormat, inputDataType, matCopy.data);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, matCopy.size().width, matCopy.size().height, 0, inputDataFormat, inputDataType, matCopy.data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

void autoAdjustHDRRange() {
	// final goal: change imv::HDRmax
	glReadBuffer(GL_BACK);
	cv::Mat buf(WINDOW_HEIGHT*7/8, WINDOW_WIDTH*7/8, CV_32FC3);
	glReadPixels(WINDOW_WIDTH / 16, WINDOW_HEIGHT / 16, WINDOW_WIDTH*7/8, WINDOW_HEIGHT*7/8, GL_BGR, GL_FLOAT, buf.data);
	{
		std::vector <cv::Mat> channels;
		cv::split(buf, channels);
		double max = 0.0f;
		for (int i = 0; i < channels.size(); ++i) {
			double cmax = 0.0f; double cmin = 1.0f;
			cv::minMaxLoc(channels[0], &cmin, &cmax);
			if (cmax > max)
				max = cmax;
		}
		imv::HDRmax = max;
	} 
}