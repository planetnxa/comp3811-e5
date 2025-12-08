#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <print>
#include <numbers>
#include <typeinfo>
#include <stdexcept>

#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"

#include "defaults.hpp"
#include "cylinder.hpp"
#include "loadcustom.hpp"

using namespace std;

namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - Simple Lighting";

	constexpr float kMovementPerSecond_ = 5.f; // units per second
	constexpr float kMouseSensitivity_ = 0.01f; // radians per pixel

	struct State_
	{
		ShaderProgram* prog;

		struct CamCtrl_
		{
			bool cameraActive;
			bool actionZoomIn, actionZoomOut;
			
			float phi, theta;
			float radius;

			float lastX, lastY;
		} camControl;
	};

	void glfw_callback_error_( int, char const* );

	void glfw_callback_key_( GLFWwindow*, int, int, int, int );
	void glfw_callback_motion_( GLFWwindow*, double, double );

	struct GLFWCleanupHelper
	{
		~GLFWCleanupHelper();
	};
	struct GLFWWindowDeleter
	{
		~GLFWWindowDeleter();
		GLFWwindow* window;
	};
}

int main() try
{
	// Initialize GLFW
	if( GLFW_TRUE != glfwInit() )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwInit() failed with '{}' ({})", msg, ecode );
	}

	// Ensure that we call glfwTerminate() at the end of the program.
	GLFWCleanupHelper cleanupHelper;

	// Configure GLFW and create window
	glfwSetErrorCallback( &glfw_callback_error_ );

	glfwWindowHint( GLFW_SRGB_CAPABLE, GLFW_TRUE );
	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );

	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	glfwWindowHint( GLFW_DEPTH_BITS, 24 );

