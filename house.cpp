// CS370 Final Project
// Fall 2022
//Theo Bloomquist

#define STB_IMAGE_IMPLEMENTATION
#include "../common/stb_image.h"	// Sean Barrett's image loader - http://nothings.org/
#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "lighting.h"
#define DEG2RAD (M_PI/180.0)

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, Table, Chair, Cylinder, Fan, Plane, Mug, Bowl, Sphere, Frame, NumVAOs};
enum ObjBuffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum Color_Buffer_IDs {RedCube, BlueCube, GreenCube, DecorCube, LightCube, FanCube, GlassWall, NumColorBuffers};
enum LightBuffer_IDs {TableLight, LightBuffer, NumLightBuffers};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {WhiteMaterial, BrownMaterial, GreenMaterial, BlueMaterial, BlackMaterial, Glass, OrangeMaterial};
enum Textures {Art, Carpet, Carpet2, Door, Rope, Snow, Wood, Wood2, DietCoke, Ceramic, MirrorTex, NumTextures};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint ColorBuffers[NumColorBuffers];
GLuint LightBuffers[NumLightBuffers];
GLuint MaterialBuffers[NumMaterialBuffers];
GLuint TextureIDs[NumTextures];

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 4;
GLint normCoords = 3;
GLint texCoords = 2;
GLint colCoords = 4;

// Model files
const char *objFiles[NumVAOs] = {"../models/unitcube.obj", "../models/table.obj", "../models/chair.obj", "../models/cylinder.obj", "../models/fan.obj", "../models/plane.obj", "../models/Mug.obj", "../models/Bowl.obj", "../models/sphere.obj"};


// Texture files
vector<const char *> texFiles = {"../textures/art.png", "../textures/carpet.png", "../textures/carpet2.jpg", "../textures/door.png", "../textures/rope.jpg", "../textures/snow.png", "../textures/wood.jpg", "../textures/wood2.jpg", "../textures/dietcoke.jpg", "../textures/ceramic.jpg"};

// Camera
vec3 eye = {3.0f, 0.0f, 0.0f};
vec3 center = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};
vec3 mirror_eye = {-4.3f, 1.0f, 0.0f};
vec3 mirror_center = {0.0f, 2.0f, 0.0f};
vec3 mirror_up = {0.0f, 1.0f, 0.0f};
GLfloat azimuth = 0.0f;
GLfloat daz = 2.0f;
GLfloat elevation = 90.0f;
GLfloat del = 2.0f;
GLfloat radius = 2.0f;
GLfloat dr = 0.1f;

//animation variables
GLboolean Lighting = true;
GLboolean fanSpin = false;
GLboolean windOpen = false;
GLfloat fan_ang = 0.0f;
GLfloat window_ang = 0.0f;

// Shader variables
// Default (color) shader program references
GLuint default_program;
GLuint default_vPos;
GLuint default_vCol;
GLuint default_proj_mat_loc;
GLuint default_cam_mat_loc;
GLuint default_model_mat_loc;
const char *default_vertex_shader = "../default.vert";
const char *default_frag_shader = "../default.frag";

// Lighting shader program reference
GLuint lighting_program;
GLuint lighting_vPos;
GLuint lighting_vNorm;
GLuint lighting_camera_mat_loc;
GLuint lighting_model_mat_loc;
GLuint lighting_proj_mat_loc;
GLuint lighting_norm_mat_loc;
GLuint lighting_lights_block_idx;
GLuint lighting_materials_block_idx;
GLuint lighting_material_loc;
GLuint lighting_num_lights_loc;
GLuint lighting_light_on_loc;
GLuint lighting_eye_loc;
const char *lighting_vertex_shader = "../lighting.vert";
const char *lighting_frag_shader = "../lighting.frag";

// Texture shader program reference
GLuint texture_program;
GLuint texture_vPos;
GLuint texture_vTex;
GLuint texture_proj_mat_loc;
GLuint texture_camera_mat_loc;
GLuint texture_model_mat_loc;
const char *texture_vertex_shader = "../texture.vert";
const char *texture_frag_shader = "../texture.frag";

// Global state
mat4 proj_matrix;
mat4 camera_matrix;
mat4 normal_matrix;
mat4 model_matrix;

vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint numLights = 0;
GLint lightOn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Global screen dimensions
GLint ww,hh;

