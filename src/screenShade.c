// Minimal shader tester: GLFW + OpenGL 3.3 core, no GLAD.
// Usage: ./shader path/to/shader.frag      (press Space to reload the file)
// Linux:   gcc main.c -o shader -lglfw -lGL
// Windows: gcc main.c -o shader.exe -lglfw3 -lgdi32 -lopengl32
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

// ---- Load only the GL functions we need ----
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER   0x8B31
#define GL_COMPILE_STATUS  0x8B81
#define GL_LINK_STATUS     0x8B82

typedef GLuint (*PFN_u_e)(GLenum);
typedef void   (*PFN_shsrc)(GLuint, GLsizei, const char *const *, const GLint *);
typedef void   (*PFN_u)(GLuint);
typedef void   (*PFN_uu)(GLuint, GLuint);
typedef GLuint (*PFN_u_v)(void);
typedef GLint  (*PFN_loc)(GLuint, const char *);
typedef void   (*PFN_1f)(GLint, GLfloat);
typedef void   (*PFN_gen)(GLsizei, GLuint *);
typedef void   (*PFN_iv)(GLuint, GLenum, GLint *);
typedef void   (*PFN_log)(GLuint, GLsizei, GLsizei *, char *);

static PFN_u_e   glCreateShader;
static PFN_shsrc glShaderSource;
static PFN_u     glCompileShader, glUseProgram, glLinkProgram, glBindVertexArray,
                 glDeleteShader, glDeleteProgram;
static PFN_uu    glAttachShader;
static PFN_u_v   glCreateProgram;
static PFN_loc   glGetUniformLocation;
static PFN_1f    glUniform1f;
static PFN_gen   glGenVertexArrays;
static PFN_iv    glGetShaderiv, glGetProgramiv;
static PFN_log   glGetShaderInfoLog, glGetProgramInfoLog;

static void loadGL(void) {
#define L(n) n = (void *)glfwGetProcAddress(#n)
    L(glCreateShader); L(glShaderSource); L(glCompileShader); L(glCreateProgram);
    L(glAttachShader); L(glLinkProgram); L(glUseProgram); L(glDeleteShader);
    L(glDeleteProgram); L(glGetUniformLocation); L(glUniform1f);
    L(glGenVertexArrays); L(glBindVertexArray);
    L(glGetShaderiv); L(glGetShaderInfoLog); L(glGetProgramiv); L(glGetProgramInfoLog);
}

// ---- Vertex shader: two triangles covering the screen, from gl_VertexID ----
static const char *VERT =
    "#version 330 core\n"
    "out vec2 UV;\n"
    "const vec2 P[6] = vec2[](vec2(-1,-1), vec2(1,-1), vec2(1,1),\n"
    "                         vec2(-1,-1), vec2(1,1),  vec2(-1,1));\n"
    "void main() {\n"
    "    UV = P[gl_VertexID] * 0.5 + 0.5;\n"
    "    gl_Position = vec4(P[gl_VertexID], 0.0, 1.0);\n"
    "}\n";

static const char *shaderPath;
static GLuint prog = 0;
static GLint uTime = -1, uAspect = -1;

static char *readFile(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", path); return NULL; }
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    char *s = malloc(n + 1);
    s[fread(s, 1, n, f)] = 0;
    fclose(f);
    return s;
}

static GLuint compile(GLenum type, const char *src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048]; glGetShaderInfoLog(s, 2048, NULL, log);
        fprintf(stderr, "Shader compile error:\n%s\n", log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

// Reads the fragment shader from disk and builds the program.
// On any failure the previous working program stays active.
static void initShader(void) {
    printf("Loading %s ...\n", shaderPath);
    char *frag = readFile(shaderPath);
    if (!frag) return;

    GLuint vs = compile(GL_VERTEX_SHADER, VERT);
    GLuint fs = compile(GL_FRAGMENT_SHADER, frag);
    free(frag);
    if (!vs || !fs) { glDeleteShader(vs); glDeleteShader(fs); return; }

    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048]; glGetProgramInfoLog(p, 2048, NULL, log);
        fprintf(stderr, "Link error:\n%s\n", log);
        glDeleteProgram(p);
        return;
    }
    if (prog) glDeleteProgram(prog);
    prog = p;
    glUseProgram(prog);
    uTime = glGetUniformLocation(prog, "iTime");
    uAspect = glGetUniformLocation(prog, "iAspect");
    printf("OK\n");
}

static void onKey(GLFWwindow *w, int key, int sc, int action, int mods) {
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) initShader();
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "Usage: %s <fragment_shader.frag>\n", argv[0]); return 1; }
    shaderPath = argv[1];

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *win = glfwCreateWindow(800, 600, "Shader (Space = reload)", NULL, NULL);
    if (!win) { fprintf(stderr, "Window/context creation failed\n"); return 1; }
    glfwMakeContextCurrent(win);
    glfwSetKeyCallback(win, onKey);
    loadGL();

    GLuint vao; glGenVertexArrays(1, &vao); glBindVertexArray(vao); // required in core profile

    initShader();

    while (!glfwWindowShouldClose(win)) {
        int w, h; glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);
        if (prog) {
            glUniform1f(uTime, (float)glfwGetTime());
            glUniform1f(uAspect, h ? (float)w / h : 1.0f);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