#	if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
#	endif // ~ !NDEBUG
	glfwWindowHint(GLFW_DEPTH_BITS, 24);

	GLFWwindow* window = glfwCreateWindow(
		1280,
		720,
		kWindowTitle,
		nullptr, nullptr
	);

	if( !window )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwCreateWindow() failed with '{}' ({})", msg, ecode );
	}

	GLFWWindowDeleter windowDeleter{ window };

	// Set up event handling
	State_ state{};

	glfwSetWindowUserPointer( window, &state );

	glfwSetKeyCallback( window, &glfw_callback_key_ );
	glfwSetCursorPosCallback( window, &glfw_callback_motion_ );

	// Set up drawing stuff
	glfwMakeContextCurrent( window );
	glfwSwapInterval( 1 ); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if( !gladLoadGLLoader( (GLADloadproc)&glfwGetProcAddress ) )
		throw Error( "gladLoadGLLoader() failed - cannot load GL API!" );

	std::print( "RENDERER {}\n", (char const*)glGetString( GL_RENDERER ) );
	std::print( "VENDOR {}\n", (char const*)glGetString( GL_VENDOR ) );
	std::print( "VERSION {}\n", (char const*)glGetString( GL_VERSION ) );
	std::print( "SHADING_LANGUAGE_VERSION {}\n", (char const*)glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	// Ddebug output
#	if !defined(NDEBUG)
	setup_gl_debug_output();
#	endif // ~ !NDEBUG

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	// TODO: global GL setup goes here


	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glClearColor(0.8f, 0.3f, 0.6f, 0.f);

	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize( window, &iwidth, &iheight );

	glViewport( 0, 0, iwidth, iheight );

	// Load shader program
	ShaderProgram prog( {
		{ GL_VERTEX_SHADER, "assets/ex5/default.vert" },
		{ GL_FRAGMENT_SHADER, "assets/ex5/default.frag" }
	} );

	state.prog = &prog;
	state.camControl.radius = 10.f;

	// Animation state
	auto last = Clock::now();

	float angle = 0.f;

	// Create vertex buffers and VAO
	//TODO: create VBOs and VAO

	auto testCylinderX = make_cylinder(true, 129, { 0.8f,0.4f,0.6f }, make_translation({ 5.f,0.f,1.f }) * make_rotation_z(numbers::pi_v<float> / 2) * make_scaling(8.f, 2.f, 2.f));
	auto testCylinderY = make_cylinder(true, 16, { 0.f,1.f,0.f }, make_rotation_y(numbers::pi_v<float> / 2) * make_scaling(5.f, 0.1f, 0.1f));
	auto testCylinderZ = make_cylinder(true, 16, { 0.f,0.f,1.f }, make_rotation_z(numbers::pi_v<float> / 2) * make_scaling(5.f, 0.1f, 0.1f));

	//auto testConeX = make_cone(true, 16, { 0.f,0.f,0.f }, make_scaling(1.f, 0.3f, 0.3f) * make_translation({ 5.f, 0.f, 0.f }));
	//auto testConeY = make_cone(true, 16, { 0.f,0.f,0.f }, make_rotation_y(numbers::pi_v<float> / 2) * make_scaling(1.f, 0.3f, 0.3f) * make_translation({ 5.f, 0.f, 0.f }));
	//auto testConeZ = make_cone(true, 16, { 0.f,0.f,0.f }, make_rotation_z(numbers::pi_v<float> / 2) * make_scaling(1.f, 0.3f, 0.3f) * make_translation({ 5.f, 0.f, 0.f }));

	//auto arrowSet = concatenate(concatenate(move(testCylinderX), move(testCylinderY)), move(testCylinderZ));
	//auto arrowX = concatenate(move(testCylinderX), testConeX);
	//auto arrowY = concatenate(move(testCylinderY), testConeY);
	//auto arrowZ = concatenate(move(testCylinderZ), testConeZ);
	//auto arrowSet = concatenate(concatenate(move(arrowX), arrowY), arrowZ);

	auto shadyArmadillo = load_simple_binary_mesh("assets/ex5/Armadillo.comp3811bin");
	//auto armadillo = load_wavefront_obj("assets/ex4/Armadillo.obj"); //intelli said models???

	//auto newDillo = SimpleMeshData{ armadillo.positions,armadillo.colors };

	GLuint dilloVao = create_vao(shadyArmadillo);
	GLuint vao = create_vao(testCylinderX);
	//GLuint vaoDillo = create_vao(armadillo);
	//int dilloVertexCount = armadillo.positions.s ize();

	int vertexCount = testCylinderX.positions.size();
	
	int dilloVertexCount = shadyArmadillo.positions.size();


	// Main loop
	while( !glfwWindowShouldClose( window ) )
	{
		// Let GLFW process events
		glfwPollEvents();
		
		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize( window, &nwidth, &nheight );

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if( 0 == nwidth || 0 == nheight )
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize( window, &nwidth, &nheight );
				} while( 0 == nwidth || 0 == nheight );
			}

			glViewport( 0, 0, nwidth, nheight );
		}

		// Update state
		auto const now = Clock::now();
		float dt = std::chrono::duration_cast<Secondsf>(now-last).count();
		last = now;


		angle += dt * std::numbers::pi_v<float> * 0.3f;
		if( angle >= 2.f*std::numbers::pi_v<float> )
			angle -= 2.f*std::numbers::pi_v<float>;

		// Update camera state
		if( state.camControl.actionZoomIn )
			state.camControl.radius -= kMovementPerSecond_ * dt;
		else if( state.camControl.actionZoomOut )
			state.camControl.radius += kMovementPerSecond_ * dt;

		if( state.camControl.radius <= 0.1f )
			state.camControl.radius = 0.1f;
 
		// Update: compute matrices
		Mat44f model2world = make_rotation_y(0);

		if (state.camControl.actionZoomIn)
			state.camControl.radius -= kMovementPerSecond_ * dt;
		else if (state.camControl.actionZoomOut)
			state.camControl.radius += kMovementPerSecond_ * dt;

		// clamp radius to avoid going through the camera
		if (state.camControl.radius < 0.1f)
			state.camControl.radius = 0.1f;


		Mat44f model2worldB = model2world * make_translation({ 3.f,0.f,0.f });
		// movve it over 3x

		Mat44f world2camera = make_translation({ 0.f,0.f,-50.f });
		Mat44f proj = make_perspective_projection(std::numbers::pi_v<float> / 3.f, fbwidth / fbheight, 0.1f, 100.f);

		Mat44f Rx = make_rotation_x(state.camControl.theta);
		Mat44f Ry = make_rotation_y(state.camControl.phi);
		Mat44f T = make_translation({ 0.f,0.f,state.camControl.radius });
		world2camera = world2camera * Rx * Ry * T; // order matters omgggggg!

		Mat44f projCameraWorld = proj * world2camera * model2world;
		Mat44f projCameraWorldB = proj * world2camera * model2worldB;

		Mat33f normalMatrix = mat44_to_mat33(transpose(invert(model2world)));

		//TODO: define and compute projCameraWorld matrix

		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		//TODO: draw frame
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(prog.programId());

		Vec3f lightDir = normalize(Vec3f{ -1.f, 1.f, 0.5f });

		glUniform3fv(2, 1, &lightDir.x);
		glUniform3f(3, 0.9f, 0.9f,0.6f);
		glUniform3f(4, 0.05f, 0.05f,0.05f);

		glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix.v);
		glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v);


		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glBindVertexArray(vao);

		glDrawArrays(GL_TRIANGLES, 0, vertexCount);


		glBindVertexArray(dilloVao);
		glDrawArrays(GL_TRIANGLES, 0, dilloVertexCount);
		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	state.prog = nullptr;

	//TODO: additional cleanup
	
	return 0;
}
catch( std::exception const& eErr )
{
	std::print( stderr, "Top-level Exception ({}):\n", typeid(eErr).name() );
	std::print( stderr, "{}\n", eErr.what() );
	std::print( stderr, "Bye.\n" );
	return 1;
}