void display();
void render_scene();
void build_geometry();
void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer);
void build_materials( );
void build_lights( );
void build_textures();
void load_object(GLuint obj);
void build_frame(GLuint obj);
void draw_color_obj(GLuint obj, GLuint color);
void draw_mat_object(GLuint obj, GLuint material);
void draw_tex_object(GLuint obj, GLuint texture);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);
void build_mirror();
void create_mirror();

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Think Inside The Box");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    // Store initial window size
    glfwGetFramebufferSize(window, &ww, &hh);

    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window,key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);

    // Create geometry buffers
    build_geometry();
    // Create material buffers
    build_materials();
    // Create light buffers
    build_lights();
    // Create textures
    build_textures();

    build_mirror();

    // Load shaders and associate variables
    ShaderInfo default_shaders[] = { {GL_VERTEX_SHADER, default_vertex_shader},{GL_FRAGMENT_SHADER, default_frag_shader},{GL_NONE, NULL} };
    default_program = LoadShaders(default_shaders);
    default_vPos = glGetAttribLocation(default_program, "vPosition");
    default_vCol = glGetAttribLocation(default_program, "vColor");
    default_proj_mat_loc = glGetUniformLocation(default_program, "proj_matrix");
    default_cam_mat_loc = glGetUniformLocation(default_program, "camera_matrix");
    default_model_mat_loc = glGetUniformLocation(default_program, "model_matrix");

    // Load shaders
    // Load light shader
    ShaderInfo lighting_shaders[] = { {GL_VERTEX_SHADER, lighting_vertex_shader},{GL_FRAGMENT_SHADER, lighting_frag_shader},{GL_NONE, NULL} };
    lighting_program = LoadShaders(lighting_shaders);
    lighting_vPos = glGetAttribLocation(lighting_program, "vPosition");
    lighting_vNorm = glGetAttribLocation(lighting_program, "vNormal");
    lighting_proj_mat_loc = glGetUniformLocation(lighting_program, "proj_matrix");
    lighting_camera_mat_loc = glGetUniformLocation(lighting_program, "camera_matrix");
    lighting_norm_mat_loc = glGetUniformLocation(lighting_program, "normal_matrix");
    lighting_model_mat_loc = glGetUniformLocation(lighting_program, "model_matrix");
    lighting_lights_block_idx = glGetUniformBlockIndex(lighting_program, "LightBuffer");
    lighting_materials_block_idx = glGetUniformBlockIndex(lighting_program, "MaterialBuffer");
    lighting_material_loc = glGetUniformLocation(lighting_program, "Material");
    lighting_num_lights_loc = glGetUniformLocation(lighting_program, "NumLights");
    lighting_light_on_loc = glGetUniformLocation(lighting_program, "LightOn");
    lighting_eye_loc = glGetUniformLocation(lighting_program, "EyePosition");

    // Load texture shaders
    ShaderInfo texture_shaders[] = { {GL_VERTEX_SHADER, texture_vertex_shader},{GL_FRAGMENT_SHADER, texture_frag_shader},{GL_NONE, NULL} };
    texture_program = LoadShaders(texture_shaders);
    texture_vPos = glGetAttribLocation(texture_program, "vPosition");
    texture_vTex = glGetAttribLocation(texture_program, "vTexCoord");
    texture_proj_mat_loc = glGetUniformLocation(texture_program, "proj_matrix");
    texture_camera_mat_loc = glGetUniformLocation(texture_program, "camera_matrix");
    texture_model_mat_loc = glGetUniformLocation(texture_program, "model_matrix");

    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Set Initial camera position
//    GLfloat x, y, z;
//    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
//    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
//    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
//    eye = vec3(x, y, z);


    GLfloat x, y, z;
    x = (GLfloat) center[0] - cos(azimuth * DEG2RAD);
    y = (GLfloat) center[1] - cos(elevation * DEG2RAD);
    z = (GLfloat) center[2] - sin(azimuth * DEG2RAD);
    eye = vec3(x, y, z);

    GLdouble elTime = glfwGetTime();
    // Start loop
    while ( !glfwWindowShouldClose( window ) ) {
        create_mirror();
    	// Draw graphics
        display();
        // Update other events like input handling
        glfwPollEvents();
        // Swap buffer onto screen
        GLdouble curTime = glfwGetTime();
        GLdouble diff = curTime - elTime;
        if(fanSpin) {
            fan_ang += (curTime - elTime)*(10.0/60)*360.0f;
        }
        if(windOpen) {
            while(window_ang > -45.0f) {
                window_ang -= (curTime - elTime)*1.0f;
            }
        } else if(!windOpen) {
            while(window_ang < -0.0f) {
                window_ang += (curTime - elTime)*1.0f;
            }
        }
        elTime= curTime;
        glfwSwapBuffers( window );
    }

    // Close window
    glfwTerminate();
    return 0;

}

