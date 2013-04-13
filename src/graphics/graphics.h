#ifndef __graphics_h
#define __graphics_h
#include <lcthw/hashmap.h>
#include "../prefix.h"
#include "../math/vrect.h"

#ifdef DABES_IOS
#include <GLKit/GLKMath.h>
#include <OpenGLES/ES2/gl.h>
#include <QuartzCore/CoreAnimation.h>
#else
#include <SDL/SDL_ttf.h>
#endif

int Graphics_init_GL(int swidth, int sheight);

typedef struct GfxSize {
    double w;
    double h;
} GfxSize;

static const GfxSize GfxSizeZero = {0,0};
VRect VRect_fill_size(GfxSize source_size, GfxSize dest_size);

GfxSize load_image_dimensions_from_image(char *filename);

typedef struct GfxTransform3D {
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;
} GfxTransform3D;

GfxTransform3D GfxTransform3D_ortho(float left, float right, float top,
                                    float bottom, float near, float far);
typedef union GfxUVertex {
  struct {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat w;
  } packed;
  float raw[4];
} GfxUVertex;

typedef union GfxUMatrix {
    GfxTransform3D gfx;
    float gl[16];
#ifdef DABES_IOS
    CATransform3D ca;
    GLKMatrix4 glk;
#endif
} GfxUMatrix;

typedef struct GfxTexture {
  GfxSize size;
  GLuint gl_tex;
} GfxTexture;

GfxTexture *GfxTexture_from_data(unsigned char *data, int width, int height);
GfxTexture *GfxTexture_from_image(char *image_name);
#ifdef DABES_IOS
GfxTexture *GfxTexture_from_CGImage(CGImageRef image);
#endif
#ifdef DABES_SDL
GfxTexture *GfxTexture_from_surface(SDL_Surface *surface);
#endif
void GfxTexture_destroy(GfxTexture *texture);

enum {
	UNIFORM_DECAL_PROJECTION_MATRIX,
	UNIFORM_DECAL_MODELVIEW_MATRIX,
  UNIFORM_DECAL_HAS_TEXTURE,
  UNIFORM_DECAL_TEXTURE,
	UNIFORM_TILEMAP_PROJECTION_MATRIX,
	UNIFORM_TILEMAP_MODELVIEW_MATRIX,
	UNIFORM_TILEMAP_TILE_SIZE,
	UNIFORM_TILEMAP_SHEET_ROWS_COLS,
	UNIFORM_TILEMAP_MAP_ROWS_COLS,
  UNIFORM_TILEMAP_ATLAS,
  UNIFORM_TILEMAP_TILESET,
	NUM_UNIFORMS
} UNIFORMS;

enum {
	ATTRIB_DECAL_VERTEX,
	ATTRIB_DECAL_COLOR,
  ATTRIB_DECAL_TEXTURE,
  ATTRIB_TILEMAP_VERTEX,
  ATTRIB_TILEMAP_TEXTURE,
	NUM_ATTRIBUTES
} ATTRIBS;

enum {
  SAMPLER_TILEMAP_ATLAS,
  SAMPLER_TILEMAP_TILESET,
  NUM_SAMPLERS
} SAMPLERS;

extern GLint GfxShader_uniforms[NUM_UNIFORMS];
extern GLint GfxShader_attributes[NUM_ATTRIBUTES];
extern GLint GfxShader_samplers[NUM_SAMPLERS];

typedef struct GfxShader {
  void (*set_up)(struct GfxShader *shader);
  void (*tear_down)(struct GfxShader *shader);
  GLuint gl_program;
} GfxShader;

///////////

typedef struct Graphics {
    Object proto;
    GfxSize screen_size;
    GLuint debug_text_texture;
    GfxShader *current_shader;
#ifndef DABES_IOS
    TTF_Font *debug_text_font;
#endif
    GLuint array_buffer;

    GfxUMatrix projection_matrix;
    GfxUMatrix modelview_matrix;
  
    Hashmap *textures;
    Hashmap *shaders;
} Graphics;

// Rendering
#ifdef DABES_IOS
CGImageRef Graphics_load_CGImage(char *image_name);
#endif
#ifdef DABES_SDL
SDL_Surface *Graphics_load_SDLImage(char *image_name);
#endif

void Graphics_stroke_rect(Graphics *graphics, VRect rect, GLfloat color[4],
        double line_width, double rotation);
void Graphics_draw_rect(Graphics *graphics, VRect rect, GLfloat color[4],
        GfxTexture *texture, VPoint textureOffset, GfxSize textureSize,
        double rotation);
void Graphics_draw_debug_text(Graphics *graphics,
        int ticks_since_last);

// Projection matrix
void Graphics_reset_projection_matrix(Graphics *graphics);
void Graphics_ortho_projection_matrix(Graphics *graphics, double left,
        double right, double top, double bottom, double far, double near);
void Graphics_scale_projection_matrix(Graphics *graphics, double x,
        double y, double z);
void Graphics_rotate_projection_matrix(Graphics *graphics, double rot_degs,
                                       double x, double y, double z);
void Graphics_translate_projection_matrix(Graphics *graphics,
                                         double x, double y, double z);

// Modelview matrix
void Graphics_reset_modelview_matrix(Graphics *graphics);
void Graphics_scale_modelview_matrix(Graphics *graphics,
                                     double x, double y, double z);
void Graphics_rotate_modelview_matrix(Graphics *graphics, double rot_degs,
                                       double x, double y, double z);
void Graphics_translate_modelview_matrix(Graphics *graphics,
                                         double x, double y, double z);

// Shader
GLuint Graphics_load_shader(Graphics *graphics, char *vert_name,
        char *frag_name, GLuint *compiled_program);
GfxShader *Graphics_get_shader(Graphics *graphics, char *name);
void Graphics_use_shader(Graphics *graphics, GfxShader *shader);
int Graphics_init(void *self);
void Graphics_log_shader(GLuint shader);
void Graphics_log_program(GLuint program);

// Textures
GfxTexture *Graphics_texture_from_image(Graphics *graphics, char *image_name);

extern Object GraphicsProto;

#endif
