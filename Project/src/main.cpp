
#include "glad.h"
#include "glfw3.h"

#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

const int WIDTH = 1000;
const int HEIGHT = 700;

struct Shape {
    int type;
    float x1, y1, x2, y2;
    float r, g, b;
};

vector<Shape> shapes;

int currentShape = 1;

float red = 0, green = 0, blue = 0;

bool drawing = false;

double startX, startY;
double mouseX, mouseY;

GLuint VAO, VBO, shaderProgram;

GLFWwindow* window;

// Vertex Shader
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// Fragment Shader
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 shapeColor;

void main() {
    FragColor = vec4(shapeColor, 1.0);
}
)";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, NULL, info);
        cout << "Shader Error: " << info << endl;
    }

    return shader;
}

void createShader() {
    GLuint vs = compileShader(
        GL_VERTEX_SHADER, vertexShaderSource
    );

    GLuint fs = compileShader(
        GL_FRAGMENT_SHADER, fragmentShaderSource
    );

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);

    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
        char info[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, info);
        cout << "Link Error: " << info << endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void addPoint(vector<float>& vertices, float x, float y) {
    int w, h;
    glfwGetWindowSize(window, &w, &h);

    float px = (2.0f * x / w) - 1.0f;
    float py = 1.0f - (2.0f * y / h);

    vertices.push_back(px);
    vertices.push_back(py);
}

void renderShape(Shape s) {
    vector<float> vertices;

    if (s.type == 1) {
        // Line
        addPoint(vertices, s.x1, s.y1);
        addPoint(vertices, s.x2, s.y2);
    }

    else if (s.type == 2) {
        // Circle
        float radius = sqrt(
            pow(s.x2 - s.x1, 2) +
            pow(s.y2 - s.y1, 2)
        );

        for (int i = 0; i < 100; i++) {
            float angle = 2.0f * 3.14159f * i / 100;

            float x = s.x1 + radius * cos(angle);
            float y = s.y1 + radius * sin(angle);

            addPoint(vertices, x, y);
        }
    }

    else if (s.type == 3) {
        // Rectangle
        addPoint(vertices, s.x1, s.y1);
        addPoint(vertices, s.x2, s.y1);
        addPoint(vertices, s.x2, s.y2);
        addPoint(vertices, s.x1, s.y2);
    }

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_DYNAMIC_DRAW
    );

    if (s.type == 1)
        glDrawArrays(GL_LINES, 0, 2);

    else if (s.type == 2)
        glDrawArrays(GL_LINE_LOOP, 0, 100);

    else if (s.type == 3)
        glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void display() {
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    GLint colorLocation =
        glGetUniformLocation(shaderProgram, "shapeColor");

    for (Shape s : shapes) {
        glUniform3f(colorLocation, s.r, s.g, s.b);
        renderShape(s);
    }

    // Drawing preview
    if (drawing) {
        Shape preview = {
            currentShape,
            (float)startX, (float)startY,
            (float)mouseX, (float)mouseY,
            red, green, blue
        };

        glUniform3f(colorLocation, red, green, blue);
        renderShape(preview);
    }

    glBindVertexArray(0);

    glfwSwapBuffers(window);
}

void mouseButton(
    GLFWwindow* win,
    int button,
    int action,
    int mods
) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {

        if (action == GLFW_PRESS) {
            glfwGetCursorPos(win, &startX, &startY);

            mouseX = startX;
            mouseY = startY;

            drawing = true;
        }

        else if (action == GLFW_RELEASE && drawing) {
            glfwGetCursorPos(win, &mouseX, &mouseY);

            Shape s = {
                currentShape,
                (float)startX, (float)startY,
                (float)mouseX, (float)mouseY,
                red, green, blue
            };

            shapes.push_back(s);
            drawing = false;
        }
    }
}

void mouseMove(GLFWwindow* win, double x, double y) {
    if (drawing) {
        mouseX = x;
        mouseY = y;
    }
}

void keyboard(
    GLFWwindow* win,
    int key,
    int scancode,
    int action,
    int mods
) {
    if (action != GLFW_PRESS)
        return;

    // Shape selection
    if (key == GLFW_KEY_1) currentShape = 1;
    if (key == GLFW_KEY_2) currentShape = 2;
    if (key == GLFW_KEY_3) currentShape = 3;

    // Color selection
    if (key == GLFW_KEY_R) {
        red = 1; green = 0; blue = 0;
    }

    if (key == GLFW_KEY_G) {
        red = 0; green = 1; blue = 0;
    }

    if (key == GLFW_KEY_B) {
        red = 0; green = 0; blue = 1;
    }

    if (key == GLFW_KEY_K) {
        red = 0; green = 0; blue = 0;
    }

    // Clear board
    if (key == GLFW_KEY_C) {
        shapes.clear();
    }

    // Exit
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(win, true);
    }
}

void resize(GLFWwindow* win, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {

    if (!glfwInit()) {
        cout << "GLFW initialization failed!" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

    window = glfwCreateWindow(
        WIDTH,
        HEIGHT,
        "Interactive Drawing Board",
        NULL,
        NULL
    );

    if (!window) {
        cout << "Window creation failed!" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(
        (GLADloadproc)glfwGetProcAddress
    )) {
        cout << "GLAD initialization failed!" << endl;
        glfwTerminate();
        return -1;
    }

    createShader();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        2 * sizeof(float), (void*)0
    );

    glEnableVertexAttribArray(0);

    glLineWidth(2.0f);

    glfwSetMouseButtonCallback(window, mouseButton);
    glfwSetCursorPosCallback(window, mouseMove);
    glfwSetKeyCallback(window, keyboard);
    glfwSetFramebufferSizeCallback(window, resize);

    cout << "\n===== DRAWING BOARD =====\n";
    cout << "1 = Line\n";
    cout << "2 = Circle\n";
    cout << "3 = Rectangle\n";
    cout << "R = Red\n";
    cout << "G = Green\n";
    cout << "B = Blue\n";
    cout << "K = Black\n";
    cout << "C = Clear Board\n";
    cout << "ESC = Exit\n";
    cout << "Hold LEFT MOUSE and DRAG to draw\n";

    while (!glfwWindowShouldClose(window)) {
        display();
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}