namespace
{
	void glfw_callback_error_( int aErrNum, char const* aErrDesc )
	{
		std::print( stderr, "GLFW error: {} ({})\n", aErrDesc, aErrNum );
	}

	void glfw_callback_key_( GLFWwindow* aWindow, int aKey, int, int aAction, int )
	{
		if( GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction )
		{
			glfwSetWindowShouldClose( aWindow, GLFW_TRUE );
			return;
		}

		if( auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow )) )
		{
			// R-key reloads shaders.
			if( GLFW_KEY_R == aKey && GLFW_PRESS == aAction )
			{
				if( state->prog )
				{
					try
					{
						state->prog->reload();
						std::print( stderr, "Shaders reloaded and recompiled.\n" );
					}
					catch( std::exception const& eErr )
					{
						std::print( stderr, "Error when reloading shader:\n" );
						std::print( stderr, "{}\n", eErr.what() );
						std::print( stderr, "Keeping old shader.\n" );
					}
				}
			}

			// Space toggles camera
			if( GLFW_KEY_SPACE == aKey && GLFW_PRESS == aAction )
			{
				state->camControl.cameraActive = !state->camControl.cameraActive;

				if( state->camControl.cameraActive )
					glfwSetInputMode( aWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN );
				else
					glfwSetInputMode( aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
			}

			// Camera controls if camera is active
			if( state->camControl.cameraActive )
			{
				if( GLFW_KEY_W == aKey )
				{
					if( GLFW_PRESS == aAction )
						state->camControl.actionZoomIn = true;
					else if( GLFW_RELEASE == aAction )
						state->camControl.actionZoomIn = false;
				}
				else if( GLFW_KEY_S == aKey )
				{
					if( GLFW_PRESS == aAction )
						state->camControl.actionZoomOut = true;
					else if( GLFW_RELEASE == aAction )
						state->camControl.actionZoomOut = false;
				}
			}
		}
	}

	void glfw_callback_motion_( GLFWwindow* aWindow, double aX, double aY )
	{
		if( auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow )) )
		{
			if( state->camControl.cameraActive )
			{
				auto const dx = float(aX-state->camControl.lastX);
				auto const dy = float(aY-state->camControl.lastY);

				state->camControl.phi += dx*kMouseSensitivity_;
				
				state->camControl.theta += dy*kMouseSensitivity_;
				if( state->camControl.theta > std::numbers::pi_v<float>/2.f )
					state->camControl.theta = std::numbers::pi_v<float>/2.f;
				else if( state->camControl.theta < -std::numbers::pi_v<float>/2.f )
					state->camControl.theta = -std::numbers::pi_v<float>/2.f;
			}

			state->camControl.lastX = float(aX);
			state->camControl.lastY = float(aY);
		}
	}
}

namespace
{
	GLFWCleanupHelper::~GLFWCleanupHelper()
	{
		glfwTerminate();
	}

	GLFWWindowDeleter::~GLFWWindowDeleter()
	{
		if( window )
			glfwDestroyWindow( window );
	}
}

