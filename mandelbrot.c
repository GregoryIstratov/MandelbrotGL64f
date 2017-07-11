#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

uint32_t width = 1024, height = 768;
double x_shift = -0.7, y_shift = 0;
double scale = 2.0;
uint32_t iterations = 256;

GLuint program;


static char* file_read(const char* filename)
{
    FILE* in = fopen(filename, "rb");
    if (in == NULL)
    {
        fprintf(stderr, "Failed to open the file '%s': %s", filename, strerror(errno));
        fflush(stderr);
        return NULL;
    }

    size_t res_size = BUFSIZ;
    char* res = (char*)malloc(res_size);
    size_t nb_read_total = 0;

    while (!feof(in) && !ferror(in)) {
        if (nb_read_total + BUFSIZ > res_size) {
            if (res_size > 10 * 1024 * 1024)
                break;
            res_size = res_size * 2;
            res = (char*)realloc(res, res_size);
        }
        char* p_res = res + nb_read_total;
        nb_read_total += fread(p_res, 1, BUFSIZ, in);
    }

    fclose(in);
    res = (char*)realloc(res, nb_read_total + 1);
    res[nb_read_total] = '\0';
    return res;
}

static void print_log(GLuint object)
{
    GLint log_length = 0;
    if (glIsShader(object))
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else if (glIsProgram(object))
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else {
        fprintf(stderr, "printlog: Not a shader or a program\n");
        return;
    }

    if(log_length == 0)
    {
        fprintf(stderr, "OK\n");
        return;
    }

    char* log = (char*)malloc(log_length);

    if (glIsShader(object))
        glGetShaderInfoLog(object, log_length, NULL, log);
    else if (glIsProgram(object))
        glGetProgramInfoLog(object, log_length, NULL, log);

    fprintf(stderr, "%s", log);
    free(log);
}

static GLuint create_shader(const char* filename, GLenum type)
{

    char *source = file_read(filename);

    if (source == NULL)
        return 0;

    const char* sources[] = {source };


    GLuint res = glCreateShader(type);

    glShaderSource(res, 1, sources, NULL);

    glCompileShader(res);
    GLint compile_ok = GL_FALSE;
    glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);

    free(source);

    fprintf(stderr, "%s: ", filename);
    print_log(res);
    if (compile_ok == GL_FALSE) {
        glDeleteShader(res);
        return 0;
    }

    return res;
}


static int init_shaders()
{
    GLint link_ok = GL_FALSE;
    GLuint gs, vs, fs;

    if ((gs = create_shader("screen_quad_gs.glsl", GL_GEOMETRY_SHADER)) == 0) return 0;
    if ((vs = create_shader("screen_quad_gs_vs.glsl", GL_VERTEX_SHADER)) == 0) return 0;
    if ((fs = create_shader("mand_fs.glsl", GL_FRAGMENT_SHADER)) == 0) return 0;

    program = glCreateProgram();

    glAttachShader(program, gs);
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        fprintf(stderr, "glLinkProgram:");
        print_log(program);
        return 0;
    }
    return 1;
}


static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

inline void scale_print()
{
    fprintf(stdout, "scale %g\n", scale);
    fflush(stdout);
}

inline void scale_increase()
{
    scale *= 1.1;
    scale_print();
}

inline void scale_decrease()
{
    if (scale > 0) scale *= 0.9;
    scale_print();
}

inline void iterations_print()
{
    fprintf(stdout, "iterations %i\n", iterations);
    fflush(stdout);
}

inline uint32_t iterations_get_mod()
{
    if (iterations < 256)
        return 1;
    if (iterations < 512)
        return 2;
    if (iterations < 1024)
        return 4;
    if (iterations < 2048)
        return 8;
    if (iterations < 4096)
        return 32;
    if (iterations < 8192)
        return 128;
    if (iterations < 16384)
        return 256;

    return 512;
}

inline void iterations_increase()
{
    iterations += iterations_get_mod();
    iterations_print();
}

inline void iterations_decrease()
{
    if (iterations > 0) iterations -= iterations_get_mod();
    iterations_print();
}

inline void shift_print()
{
    fprintf(stdout, "x %.15f y %.15f\n", x_shift, y_shift);
    fflush(stdout);
}

inline void shift_right()
{
    x_shift += 0.1 * scale;
    shift_print();
}

inline void shift_left()
{
    x_shift -= 0.1 * scale;
    shift_print();
}

inline void shift_up()
{
    y_shift += 0.1 * scale;
    shift_print();
}

inline void shift_down()
{
    y_shift -= 0.1 * scale;
    shift_print();
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (key)
        {
            case GLFW_KEY_1:
            {
                scale_increase();
                return;
            }
            case GLFW_KEY_2:
            {
                scale_decrease();
                return;
            }
            case GLFW_KEY_3:
            {
                iterations_decrease();
                return;
            }
            case GLFW_KEY_4:
            {
                iterations_increase();
                return;
            }
            case GLFW_KEY_RIGHT:
            {
                shift_right();
                return;
            }
            case GLFW_KEY_LEFT:
            {
                shift_left();
                return;
            }
            case GLFW_KEY_UP:
            {
                shift_up();
                return;
            }
            case GLFW_KEY_DOWN:
            {
                shift_down();
                return;
            }
        }
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(int argc, char** argv)
{
    fprintf(stdout,
            "Usage:\n[1][2] - Zoom\n[3][4] - Increase/Decrease iteration count\n[Arrows] - Move Up/Down/Left/Right\n");
    fflush(stdout);

    GLint vao;
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(width, height, "Mandlebrot", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize OpenGL context\n");
        exit(EXIT_FAILURE);
    }

    glfwSwapInterval(1);

    printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);
    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

    if (!init_shaders())
        exit(EXIT_FAILURE);

    glGenVertexArrays(1, &vao);

    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);

        GLint width_location = glGetUniformLocation(program, "width");
        GLint height_location = glGetUniformLocation(program, "height");
        GLint scale_location = glGetUniformLocation(program, "scale");
        GLint x_location = glGetUniformLocation(program, "x_shift");
        GLint y_location = glGetUniformLocation(program, "y_shift");
        GLint iterations_location = glGetUniformLocation(program, "iterations");

        glUniform1i(width_location, width);
        glUniform1i(height_location, height);
        glUniform1d(scale_location, scale);
        glUniform1d(x_location, x_shift);
        glUniform1d(y_location, y_shift);
        glUniform1i(iterations_location, iterations);

        glBindVertexArray(vao);

        glDrawArrays(GL_POINTS, 0, 1);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glfwDestroyWindow(window);
    glDeleteProgram(program);
    glDeleteVertexArrays(1, &vao);


    return 0;
}