void display( )
{
    // Declare projection and camera matrices
    proj_matrix = mat4().identity();
    camera_matrix = mat4().identity();

	// Clear window and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute anisotropic scaling
    GLfloat xratio = 1.0f;
    GLfloat yratio = 1.0f;
    // If taller than wide adjust y
    if (ww <= hh)
    {
        yratio = (GLfloat)hh / (GLfloat)ww;
    }
        // If wider than tall adjust x
    else if (hh <= ww)
    {
        xratio = (GLfloat)ww / (GLfloat)hh;
    }

    // DEFAULT ORTHOGRAPHIC PROJECTION
    proj_matrix = frustum(-.1f*xratio, .1f*xratio, -.1f*yratio, .1f*yratio, .2f, 100.0f);

    // Set camera matrix
    camera_matrix = lookat(eye, center, up);

    // Render objects
	render_scene();

	// Flush pipeline
	glFlush();
}

void render_scene( ) {
    // Declare transformation matrices
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();
    mat4 rot_matrix2 = mat4().identity();
    mat4 trans_matrix2 = mat4().identity();
    mat4 scale_matrix2 = mat4().identity();



    // Set cube transformation matrix

//    trans_matrix = translate(0.0f, 0.0f, 0.0f);
//    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
//    scale_matrix = scale(1.0f, 1.5f, 0.7f);
//    model_matrix = trans_matrix*rot_matrix*scale_matrix;
//    // temp table
//    draw_color_obj(Cube, RedCube);

//    trans_matrix = translate(0.0f, 3.0f, 0.0f);
//    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
//    scale_matrix = scale(0.5f, 0.3f, 0.5f);
//    model_matrix = trans_matrix*rot_matrix*scale_matrix;
//    //temp light
//    draw_color_obj(Cube, LightCube);


    if(Lighting) {
        trans_matrix = translate(0.0f, -0.8f, 0.0f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        // temp table
        draw_tex_object(Table, Wood);

        trans_matrix = translate(0.0f, 3.5f, 0.0f);
        rot_matrix = rotate(fan_ang, vec3(0.0f, 1.0f, 0.0f));
        scale_matrix = scale(0.2f, 0.2f, 0.2f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        //temp fan
        draw_mat_object(Fan, WhiteMaterial);

        trans_matrix = translate(0.45f, -0.65f, 0.9f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        scale_matrix = scale(0.2f, 0.2f, 0.2f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        //temp chair
        draw_tex_object(Chair, Wood2);

        trans_matrix = translate(-0.45f, -0.65f, -0.9f);
        rot_matrix = rotate(180.0f, vec3(0.0f, 1.0f, 0.0f));
        scale_matrix = scale(0.2f, 0.2f, 0.2f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        //temp chair
        draw_tex_object(Chair, Wood2);

        trans_matrix = translate(0.45f, -0.65f, -0.9f);
        rot_matrix = rotate(180.0f, vec3(0.0f, 1.0f, 0.0f));
        scale_matrix = scale(0.2f, 0.2f, 0.2f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        //temp chair
        draw_tex_object(Chair, Wood2);

        trans_matrix = translate(-0.45f, -0.65f, 0.9f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        scale_matrix = scale(0.2f, 0.2f, 0.2f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        //temp chair
        draw_tex_object(Chair, Wood2);

        trans_matrix = translate(0.0f, 0.65f, 4.45f);
        rot_matrix = rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
        scale_matrix = scale(2.0f, 0.5f, 2.0f);
        model_matrix = trans_matrix*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_tex_object(Plane, Door);

        trans_matrix = translate(0.0f, 4.7f, 0.0f);
        scale_matrix = scale(11.0f, 0.5f, 11.0f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        model_matrix = trans_matrix*scale_matrix*rot_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_tex_object(Cube, Carpet);

        trans_matrix = translate(-0.0f, 2.0f, -5.4f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        scale_matrix = scale(2.0f, 3.0f, 2.0f);
        model_matrix = trans_matrix*rot_matrix*scale_matrix;
        //temp art
        draw_tex_object(Cube, Art);

        //WINDOW FRAME _______
        scale_matrix = scale(0.05f, 2.0f, 0.05f);
        trans_matrix = translate(4.4f, 0.97f, -1.25f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        model_matrix = rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.05f);
        trans_matrix = translate(4.4f, 0.97f, -1.25f);
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(0.0f, 0.7f, -1.20f);
        model_matrix = trans_matrix2*rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.05f);
        trans_matrix = translate(4.4f, 0.97f, -1.25f);
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(0.0f, -1.25f, -1.20f);
        model_matrix = trans_matrix2*rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.05f);
        trans_matrix = translate(4.4f, 0.97f, -1.25f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(0.0f, 0.0f, 2.0f);
        model_matrix = trans_matrix2*rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.05f);
        trans_matrix = translate(4.4f, 0.97f, -1.25f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(0.0f, 0.0f, 1.0f);
        model_matrix = trans_matrix2*rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);


        scale_matrix = scale(0.05f, 2.0f, 0.05f);
        trans_matrix = translate(4.4f, 0.97f, -1.25f);
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(0.0f, -0.25f, -1.20f);
        model_matrix = trans_matrix2*rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        //WINDOW FRAME_______


        //window panes-->
        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 1.81f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 1.61f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 1.41f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 1.21f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 1.01f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 0.81f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 0.61f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);


        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 0.41f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);


        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 0.21f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 0.01f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.01f, 1.9f, 0.01f);
        trans_matrix = translate(4.3f, 0.97f, -1.25f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(0.0f, 0.0f, 0.5f);
        model_matrix = trans_matrix2*rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_tex_object(Cube, Rope);

        scale_matrix = scale(0.01f, 1.9f, 0.01f);
        trans_matrix = translate(4.3f, 0.97f, -1.25f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(0.0f, 0.0f, 1.5f);
        model_matrix = trans_matrix2*rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_tex_object(Cube, Rope);


        //carpet
        scale_matrix = scale(2.0f, 0.01f, 2.0f);
        trans_matrix = translate(0.0f, -85.0f, 0.0f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        model_matrix = scale_matrix*trans_matrix*rot_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_tex_object(Cube, Carpet2);

        scale_matrix = scale(0.05f, 0.07f, 0.05f);
        trans_matrix = translate(0.0f, -2.0f, 0.0f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        model_matrix = scale_matrix * trans_matrix * rot_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_tex_object(Cylinder, DietCoke);

        scale_matrix = scale(0.2f, 0.2f, 0.2f);
        trans_matrix = translate(-3.0f, -1.2f, -1.0f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        scale_matrix2 = scale(0.80f, 0.80f, 0.80f);
        model_matrix = scale_matrix*trans_matrix*rot_matrix*scale_matrix2;
        normal_matrix = model_matrix.inverse().transpose();
        draw_tex_object(Bowl, Ceramic);

        scale_matrix = scale(0.1f, 0.1f, 0.1f);
        trans_matrix = translate(-5.5f, -1.8f, -2.0f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        scale_matrix2 = scale(0.75f, 0.75f, 0.75f);
        model_matrix = scale_matrix*trans_matrix*rot_matrix*scale_matrix2;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Sphere, OrangeMaterial);

        scale_matrix = scale(0.1f, 0.1f, 0.1f);
        trans_matrix = translate(-6.7f, -0.5f, -2.5f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        scale_matrix2 = scale(0.75f, 0.75f, 0.75f);
        model_matrix = scale_matrix*trans_matrix*rot_matrix*scale_matrix2;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Sphere, OrangeMaterial);

        scale_matrix = scale(0.1f, 0.1f, 0.1f);
        trans_matrix = translate(-5.3f, -0.5f, -2.3f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        scale_matrix2 = scale(0.75f, 0.75f, 0.75f);
        model_matrix = scale_matrix*trans_matrix*rot_matrix*scale_matrix2;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Sphere, OrangeMaterial);

        scale_matrix = scale(0.1f, 0.1f, 0.1f);
        trans_matrix = translate(-6.0f, -0.5f, -1.3f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        scale_matrix2 = scale(0.75f, 0.75f, 0.75f);
        model_matrix = scale_matrix*trans_matrix*rot_matrix*scale_matrix2;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Sphere, OrangeMaterial);


        scale_matrix = scale(0.05f, 0.05f, 0.05f);
        trans_matrix = translate(-5.0f, -3.0f, -1.0f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        scale_matrix2 = scale(0.90f, 0.75f, 0.90f);
        model_matrix = scale_matrix * trans_matrix * rot_matrix * scale_matrix2;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cylinder, BrownMaterial);

    } else {
        trans_matrix = translate(0.0f, -0.8f, 0.0f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
        scale_matrix = scale(0.5f, 0.5f, 0.5f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        // temp table
        draw_mat_object(Table, BlackMaterial);

        trans_matrix = translate(0.0f, 3.5f, 0.0f);
        rot_matrix = rotate(fan_ang, vec3(0.0f, 1.0f, 0.0f));
        scale_matrix = scale(0.2f, 0.2f, 0.2f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        //temp fan
        draw_mat_object(Fan, BlackMaterial);

        trans_matrix = translate(0.45f, -0.65f, 0.9f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        scale_matrix = scale(0.2f, 0.2f, 0.2f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        //temp chair
        draw_mat_object(Chair, BlackMaterial);

        trans_matrix = translate(-0.45f, -0.65f, -0.9f);
        rot_matrix = rotate(180.0f, vec3(0.0f, 1.0f, 0.0f));
        scale_matrix = scale(0.2f, 0.2f, 0.2f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        //temp chair
        draw_mat_object(Chair, BlackMaterial);

        trans_matrix = translate(0.45f, -0.65f, -0.9f);
        rot_matrix = rotate(180.0f, vec3(0.0f, 1.0f, 0.0f));
        scale_matrix = scale(0.2f, 0.2f, 0.2f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        //temp chair
        draw_mat_object(Chair, BlackMaterial);

        trans_matrix = translate(-0.45f, -0.65f, 0.9f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        scale_matrix = scale(0.2f, 0.2f, 0.2f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        //temp chair
        draw_mat_object(Chair, BlackMaterial);

        trans_matrix = translate(0.0f, 0.65f, 4.40f);
        rot_matrix = rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
        scale_matrix = scale(2.0f, 0.5f, 2.0f);
        model_matrix = trans_matrix*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Plane, BlackMaterial);

        trans_matrix = translate(0.0f, 4.7f, 0.0f);
        scale_matrix = scale(11.0f, 0.5f, 11.0f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        model_matrix = trans_matrix*scale_matrix*rot_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, BlackMaterial);

        trans_matrix = translate(-0.0f, 2.0f, -5.4f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        scale_matrix = scale(2.0f, 3.0f, 2.0f);
        model_matrix = trans_matrix*rot_matrix*scale_matrix;
        //temp art
        draw_mat_object(Cube, BlackMaterial);

        //WINDOW FRAME _______
        scale_matrix = scale(0.05f, 2.0f, 0.05f);
        trans_matrix = translate(4.4f, 0.97f, -1.25f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        model_matrix = rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, BlackMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.05f);
        trans_matrix = translate(4.4f, 0.97f, -1.25f);
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(0.0f, 0.7f, -1.20f);
        model_matrix = trans_matrix2*rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, BlackMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.05f);
        trans_matrix = translate(4.4f, 0.97f, -1.25f);
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(0.0f, -1.25f, -1.20f);
        model_matrix = trans_matrix2*rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, BlackMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.05f);
        trans_matrix = translate(4.4f, 0.97f, -1.25f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(0.0f, 0.0f, 2.0f);
        model_matrix = trans_matrix2*rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, BlackMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.05f);
        trans_matrix = translate(4.4f, 0.97f, -1.25f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(0.0f, 0.0f, 1.0f);
        model_matrix = trans_matrix2*rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, BlackMaterial);


        scale_matrix = scale(0.05f, 2.0f, 0.05f);
        trans_matrix = translate(4.4f, 0.97f, -1.25f);
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(0.0f, -0.25f, -1.20f);
        model_matrix = trans_matrix2*rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, BlackMaterial);

        scale_matrix = scale(0.2f, 0.2f, 0.2f);
        trans_matrix = translate(-3.0f, -1.2f, -1.0f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        scale_matrix2 = scale(0.80f, 0.80f, 0.80f);
        model_matrix = scale_matrix*trans_matrix*rot_matrix*scale_matrix2;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Bowl, BlackMaterial);
        //WINDOW FRAME_______


        //window panes-->
        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 1.81f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 1.61f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 1.41f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 1.21f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 1.01f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 0.81f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 0.61f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);


        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 0.41f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);


        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 0.21f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.05f, 2.0f, 0.10f);
        rot_matrix2 = rotate(window_ang, vec3(0.0f, 0.0f, 1.0f));
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(4.3f, 0.01f, -0.25f);
        model_matrix = trans_matrix2*rot_matrix2*rot_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, WhiteMaterial);

        scale_matrix = scale(0.01f, 1.9f, 0.01f);
        trans_matrix = translate(4.3f, 0.97f, -1.25f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(0.0f, 0.0f, 0.5f);
        model_matrix = trans_matrix2*rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, BlackMaterial);

        scale_matrix = scale(0.01f, 1.9f, 0.01f);
        trans_matrix = translate(4.3f, 0.97f, -1.25f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        trans_matrix2 = translate(0.0f, 0.0f, 1.5f);
        model_matrix = trans_matrix2*rot_matrix*trans_matrix*scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cube, BlackMaterial);

        scale_matrix = scale(0.05f, 0.07f, 0.05f);
        trans_matrix = translate(0.0f, -2.0f, 0.0f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        model_matrix = scale_matrix * trans_matrix * rot_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cylinder, BlackMaterial);


        scale_matrix = scale(0.05f, 0.05f, 0.05f);
        trans_matrix = translate(-5.0f, -3.0f, -1.0f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        scale_matrix2 = scale(0.90f, 0.75f, 0.90f);
        model_matrix = scale_matrix * trans_matrix * rot_matrix * scale_matrix2;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Cylinder, BlackMaterial);

        scale_matrix = scale(0.05f, 0.05f, 0.05f);
        trans_matrix = translate(-5.0f, -3.0f, -1.0f);
        rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
        model_matrix = scale_matrix * trans_matrix * rot_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_object(Mug, BlackMaterial);

    }

    trans_matrix = translate(0.0f, -1.0f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(10.0f, 0.3f, 10.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    //temp floor
    draw_mat_object(Cube, WhiteMaterial);


    trans_matrix = translate(2.25f, 1.0f, 1.0f);
    rot_matrix = rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
    scale_matrix = scale(1.0f, 4.0f, 1.0f);
    rot_matrix2 = rotate(90.0f, vec3(0.0f, 1.0f, 0.0f));
    trans_matrix2 = translate(3.4f, 0.0f, 2.0f);
    model_matrix = trans_matrix2*rot_matrix2*trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    //temp window
    draw_tex_object(Plane, Snow);


    // MIRROR
    // Render mirror in scene
    trans_matrix = translate(mirror_eye);
    rot_matrix = rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
    rot_matrix2 = rotate(-90.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(2.0f, 1.0f, 2.0f);
    model_matrix = trans_matrix * rot_matrix * scale_matrix*rot_matrix2;
    draw_tex_object(Plane, MirrorTex);


    trans_matrix = translate(-5.0f, 1.5f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(1.0f, 5.0f, 10.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    //temp wall
    draw_mat_object(Cube, BlueMaterial);

    trans_matrix = translate(5.0f, 1.5f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(1.0f, 5.0f, 10.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    //temp wall
    draw_mat_object(Cube, BlueMaterial);

    trans_matrix = translate(0.0f, 1.5f, -5.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(10.0f, 5.0f, 1.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    //temp wall
    draw_mat_object(Cube, BlueMaterial);

    trans_matrix = translate(0.0f, 1.5f, 5.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(10.0f, 5.0f, 1.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    //temp wall
    draw_mat_object(Cube, BlueMaterial);


    scale_matrix = scale(0.05f, 0.05f, 0.05f);
    trans_matrix = translate(-5.0f, -3.0f, -1.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 0.0f));
    model_matrix = scale_matrix * trans_matrix * rot_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    glDepthMask(GL_FALSE);
    draw_mat_object(Mug, Glass);
    glDepthMask(GL_TRUE);


}

void build_geometry( )
{
    // Generate vertex arrays and buffers
    glGenVertexArrays(NumVAOs, VAOs);

    // Load models
    load_object(Cube);

    load_object(Table);

    load_object(Chair);

    load_object(Cylinder);

    load_object(Fan);

    load_object(Plane);

    load_object(Mug);

    load_object(Bowl);

    load_object(Sphere);

    build_frame(Frame);

    // Generate color buffers
    glGenBuffers(NumColorBuffers, ColorBuffers);

    // Build color buffers
    // Define cube vertex colors (red)
    build_solid_color_buffer(numVertices[Cube], vec4(1.0f, 0.0f, 0.0f, 1.0f), RedCube);
    build_solid_color_buffer(numVertices[Cube], vec4(0.0f, 0.0f, 1.0f, 1.0f), BlueCube);
    build_solid_color_buffer(numVertices[Cube], vec4(0.0f, 1.0f, 0.0f, 1.0f), GreenCube);
    build_solid_color_buffer(numVertices[Cube], vec4(0.3f, 0.3f, 0.5f, 1.0f), DecorCube);
    build_solid_color_buffer(numVertices[Cube], vec4(1.0f, 1.0f, 1.0f, 1.0f), LightCube);
    build_solid_color_buffer(numVertices[Cube], vec4(0.58f, 0.29f, 0.0f, 1.0f), FanCube);

    build_solid_color_buffer(numVertices[Table], vec4(1.0f, 0.0f, 0.0f, 1.0f), RedCube);


    build_solid_color_buffer(numVertices[Chair], vec4(0.0f, 1.0f, 0.0f, 1.0f), GreenCube);


    build_solid_color_buffer(numVertices[Fan], vec4(0.58f, 0.29f, 0.0f, 1.0f), FanCube);


}

void build_materials( ) {
    // Add materials to Materials vector
    MaterialProperties white_material = {
            vec4(0.33f, 0.33, 0.33f, 1.0f),
            vec4(1.00f, 1.00f, 1.00f, 1.0f),
            vec4(0.99f, 0.91f, 0.81f, 1.0f),
            27.8f,
            {0.0f, 0.0f, 0.0f}
    };
    MaterialProperties brown_material = {
            vec4(0.33f, 0.22f, 0.03f, 1.0f), //ambient
            vec4(0.5216, 0.2745, 0.1176, 1.0f), //diffuse
            vec4(0.99f, 0.91f, 0.81f, 1.0f), //specular
            27.8f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties green_material = {
            vec4(0.7f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(0.5f, 0.5f, 0.5f, 1.0f), //specular
            88.8f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    MaterialProperties blue_material = {
            vec4(0.12f, 0.34f, 0.52f, 1.0f),
            vec4(0.23f, 0.44f, 0.91f, 1.0f),
            vec4(0.22f, 0.11f, 0.01f, 1.0f),
            31.4f,
            {0.0f, 0.0f, 0.0f}
    };
    MaterialProperties black_material = {
            vec4(0.00f, 0.00f, 0.00f, 1.0f),
            vec4(0.00f, 0.00f, 0.00f, 1.0f),
            vec4(0.00f, 0.00f, 0.00f, 1.0f),
            05.6f,
            {0.0f, 0.0f, 0.0f}
    };
    MaterialProperties glass = {
            vec4(0.10f, 1.00f, 0.01f, 0.6f),
            vec4(0.9664f, 0.824f, 1.0f, 0.6f),
            vec4(0.40f, 0.50f, 0.10f, 0.6f),
            16.0f,
            {0.0f, 0.0f, 0.0f}
    };
    MaterialProperties orange = {
            vec4(0.33f, 0.44f, 0.55f, 1.0f),
            vec4(1.0f, 0.647, 0.0f, 1.0f),
            vec4(0.40f, 0.50f, 0.10f, 1.0f),
            48.0f,
            {0.0f, 0.0f, 0.0f}
    };

    Materials.push_back(white_material);
    Materials.push_back(brown_material);
    Materials.push_back(green_material);
    Materials.push_back(blue_material);
    Materials.push_back(black_material);
    Materials.push_back(glass);
    Materials.push_back(orange);

    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);
}

void build_lights( ) {
    // Add lights to Lights vector
    LightProperties whitePointLight = {POINT, //type
                                       {0.0f, 0.0f, 0.0f}, //pad
                                       vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
                                       vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
                                       vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
                                       vec4(0.0f, 3.0f, 0.0f, 1.0f),  //position
                                       vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
                                       10.0f,   //cutoff
                                       20.0f,  //exponent
                                       {0.0f, 0.0f}  //pad2
            };

    Lights.push_back(whitePointLight);

    // Set numLights
    numLights = Lights.size();

    // Turn all lights on
    for (int i = 0; i < numLights; i++) {
        lightOn[i] = 1;
    }

    // Create uniform buffer for lights
    glGenBuffers(NumLightBuffers, LightBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBuffers[LightBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Lights.size()*sizeof(LightProperties), Lights.data(), GL_STATIC_DRAW);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // ESC to quit
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }
    if(key == GLFW_KEY_L && action == GLFW_PRESS) {
        lightOn[TableLight] = (lightOn[TableLight] + 1) % 2;
        Lighting  = !Lighting;
    }
    if(key == GLFW_KEY_F && action == GLFW_PRESS) {
        fanSpin = !fanSpin;
    }
    if(key == GLFW_KEY_O && action == GLFW_PRESS) {
        windOpen = !windOpen;
    }
    // Adjust azimuth
    if (key == GLFW_KEY_D) {
        azimuth += daz;
        if (azimuth > 360.0) {
            azimuth -= 360.0;
        }
    } else if (key == GLFW_KEY_A) {
        azimuth -= daz;
        if (azimuth < 0.0)
        {
            azimuth += 360.0;
        }
    }

    // Adjust elevation angle
    if (key == GLFW_KEY_Z)
    {
        elevation += del;
        if (elevation > 179.0)
        {
            elevation = 179.0;
        }
    }
    else if (key == GLFW_KEY_X)
    {
        elevation -= del;
        if (elevation < 1.0)
        {
            elevation = 1.0;
        }
    }

    if(key == GLFW_KEY_W) {
        GLfloat firstX = center[0] - eye[0];
        GLfloat firstZ = center[2] - eye[2];
        eye[0] = eye[0] + firstX * 0.05f;
        eye[2] = eye[2] + firstZ * 0.05f;
    } else if(key == GLFW_KEY_S) {
        GLfloat firstX = center[0] - eye[0];
        GLfloat firstZ = center[2] - eye[2];
        eye[0] = eye[0] - firstX * 0.05f;
        eye[2] = eye[2] - firstZ * 0.05f;
    }

    center[0] = eye[0] + cos(azimuth * DEG2RAD);
    center[1] = eye[1] + cos(elevation * DEG2RAD);
    center[2] = eye[2] + sin(azimuth * DEG2RAD);
    if(eye[0] < -4.0) {
        eye[0] = -4.0;
    }
    else if(eye[0] > 4.0) {
        eye[0] = 4.0;
    }
    if(eye[2] < -4.0) {
        eye[2] = -4.0;
    } else if(eye[2] > 4.0) {
        eye[2] = 4.0;
    }

    // Compute updated camera position
//    GLfloat x, y, z;
//    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
//    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
//    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
//    eye = vec3(x,y,z);

}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}

void build_mirror() {
    // Generate mirror texture
    glGenTextures(1, &TextureIDs[MirrorTex]);
    // Bind mirror texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[MirrorTex]);
    // TODO: Create empty mirror texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ww, hh, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void create_mirror( ) {
    // Clear framebuffer for mirror rendering pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: Set mirror projection matrix
    proj_matrix = frustum(-0.2f, 0.2f, -0.2f, 0.2f, 0.2f, 100.0f);

    // TODO: Set mirror camera matrix
    camera_matrix = lookat(mirror_eye, mirror_center, mirror_up);

    // Render mirror scene (without mirror)
    render_scene();
    glFlush();

    // TODO: Activate texture unit 0
    glActiveTexture(GL_TEXTURE0);
    // TODO: Bind mirror texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[MirrorTex]);
    // TODO: Copy framebuffer into mirror texture
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, ww, hh, 0);
}

void build_frame(GLuint obj) {
    vector<vec4> vertices;
    vector<vec3> normals;

    // Create wireframe for mirror
    vertices = {
            vec4(1.0f, 0.0f, -1.0f, 1.0f),
            vec4(1.0f, 0.0f, 1.0f, 1.0f),
            vec4(-1.0f, 0.0f, 1.0f, 1.0f),
            vec4(-1.0f, 0.0f, -1.0f, 1.0f)
    };

    normals = {
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f)
    };

    numVertices[obj] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}




#include "utilfuncs.cpp"
