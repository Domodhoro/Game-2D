#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <time.h>
#include <math.h>

#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define FNL_IMPL

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "./lua54/lua.h"
#include "./lua54/lualib.h"
#include "./lua54/lauxlib.h"
#include "./stb/stb_image.h"
#include "./stb/stb_truetype.h"
#include "./FastNoiseLite/FastNoiseLite.h"

typedef struct {
    GLFWwindow* window;
    GLint       width;
    GLint       height;
} Window;

typedef struct {
    GLfloat v[2];
} Vec2;

typedef struct {
    GLfloat v[3];
} Vec3;

typedef struct {
    GLfloat v[4];
} Vec4;

typedef struct {
    GLfloat m[4][4];
} Mat4;

typedef struct {
    GLuint  VAO;
    GLuint  VBO;
    GLuint  EBO;
    Vec3    position;
    Vec2    scale;
    GLfloat angle_of_rotation;
} Mesh;

typedef struct {
    GLuint FBO;
    GLuint Texture;
    GLuint RBO;
} Framebuffer;

static int create_window          (lua_State*);
static int delete_window          (lua_State*);
static int window_should_close    (lua_State*);
static int set_window_should_close(lua_State*);
static int swap_buffers           (lua_State*);
static int clear_color            (lua_State*);
static int poll_events            (lua_State*);
static int delay                  (lua_State*);
static int get_key                (lua_State*);
static int create_framebuffer     (lua_State*);
static int delete_framebuffer     (lua_State*);
static int create_shader          (lua_State*);
static int delete_shader          (lua_State*);
static int load_font              (lua_State*);
static int delete_font            (lua_State*);
static int create_text            (lua_State*);
static int load_texture           (lua_State*);
static int delete_texture         (lua_State*);
static int create_mesh            (lua_State*);
static int delete_mesh            (lua_State*);
static int draw                   (lua_State*);
static int set_position           (lua_State*);
static int set_scale              (lua_State*);
static int set_rotate             (lua_State*);
static int create_noise           (lua_State*);
static int get_noise              (lua_State*);
static int delete_noise           (lua_State*);
static int engine                 (lua_State*);
GLvoid     set_window_icon        (GLFWwindow*, const char*);
char*      read_file              (const char*);
GLuint     compile_vertex_shader  (const GLchar*);
GLuint     compile_fragment_shader(const GLchar*);
GLvoid     setup_VBO              (GLuint*);
GLvoid     setup_EBO              (GLuint*);
Mat4       ortho                  (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
GLvoid     identity               (Mat4*);
GLvoid     scale                  (Mat4*, GLfloat, GLfloat, GLfloat);
GLvoid     rotate                 (Mat4*, GLfloat);
GLvoid     translate              (Mat4*, GLfloat, GLfloat, GLfloat);

int main(int argc, char* argv[]) {
    puts(argv[0]);

    lua_State* L = luaL_newstate();

    luaL_openlibs(L);
    luaL_requiref(L, "engine", engine, 1);

    const GLchar* constant_prefix = "KEY_";
    GLint         key             = 0;

    for (key = GLFW_KEY_A; key <= GLFW_KEY_Z; key++) {
        GLchar constant_name[9];

        snprintf       (constant_name, sizeof(constant_name), "%s%c", constant_prefix, key);
        lua_pushinteger(L, key);
        lua_setglobal  (L, constant_name);
    }

    lua_pushinteger(L, GLFW_KEY_ESCAPE);
    lua_setglobal  (L, "KEY_ESC");

    if (luaL_dofile(L, "./script.lua") == LUA_OK) {
        lua_getglobal(L, "script");
        lua_pcall    (L, 0, 0, 0);
        lua_close    (L);

        return EXIT_SUCCESS;
    }

    printf   ("Error (%s): %s\n", __func__, lua_tostring(L, -1));
    lua_close(L);

    return EXIT_FAILURE;
}

static int create_window(lua_State* L) {
    if (glfwInit() == GLFW_NOT_INITIALIZED) {
        printf("Error (%s): Failed to initialize.\n", __func__);

        return 0;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const GLchar*      title     = luaL_checkstring (L, 1);
    const GLint        width     = luaL_checkinteger(L, 2);
    const GLint        height    = luaL_checkinteger(L, 3);
    const GLchar*      icon_path = luaL_checkstring (L, 4);
    Window*            window    = malloc           (sizeof(Window));
    GLFWvidmode const* mode      = glfwGetVideoMode (glfwGetPrimaryMonitor());

    window->window = glfwCreateWindow(width, height, title, NULL, NULL);
    window->width  = width;
    window->height = height;

    if (window->window != NULL) {
        glfwMakeContextCurrent(window->window);
        glfwSetWindowAttrib   (window->window, GLFW_RESIZABLE, false);
        glfwSetWindowPos      (window->window, (mode->width - window->width) / 2, (mode->height - window->height) / 2);
        set_window_icon       (window->window, icon_path);
        glEnable              (GL_DEPTH_TEST);

        glewExperimental = true;

        if (glewInit() != GLEW_OK) {
            printf           ("Error (%s): Failed to initialize.\n", __func__);
            glfwDestroyWindow(window->window);
            glfwTerminate    ();

            return 0;
        }

        lua_pushlightuserdata(L, window);

        return 1;
    } else {
        printf       ("Error (%s): Failed to create window.", __func__);
        glfwTerminate();

        return 0;
    }
}

static int delete_window(lua_State* L) {
    Window* window = lua_touserdata(L, 1);

    if (window->window != NULL) {
        glfwDestroyWindow(window->window);
        glfwTerminate    ();
        free             (window);
    }

    return 0;
}

static int window_should_close(lua_State* L) {
    Window* window = lua_touserdata(L, 1);

    if (window->window != NULL) {
        lua_pushboolean(L, glfwWindowShouldClose(window->window));

        return 1;
    }

    return 0;
}

static int set_window_should_close(lua_State* L) {
    Window* window = lua_touserdata(L, 1);

    if (window->window != NULL) glfwSetWindowShouldClose(window->window, true);

    return 0;
}

static int clear_color(lua_State* L) {
    const GLclampf red   = (GLclampf)lua_tonumber(L, 1);
    const GLclampf green = (GLclampf)lua_tonumber(L, 2);
    const GLclampf blue  = (GLclampf)lua_tonumber(L, 3);

    glClear     (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(red, green, blue, 1.0f);

    return 0;
}

static int swap_buffers(lua_State* L) {
    Window* window = lua_touserdata(L, 1);

    if (window->window != NULL) glfwSwapBuffers(window->window);

    return 0;
}

static int poll_events(lua_State* L) {
    glfwPollEvents();

    return 0;
}

static int delay(lua_State* L) {
    const   GLdouble seconds = luaL_checknumber(L, 1);
    clock_t start_time       = clock           ();
    clock_t end_time         = start_time + (seconds * CLOCKS_PER_SEC);

    while (clock() < end_time) {};

    return 0;
}

static int get_key(lua_State* L) {
    Window* window    = lua_touserdata   (L, 1);
    const   GLint key = luaL_checkinteger(L, 2);

    if (window->window != NULL) {
        lua_pushboolean(L, glfwGetKey(window->window, key) == GLFW_PRESS);

        return 1;
    }

    return 0;
}

static int create_framebuffer(lua_State* L) {
    Window*      window      = lua_touserdata(L, 1);
    Framebuffer* framebuffer = malloc        (sizeof(Framebuffer));

    if (framebuffer != NULL) {
        glGenFramebuffers(1, &(framebuffer->FBO));
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->FBO);

        glGenTextures         (1, &(framebuffer->Texture));
        glBindTexture         (GL_TEXTURE_2D, framebuffer->Texture);
        glTexImage2D          (GL_TEXTURE_2D, 0, GL_RGB, window->width, window->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri       (GL_TEXTURE_2D,  GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri       (GL_TEXTURE_2D,  GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer->Texture, 0);

        glGenRenderbuffers       (1, &(framebuffer->RBO));
        glBindRenderbuffer       (GL_RENDERBUFFER, framebuffer->RBO);
        glRenderbufferStorage    (GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window->width, window->height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,  GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebuffer->RBO);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            printf("Error (%s): Framebuffer is not complete.\n", __func__);
        }

        glBindFramebuffer    (GL_FRAMEBUFFER, 0);
        lua_pushlightuserdata(L, framebuffer);

        return 1;
    } else {
        printf("Error (%s): Failed to create framebuffer.\n", __func__);

        return 0;
    }
}

static int delete_framebuffer(lua_State* L) {
    Framebuffer* framebuffer = lua_touserdata(L, 1);

    if (framebuffer != NULL) {
        glDeleteRenderbuffers(1, &(framebuffer->RBO));
        glDeleteTextures     (1, &(framebuffer->Texture));
        glDeleteFramebuffers (1, &(framebuffer->FBO));
        free                 (framebuffer);
    }

    return 0;
}

static int create_shader(lua_State* L) {
    const GLchar* vertex_path   = luaL_checkstring(L, 1);
    const GLchar* fragment_path = luaL_checkstring(L, 2);
    GLuint*       shader        = malloc          (sizeof(GLuint));

    if (shader != NULL) {
        *shader = glCreateProgram();

        GLuint vertex   = compile_vertex_shader  (vertex_path);
        GLuint fragment = compile_fragment_shader(fragment_path);

        glAttachShader(*shader, vertex);
        glAttachShader(*shader, fragment);
        glLinkProgram (*shader);
        glDeleteShader(vertex);
        glDeleteShader(fragment);

        lua_pushlightuserdata(L, shader);

        return 1;
    }

    return 0;
}

static int delete_shader(lua_State* L) {
    GLuint* shader = lua_touserdata(L, 1);

    if (shader != NULL) {
        glDeleteProgram(*shader);
        free           (shader);
    }

    return 0;
}

static int load_font(lua_State* L) {
    const char* font_path = luaL_checkstring(L, 1);
    FILE*       file      = fopen           (font_path, "rb");

    if (file != NULL) {
        fseek(file, 0L, SEEK_END);

        long size = ftell(file);

        fseek(file, 0L, SEEK_SET);

        unsigned char* font_buffer = malloc(size);

        fread (font_buffer, size, 1, file);
        fclose(file);

        lua_pushlightuserdata(L, font_buffer);

        return 1;
    } else {
        printf("Error (%s): Failed to open font file: %s.\n", __func__, font_path);

        return 0;
    }
}

static int delete_font(lua_State* L) {
    unsigned char* font = lua_touserdata(L, 1);

    if (font != NULL) free(font);

    return 0;
}

static int create_text(lua_State* L) {
    unsigned char* font = lua_touserdata  (L, 1);
    const char*    word = luaL_checkstring(L, 2);

    stbtt_fontinfo info;

    if (stbtt_InitFont(&info, font, 0)) {
        const int      width     = 512;
        const int      height    = 64;
        const int      length    = 64;
        unsigned char* bitmap    = calloc                   (width * height, sizeof(unsigned char));
        const float    scale     = stbtt_ScaleForPixelHeight(&info, length);
        int            ascent    = 0;
        int            descent   = 0;
        int            line_gap  = 0;
        int            x         = 0;
        int            y         = 0;
        int            i         = 0;

        stbtt_GetFontVMetrics(&info, &ascent, &descent, &line_gap);

        ascent  = roundf(ascent  * scale);
        descent = roundf(descent * scale);

        for (i = 0; i < strlen(word); ++i) {
            int ax   = 0;
            int lsb  = 0;
            int c_x1 = 0;
            int c_y1 = 0;
            int c_x2 = 0;
            int c_y2 = 0;

            stbtt_GetCodepointHMetrics (&info, word[i], &ax, &lsb);
            stbtt_GetCodepointBitmapBox(&info, word[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

            y               = ascent + c_y1;
            int byte_offset = x + roundf(lsb * scale) + (y * width);

            stbtt_MakeCodepointBitmap(&info, bitmap + byte_offset, c_x2 - c_x1, c_y2 - c_y1, width, scale, scale, word[i]);

            int kern  = stbtt_GetCodepointKernAdvance(&info, word[i], word[i + 1]);
            x        += roundf(ax   * scale);
            x        += roundf(kern * scale);
        }

        GLuint* texture = malloc(sizeof(GLuint));

        glGenTextures  (1, texture);
        glBindTexture  (GL_TEXTURE_2D, *texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        unsigned char* inverted_bitmap = malloc(width * height * sizeof(unsigned char));

        int row = 0;
        int col = 0;

        for (row = 0; row < height; ++row) {
            for (col = 0; col < width; ++col) {
                inverted_bitmap[(height - 1 - row) * width + col] = bitmap[row * width + col];
            }
        }

        glTexImage2D         (GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, inverted_bitmap);
        free                 (inverted_bitmap);
        free                 (bitmap);
        lua_pushlightuserdata(L, texture);

        return 1;
    } else {
        printf("Error (%s): Failed to initialize font.\n", __func__);

        return 0;
    }
}

static int load_texture(lua_State* L) {
    const GLchar* texture_path = luaL_checkstring(L, 1);
    GLuint*       texture      = malloc          (sizeof(GLuint));

    if (texture != NULL) {
        glGenTextures  (1, texture);
        glBindTexture  (GL_TEXTURE_2D, *texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_set_flip_vertically_on_load(true);

        GLint    width    = 0;
        GLint    height   = 0;
        GLint    channels = 0;
        GLubyte* pixels   = stbi_load(texture_path, &width, &height, &channels, 0);

        if (pixels != NULL) {
            glTexImage2D    (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            printf("Error (%s): Failed to load texture file: %s.\n.", __func__, texture_path);
        }

        stbi_image_free      (pixels);
        lua_pushlightuserdata(L, texture);

        return 1;
    }

    return 0;
}

static int delete_texture(lua_State* L) {
    GLuint* texture = lua_touserdata(L, 1);

    if (texture != NULL) {
        glDeleteTextures(1, texture);
        free            (texture);
    }

    return 0;
}

static int create_mesh(lua_State* L) {
    Mesh* mesh = malloc(sizeof(Mesh));

    if (mesh != NULL) {
        glGenVertexArrays    (1, &(mesh->VAO));
        glGenBuffers         (1, &(mesh->VBO));
        glGenBuffers         (1, &(mesh->EBO));
        glBindVertexArray    (mesh->VAO);
        setup_VBO            (&(mesh->VBO));
        setup_EBO            (&(mesh->EBO));
        lua_pushlightuserdata(L, mesh);

        return 1;
    }

    return 0;
}

static int delete_mesh(lua_State* L) {
    Mesh* mesh = lua_touserdata(L, 1);

    if (mesh != NULL) {
        glDeleteVertexArrays(1, &(mesh->VAO));
        glDeleteBuffers     (1, &(mesh->VBO));
        glDeleteBuffers     (1, &(mesh->EBO));
        free                (mesh);
    }

    return 0;
}

static int draw(lua_State* L) {
    Mesh*         mesh     = lua_touserdata           (L, 1);
    Window*       window   = lua_touserdata           (L, 2);
    GLuint*       shader   = lua_touserdata           (L, 3);
    GLuint*       texture  = lua_touserdata           (L, 4);
    const GLfloat u        = (GLfloat)luaL_checknumber(L, 5);
    const GLfloat v        = (GLfloat)luaL_checknumber(L, 6);
    const GLfloat du       = (GLfloat)luaL_checknumber(L, 7);
    const GLfloat dv       = (GLfloat)luaL_checknumber(L, 8);

    Vec4 TexCoords = {
        .v = {
            u, v, du, dv
        }
    };

    if (mesh != NULL && window != NULL && shader != NULL && texture != NULL) {
        Mat4 Model;

        identity (&Model);
        scale    (&Model, mesh->scale.v[0], mesh->scale.v[1], 1.0f);
        rotate   (&Model, (M_PI * mesh->angle_of_rotation) / 180.0f);
        translate(&Model, mesh->position.v[0], mesh->position.v[1], mesh->position.v[2]);

        const GLfloat aspect  = (GLfloat)window->width / (GLfloat)window->height;
        const Mat4 Projection = ortho(-aspect, aspect, -1.0f, 1.0f, -10.0f, 10.0f);

        glUseProgram      (*shader);
        glUniformMatrix4fv(glGetUniformLocation(*shader, "Model"),      1, false, (const GLfloat*)&Model);
        glUniformMatrix4fv(glGetUniformLocation(*shader, "Projection"), 1, false, (const GLfloat*)&Projection);
        glUniform4fv      (glGetUniformLocation(*shader, "TexCoords"),  1,        (const GLfloat*)&TexCoords);
        glBindTexture     (GL_TEXTURE_2D, *texture);
        glBindVertexArray (mesh->VAO);
        glDrawElements    (GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*)0);
    }

    return 0;
}

static int set_position(lua_State* L) {
    Mesh*         mesh = lua_touserdata           (L, 1);
    const GLfloat x    = (GLfloat)luaL_checknumber(L, 2);
    const GLfloat y    = (GLfloat)luaL_checknumber(L, 3);
    const GLfloat z    = (GLfloat)luaL_checknumber(L, 4);

    if (mesh != NULL) {
        mesh->position.v[0] = x;
        mesh->position.v[1] = y;
        mesh->position.v[2] = z;
    }

    return 0;
}

static int set_scale(lua_State* L) {
    Mesh*         mesh = lua_touserdata           (L, 1);
    const GLfloat w    = (GLfloat)luaL_checknumber(L, 2);
    const GLfloat h    = (GLfloat)luaL_checknumber(L, 3);

    if (mesh != NULL) {
        mesh->scale.v[0] = w;
        mesh->scale.v[1] = h;
    }

    return 0;
}

static int set_rotate(lua_State* L) {
    Mesh*         mesh  = lua_touserdata           (L, 1);
    const GLfloat angle = (GLfloat)luaL_checknumber(L, 2);

    if (mesh != NULL) mesh->angle_of_rotation = angle;

    return 0;
}

static int create_noise(lua_State* L) {
    const int  seed  = luaL_checkinteger(L, 1);
    fnl_state* noise = malloc           (sizeof(fnl_state));

    if (noise != NULL) {
        *noise = fnlCreateState();

        noise->noise_type = FNL_NOISE_OPENSIMPLEX2;
        noise->seed       = seed;

        lua_pushlightuserdata(L, noise);

        return 1;
    }

    return 0;
}

static int get_noise(lua_State* L) {
    fnl_state*  noise = lua_touserdata   (L, 1);
    const GLint x     = luaL_checkinteger(L, 2);
    const GLint y     = luaL_checkinteger(L, 3);

    if (noise != NULL) {
        lua_pushnumber(L, (float)fnlGetNoise2D(noise, x, y));

        return 1;
    }

    return 0;
}

static int delete_noise(lua_State* L) {
    fnl_state* noise = lua_touserdata(L, 1);

    if (noise != NULL) free(noise);

    return 0;
}

static int engine(lua_State* L) {
    const luaL_Reg functions[] = {
        {"create_window",           create_window},
        {"delete_window",           delete_window},
        {"window_should_close",     window_should_close},
        {"set_window_should_close", set_window_should_close},
        {"clear_color",             clear_color},
        {"swap_buffers",            swap_buffers},
        {"poll_events",             poll_events},
        {"delay",                   delay},
        {"get_key",                 get_key},
        {"create_framebuffer",      create_framebuffer},
        {"delete_framebuffer",      delete_framebuffer},
        {"create_shader",           create_shader},
        {"delete_shader",           delete_shader},
        {"load_font",               load_font},
        {"delete_font",             delete_font},
        {"create_text",             create_text},
        {"load_texture",            load_texture},
        {"delete_texture",          delete_texture},
        {"create_mesh",             create_mesh},
        {"delete_mesh",             delete_mesh},
        {"draw",                    draw},
        {"set_position",            set_position},
        {"set_scale",               set_scale},
        {"set_rotate",              set_rotate},
        {"create_noise",            create_noise},
        {"get_noise",               get_noise},
        {"delete_noise",            delete_noise},

        {NULL, NULL}
    };

    luaL_newlib(L, functions);

    return 1;
}

GLvoid set_window_icon(GLFWwindow* window, const char* icon_path) {
    if (window != NULL) {
        GLFWimage icon_image;

        icon_image.pixels = stbi_load(icon_path, &icon_image.width, &icon_image.height, 0, 4);

        if (icon_image.pixels != NULL) {
            glfwSetWindowIcon(window, 1, &icon_image);
            stbi_image_free  (icon_image.pixels);
        } else {
            printf("Error (%s): Failed to open icon file: %s.\n", __func__, icon_path);
        }
    }
}

char* read_file(const char* file_path) {
    FILE* file      = fopen(file_path, "rb");
    char* file_data = "";

    if(file != NULL) {
        fseek(file, 0L, SEEK_END);

        const long file_size = ftell(file);

        fclose(file);

        file = fopen(file_path, "r");

        file_data = memset(malloc(file_size), '\0', file_size + 1);

        fread (file_data, 1, file_size, file);
        fclose(file);
    } else {
        printf("Error (%s): File not found: %s.\n", __func__, file_path);
    }

    return file_data;
}

GLuint compile_vertex_shader(const GLchar* vertex_path) {
    const char* vertex_shader_source = read_file(vertex_path);

    if (vertex_shader_source != NULL) {
        GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
        GLint success = 0;

        glShaderSource (vertex, 1, &vertex_shader_source, NULL);
        glCompileShader(vertex);
        glGetShaderiv  (vertex, GL_COMPILE_STATUS, &success);

        if (success == 0) {
            GLchar info_log [512];

            glGetShaderInfoLog(vertex, 512, NULL, info_log);
            printf            ("Error (%s): %s\n", __func__, info_log);

            return 0u;
        }

        return vertex;
    }

    return 0u;
}

GLuint compile_fragment_shader(const GLchar* fragment_path) {
    const GLchar* fragment_shader_source = read_file(fragment_path);

    if (fragment_shader_source != NULL) {
        GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
        GLint success   = 0;

        glShaderSource (fragment, 1, &fragment_shader_source, NULL);
        glCompileShader(fragment);
        glGetShaderiv  (fragment, GL_COMPILE_STATUS, &success);

        if (success == 0) {
            GLchar info_log [512];

            glGetShaderInfoLog(fragment, 512, NULL, info_log);
            printf            ("Error (%s): %s\n", __func__, info_log);

            return 0u;
        }

        return fragment;
    }

    return 0u;
}

GLvoid setup_VBO(GLuint* VBO) {
    GLfloat* vertice = malloc(4 * 4 * sizeof(GLfloat));

    if (vertice != NULL) {
        vertice[ 0] = -0.5f;
        vertice[ 1] =  0.5f;
        vertice[ 2] =  0.0f;
        vertice[ 3] =  1.0f;
        vertice[ 4] = -0.5f;
        vertice[ 5] = -0.5f;
        vertice[ 6] =  0.0f;
        vertice[ 7] =  0.0f;
        vertice[ 8] =  0.5f;
        vertice[ 9] = -0.5f;
        vertice[10] =  1.0f;
        vertice[11] =  0.0f;
        vertice[12] =  0.5f;
        vertice[13] =  0.5f;
        vertice[14] =  1.0f;
        vertice[15] =  1.0f;

        glBindBuffer             (GL_ARRAY_BUFFER, *VBO);
        glBufferData             (GL_ARRAY_BUFFER, 4 * 4 * sizeof(GLfloat), vertice, GL_STATIC_DRAW);
        glVertexAttribPointer    (0, 2, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)(0 * sizeof(GLfloat)));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer    (1, 2, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        free                     (vertice);
    }
}

GLvoid setup_EBO(GLuint* EBO) {
    GLuint* indices = malloc(2 * 3 * sizeof(GLuint));

    if (indices != NULL) {
        indices[0] = 0u;
        indices[1] = 1u;
        indices[2] = 3u;
        indices[3] = 1u;
        indices[4] = 2u;
        indices[5] = 3u;

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 3 * sizeof(GLuint), indices, GL_STATIC_DRAW);
        free        (indices);
    }
}

Mat4 ortho(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat z_near, GLfloat z_far) {
    Mat4 Projection;

    Projection.m[0][0] =  2.0f / (right - left);
    Projection.m[0][1] =  0.0f;
    Projection.m[0][2] =  0.0f;
    Projection.m[0][3] = -(right + left) / (right - left);
    Projection.m[1][0] =  0.0f;
    Projection.m[1][1] =  2.0f / (top - bottom);
    Projection.m[1][2] =  0.0f;
    Projection.m[1][3] = -(top + bottom) / (top - bottom);
    Projection.m[2][0] =  0.0f;
    Projection.m[2][1] =  0.0f;
    Projection.m[2][2] = -2.0f / (z_far - z_near);
    Projection.m[2][3] = -(z_far + z_near) / (z_far - z_near);
    Projection.m[3][0] =  0.0f;
    Projection.m[3][1] =  0.0f;
    Projection.m[3][2] =  0.0f;
    Projection.m[3][3] =  1.0f;

    return Projection;
}

GLvoid identity(Mat4* matrix) {
    int i = 0;
    int j = 0;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (i == j) {
                matrix->m[i][j] = 1.0f;
            } else {
                matrix->m[i][j] = 0.0f;
            }
        }
    }
}

GLvoid scale(Mat4* matrix, GLfloat w, GLfloat h, GLfloat l) {
    matrix->m[0][0] = w;
    matrix->m[1][1] = h;
    matrix->m[2][2] = l;
}

GLvoid rotate(Mat4* matrix, GLfloat angle) {
    GLfloat angles[2];

    angles[0] = cos(angle);
    angles[1] = sin(angle);

    GLfloat temp[4];

    temp[0]         = matrix->m[0][0];
    temp[1]         = matrix->m[0][1];
    temp[2]         = matrix->m[0][2];
    temp[3]         = matrix->m[0][3];
    matrix->m[0][0] = angles[0] * temp[0] - angles[1] * temp[1];
    matrix->m[0][1] = angles[1] * temp[0] + angles[0] * temp[1];
    matrix->m[0][2] = temp[2];
    matrix->m[0][3] = temp[3];
    temp[0]         = matrix->m[1][0];
    temp[1]         = matrix->m[1][1];
    temp[2]         = matrix->m[1][2];
    temp[3]         = matrix->m[1][3];
    matrix->m[1][0] = angles[0] * temp[0] - angles[1] * temp[1];
    matrix->m[1][1] = angles[1] * temp[0] + angles[0] * temp[1];
    matrix->m[1][2] = temp[2];
    matrix->m[1][3] = temp[3];
    temp[0]         = matrix->m[2][0];
    temp[1]         = matrix->m[2][1];
    temp[2]         = matrix->m[2][2];
    temp[3]         = matrix->m[2][3];
    matrix->m[2][0] = angles[0] * temp[0] - angles[1] * temp[1];
    matrix->m[2][1] = angles[1] * temp[0] + angles[0] * temp[1];
    matrix->m[2][2] = temp[2];
    matrix->m[2][3] = temp[3];
}

GLvoid translate(Mat4* matrix, GLfloat x, GLfloat y, GLfloat z) {
    matrix->m[3][0] = x;
    matrix->m[3][1] = y;
    matrix->m[3][2] = z;
}
