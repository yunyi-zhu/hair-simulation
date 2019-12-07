#include "gl.h"
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <nanogui/nanogui.h>

#include "vertexrecorder.h"
#include "starter3_util.h"
#include "camera.h"
#include "timestepper.h"
#include "hairsystem.h"
#include "hairgroup.h"

using namespace std;
namespace ng = ::nanogui;

namespace {
// Declarations of functions whose implementations occur later.
  void initSystem();

  void stepSystem();

  void drawSystem();

  void freeSystem();

  void resetTime();

  void initRendering();

  void drawAxis();

// Some constants
const Vector3f LIGHT_POS(3.0f, 3.0f, -5.0f);
const Vector3f LIGHT_COLOR(120.0f, 120.0f, 120.0f);
const Vector3f FLOOR_COLOR(1.0f, 0.0f, 0.0f);

// time keeping
// current "tick" (e.g. clock number of processor)
  uint64_t start_tick;
// number of seconds since start of program
  double elapsed_s;
// number of seconds simulated
  double simulated_s;

// Globals here.
  TimeStepper *timeStepper;
  float h;
  char integrator;
  GLFWwindow *window;
  ng::Screen *screen;

  Camera camera;
  bool gMousePressed = false;
  GLuint program_color;
  GLuint program_light;
  VertexRecorder rec;

  HairGroup *hairGroup;

// Function implementations
  static void keyCallback(GLFWwindow *window, int key,
                          int scancode, int action, int mods) {
    if (action == GLFW_RELEASE) { // only handle PRESS and REPEAT
      return;
    }

    // Special keys (arrows, CTRL, ...) are documented
    // here: http://www.glfw.org/docs/latest/group__keys.html
    switch (key) {
      case GLFW_KEY_ESCAPE: // Escape key
        exit(0);
        break;
      case ' ': {
        Matrix4f eye = Matrix4f::identity();
        camera.SetRotation(eye);
        camera.SetCenter(Vector3f(0, 0, 0));
        break;
      }
      case 'R': {
        cout << "Resetting simulation\n";
        freeSystem();
        initSystem();
        resetTime();
        break;
      }
      default:
        cout << "Unhandled key press " << key << "." << endl;
    }
  }

  static void mouseCallback(GLFWwindow *window, int button, int action, int mods) {
    double xd, yd;
    glfwGetCursorPos(window, &xd, &yd);
    int x = (int) xd;
    int y = (int) yd;

    int lstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    int rstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    int mstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
    if (lstate == GLFW_PRESS) {
      gMousePressed = true;
      camera.MouseClick(Camera::LEFT, x, y);
    } else if (rstate == GLFW_PRESS) {
      gMousePressed = true;
      camera.MouseClick(Camera::RIGHT, x, y);
    } else if (mstate == GLFW_PRESS) {
      gMousePressed = true;
      camera.MouseClick(Camera::MIDDLE, x, y);
    } else {
      gMousePressed = true;
      camera.MouseRelease(x, y);
      gMousePressed = false;
    }
  }

  static void motionCallback(GLFWwindow *window, double x, double y) {
    if (!gMousePressed) {
      return;
    }
    camera.MouseDrag((int) x, (int) y);
  }

  void setViewport(GLFWwindow *window) {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);

    camera.SetDimensions(w, h);
    camera.SetViewport(0, 0, w, h);
    camera.ApplyViewport();
  }

  void drawAxis() {
    glUseProgram(program_color);
    Matrix4f M = Matrix4f::translation(camera.GetCenter()).inverse();
    camera.SetUniforms(program_color, M);

    const Vector3f DKRED(1.0f, 0.5f, 0.5f);
    const Vector3f DKGREEN(0.5f, 1.0f, 0.5f);
    const Vector3f DKBLUE(0.5f, 0.5f, 1.0f);
    const Vector3f GREY(0.5f, 0.5f, 0.5f);

    const Vector3f ORGN(0, 0, 0);
    const Vector3f AXISX(5, 0, 0);
    const Vector3f AXISY(0, 5, 0);
    const Vector3f AXISZ(0, 0, 5);

    VertexRecorder recorder;
    recorder.record_poscolor(ORGN, DKRED);
    recorder.record_poscolor(AXISX, DKRED);
    recorder.record_poscolor(ORGN, DKGREEN);
    recorder.record_poscolor(AXISY, DKGREEN);
    recorder.record_poscolor(ORGN, DKBLUE);
    recorder.record_poscolor(AXISZ, DKBLUE);

    recorder.record_poscolor(ORGN, GREY);
    recorder.record_poscolor(-AXISX, GREY);
    recorder.record_poscolor(ORGN, GREY);
    recorder.record_poscolor(-AXISY, GREY);
    recorder.record_poscolor(ORGN, GREY);
    recorder.record_poscolor(-AXISZ, GREY);

    glLineWidth(3);
    recorder.draw(GL_LINES);
  }


