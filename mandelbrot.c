#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif

#include <GL/glew.h>
#include <GL/glut.h>


uint32_t width = 1024, height = 768;
double x_shift = -0.782174, y_shift = 0;
double scale = 0.4;
uint32_t iterations = 256;

GLuint vbo_quad;
GLuint program;
GLint attribute_coord2d;


char* file_read(const char* filename) {
    FILE* in = fopen(filename, "rb");
    if (in == NULL) return NULL;

    size_t res_size = BUFSIZ;
    char* res = (char*)malloc(res_size);
    size_t nb_read_total = 0;

    while (!feof(in) && !ferror(in)) {
        if (nb_read_total + BUFSIZ > res_size) {
            if (res_size > 10*1024*1024)
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

void print_log(GLuint object) {
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

GLuint create_shader(const char* filename, GLenum type) {

    char *source = file_read(filename);

    const char* sources[] = { source };


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


int init()
{
    GLdouble quad_vertices[] = {
            -1.0, -1.0,
            1.0, -1.0,
            1.0,  1.0,
            -1.0,  1.0
    };

    glGenBuffers(1, &vbo_quad);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    GLint link_ok = GL_FALSE;
    GLuint vs, fs;
    if ((vs = create_shader("screen_quad_vs.glsl", GL_VERTEX_SHADER))   == 0) return 0;
    if ((fs = create_shader("mand_fs.glsl", GL_FRAGMENT_SHADER)) == 0) return 0;

    program = glCreateProgram();

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        fprintf(stderr, "glLinkProgram:");
        print_log(program);
        return 0;
    }

    const char* attribute_name = "coord2d";
    attribute_coord2d = glGetAttribLocation(program, attribute_name);
    if (attribute_coord2d == -1) {
        fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
        return 0;
    }

    return 1;
}

void onReshape(int screen_width, int screen_height) {
    width = screen_width;
    height = screen_height;
    glViewport(0, 0, width, height);

    fprintf(stdout, "WidthxHeight %ix%i\n", width, height); fflush(stdout);
}

//handles keyboard input (alphanumeric)
void keyboardCallback(unsigned char key, int x, int y) {
    switch(key) {
        case '2':
        {
            scale *= 1.1;
            fprintf(stdout, "scale %lf\n", scale); fflush(stdout);
            break;
        }
        case '1':
        {
            if (scale > 0) scale *= 0.9;
            fprintf(stdout, "scale %lf\n", scale); fflush(stdout);
            break;
        }
        case '4':
        {
            iterations += 4;
            fprintf(stdout, "iterations %i\n", iterations); fflush(stdout);
            break;
        }
        case '3':
        {
            if (iterations > 1) iterations -= 4;
            fprintf(stdout, "iterations %i\n", iterations); fflush(stdout);
            break;
        }
    }
    glutPostRedisplay();
}

//handles special input for arrow keys
void SpecialInput(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_UP:
            y_shift += 0.1/scale;
            break;
        case GLUT_KEY_DOWN:
            y_shift -= 0.1/scale;
            break;
        case GLUT_KEY_LEFT:
            x_shift -= 0.1/scale;
            break;
        case GLUT_KEY_RIGHT:
            x_shift += 0.1/scale;
            break;
    }
    fprintf(stdout, "x_shift %lf y_shift %lf\n", x_shift, y_shift); fflush(stdout);
    glutPostRedisplay();
}

void free_resources() {
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_quad);
}

void onDisplay() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);

    //Every variable that must be passed to fragment shader
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

    glEnableVertexAttribArray(attribute_coord2d);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_quad);
    glVertexAttribPointer(
            attribute_coord2d, // attribute
            2,                 // number of elements per vertex, here (x,y)
            GL_DOUBLE,          // the type of each element
            GL_FALSE,          // take our values as-is
            0,                 // no extra data between each position
            0                  // offset of first element
    );

    //Push each element in buffer_vertices to the vertex shader
    glDrawArrays(GL_QUADS, 0, 4);

    glDisableVertexAttribArray(attribute_coord2d);



    glutSwapBuffers();
}


int main(int argc, char** argv)
{
    fprintf(stdout,"Usage:\n[1][2] - Zoom\n[3][4] - Increase/Decrease iteration count\n[Arrows] - Move Up/Down/Left/Right\n"); fflush(stdout);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
    glutInitWindowSize(width, height);
    glutCreateWindow("Mandelbrot");

    GLenum status = glewInit();
    if (status != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(status));
        exit(EXIT_FAILURE);
    }

    if (!GLEW_VERSION_4_1) {
        fprintf(stderr, "Error: your graphic card does not support OpenGL 4.1\n");
        exit(EXIT_FAILURE);
    }

    if (init()) {
        glutDisplayFunc(onDisplay);
        glutReshapeFunc(onReshape);
        glutKeyboardFunc(keyboardCallback);
        glutSpecialFunc(SpecialInput);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glutMainLoop();
    }

    free_resources();



    return 0;
}