// initialize your particle systems
  void initSystem() {
    switch (integrator) {
      case 'e':
        timeStepper = new ForwardEuler();
        break;
      case 't':
        timeStepper = new Trapezoidal();
        break;
      case 'r':
        timeStepper = new RK4();
        break;
      default:
        printf("Unrecognized integrator\n");
        exit(-1);
    }

    hairGroup = new HairGroup();
  }

  void freeSystem() {
    delete timeStepper;
    timeStepper = nullptr;
    delete hairGroup;
    hairGroup = nullptr;
  }

  void resetTime() {
    elapsed_s = 0;
    simulated_s = 0;
    start_tick = glfwGetTimerValue();
  }

  void stepSystem() {
    // step until simulated_s has caught up with elapsed_s.
    while (simulated_s < elapsed_s) {
      hairGroup->step(timeStepper, h);
      simulated_s += h;
    }
  }

// Draw the current particle positions
  void drawSystem() {
    // GLProgram wraps up all object that
    // particle systems need for drawing themselves
    GLProgram gl(program_light, program_color, &camera);
    gl.updateLight(LIGHT_POS, LIGHT_COLOR.xyz()); // once per frame
    hairGroup->draw(gl, rec);

    // set uniforms for floor
    // gl.updateMaterial(FLOOR_COLOR);
    // gl.updateModelMatrix(Matrix4f::translation(0, -5.0f, 0));
    // draw floor
    // drawQuad(50.0f);
  }

  void initRendering() {
    // Clear to black
    glClearColor(0, 0, 0, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

/*
   initializes a simple NanoGUI-based UI
   must call freeGUI() when done.

   This function implements a simple GUI with three sliders
   for each joint. You won't have to touch it, but feel free
   to add your own features.

   The GUI is drawn in the same window as the main application.
   Any mouse and keyboard events, we first send to the GUI. If the
   GUI didn't handle the event, we forward it to the event handler
   functions above.

   Once initialized, the GUI is drawn in the main loop of the application
   The GUI is drawn in the same window as the main application.
   Any mouse and keyboard events, we first send to the GUI. If the
   GUI didn't handle the event, we forward it to the event handler
   functions above.

   Once initialized, the GUI is drawn in the main loop of the
   application.
*/
  void initGUI(GLFWwindow *glfwwin) {
    // Create a nanogui screen and pass the glfw pointer to initialize
    const int FONTSZ = 14;
    const int ROWH = 18;

    screen = new ng::Screen();
    screen->initialize(glfwwin, false);

    ng::Window *window = nullptr;
    ng::Widget *animator = nullptr;

      window = new ng::Window(screen, "Animator 1");
      window->setPosition(ng::Vector2i(10));
      window->setLayout(new ng::BoxLayout(ng::Orientation::Vertical));
      window->setFixedHeight(800);

      // Scrollpanel is broken. Slider drag mouse events not transformed properly
      // ng::VScrollPanel* vspanel = new ng::VScrollPanel(window);
      // vspanel->setLayout(new ng::BoxLayout(ng::Orientation::Vertical));
      // vspanel->setFixedHeight(600);

      animator = new ng::Widget(window);
      animator->setLayout(new ng::BoxLayout(ng::Orientation::Vertical));

      // sample of a button
      ng::Button *btn = new ng::Button(animator, "Take Screenshot");
//      btn->setCallback([glfwwin]() {
//        screencapture(glfwwin);
//      });

      ng::Widget *jointpanel = new ng::Widget(animator);
      jointpanel->setLayout(new ng::BoxLayout(ng::Orientation::Vertical, ng::Alignment::Minimum, 2, 0));

      ng::Label *label = new ng::Label(jointpanel, "MEOW");
      label->setFontSize(FONTSZ);

      for (int dim = 0; dim < 3; ++dim) {

        ng::Widget *panel = new ng::Widget(jointpanel);
        panel->setLayout(new ng::BoxLayout(ng::Orientation::Horizontal, ng::Alignment::Middle, 3, 10));

        char buff[80];
        switch (dim) {
          case 0:
            sprintf(buff, "%s", "x");
            break;
          case 1:
            sprintf(buff, "%s", "y");
            break;
          case 2:
            sprintf(buff, "%s", "z");
            break;
        }

        ng::Label *label = new ng::Label(panel, buff);
        label->setFontSize(FONTSZ);
        label->setFixedSize(ng::Vector2i(10, ROWH));

        ng::Slider *slider = new ng::Slider(panel);
        slider->setFixedWidth(160);
        slider->setFixedHeight(ROWH);
        slider->setValue(0.5);
        slider->setFinalCallback([&](float value) {
          //cout << "Final slider value: " << (int)(value * 100) << endl;
        });

        ng::TextBox *textBox = new ng::TextBox(panel);
        textBox->setFixedSize(ng::Vector2i(40, ROWH));
//        slider->setCallback([textBox, i, dim](float value) {
//          char buff[80];
//          g_jointangles[i][dim] = (value - 0.5f) * 2 * (float) M_PI;
//          sprintf(buff, "%.2f", g_jointangles[i][dim]);
//          textBox->setValue(buff);
//
//          if (skeleton) {
//            // update animation
//            skeleton->setJointTransform(i, g_jointangles[i].x(), g_jointangles[i].y(), g_jointangles[i].z());
//            updateMesh();
//          }
//        });

        //textBox->setFixedSize(ng::Vector2i(40, ROWH));
        textBox->setFontSize(FONTSZ);
        textBox->setAlignment(ng::TextBox::Alignment::Right);

        // update text box and global vars.
        slider->notifyCallback();
      }

    screen->performLayout();

    // nanoGUI wants to handle events.
    // We forward GLFW events to nanoGUI first. If nanoGUI didn't handle
    // the event, we pass it to the handler routine.
    glfwSetCursorPosCallback(glfwwin,
                             [](GLFWwindow *window, double x, double y) {
                               if (gMousePressed) {
                                 // sticky mouse gestures
                                 motionCallback(window, x, y);
                                 return;
                               }
                               if (screen->cursorPosCallbackEvent(x, y)) {
                                 return;
                               }
                               motionCallback(window, x, y);
                             }
    );

    glfwSetMouseButtonCallback(glfwwin,
                               [](GLFWwindow *window, int button, int action, int modifiers) {
                                 if (screen->mouseButtonCallbackEvent(button, action, modifiers)) {
                                   return;
                                 }
                                 mouseCallback(window, button, action, modifiers);
                               }
    );

    glfwSetKeyCallback(glfwwin,
                       [](GLFWwindow *window, int key, int scancode, int action, int mods) {
                         if (screen->keyCallbackEvent(key, scancode, action, mods)) {
                           return;
                         }
                         keyCallback(window, key, scancode, action, mods);
                       }
    );

    glfwSetCharCallback(glfwwin,
                        [](GLFWwindow *, unsigned int codepoint) {
                          screen->charCallbackEvent(codepoint);
                        }
    );

    glfwSetDropCallback(glfwwin,
                        [](GLFWwindow *, int count, const char **filenames) {
                          screen->dropCallbackEvent(count, filenames);
                        }
    );

    glfwSetScrollCallback(glfwwin,
                          [](GLFWwindow *, double x, double y) {
                            screen->scrollCallbackEvent(x, y);
                          }
    );

    glfwSetFramebufferSizeCallback(glfwwin,
                                   [](GLFWwindow *, int width, int height) {
                                     screen->resizeCallbackEvent(width, height);
                                   }
    );
  }

  void freeGUI() {
    delete screen;
    screen = nullptr;
  }
}



// Main routine.
// Set up OpenGL, define the callbacks and start the main loop
int main(int argc, char** argv)
{
    if (argc != 3) {
        printf("Usage: %s <e|t|r> <timestep>\n", argv[0]);
        printf("       e: Integrator: Forward Euler\n");
        printf("       t: Integrator: Trapezoid\n");
        printf("       r: Integrator: RK 4\n");
        printf("\n");
        printf("Try  : %s t 0.001\n", argv[0]);
        printf("       for trapezoid (1ms steps)\n");
        printf("Or   : %s r 0.01\n", argv[0]);
        printf("       for RK4 (10ms steps)\n");
        return -1;
    }

    integrator = argv[1][0];
    h = (float)atof(argv[2]);
    printf("Using Integrator %c with time step %.4f\n", integrator, h);


    GLFWwindow* window = createOpenGLWindow(1024, 1024, "Assignment 3");
    initGUI(window);

    // setup the event handlers
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);
    glfwSetCursorPosCallback(window, motionCallback);

    initRendering();

    // The program object controls the programmable parts
    // of OpenGL. All OpenGL programs define a vertex shader
    // and a fragment shader.
    program_color = compileProgram(c_vertexshader, c_fragmentshader_color);
    if (!program_color) {
        printf("Cannot compile program\n");
        return -1;
    }
    program_light = compileProgram(c_vertexshader, c_fragmentshader_light);
    if (!program_light) {
        printf("Cannot compile program\n");
        return -1;
    }

    camera.SetDimensions(600, 600);
    camera.SetPerspective(50);
    camera.SetDistance(10);

    // Setup particle system
    initSystem();

    // Main Loop
    uint64_t freq = glfwGetTimerFrequency();
    resetTime();
    while (!glfwWindowShouldClose(window)) {
        // Clear the rendering window
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw nanogui
      screen->drawContents();
      screen->drawWidgets();
      glEnable(GL_DEPTH_TEST);

        setViewport(window);

        if (gMousePressed) {
            drawAxis();
        }

        uint64_t now = glfwGetTimerValue();
        elapsed_s = (double)(now - start_tick) / freq;
        stepSystem();


        // Draw the simulation
        drawSystem();

        // Make back buffer visible
        glfwSwapBuffers(window);

        // Check if any input happened during the last frame
        glfwPollEvents();
    }

    freeGUI();
    // All OpenGL resource that are created with
    // glGen* or glCreate* must be freed.
    glDeleteProgram(program_color);
    glDeleteProgram(program_light);


    return 0;	// This line is never reached.
}
