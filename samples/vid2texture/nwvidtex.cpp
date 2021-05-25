#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/input.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <gst/gst.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "essos.h"

#define DEFAULT_WIDTH (1920)
#define DEFAULT_HEIGHT (1080)
#define WINDOW_BORDER_COLOR (0xFFFFFFFF)
#define WINDOW_BORDER_THICKNESS (4)

#ifndef DRM_FORMAT_R8
#define DRM_FORMAT_R8 (0x20203852)
#endif

#ifndef DRM_FORMAT_GR88
#define DRM_FORMAT_GR88 (0x38385247)
#endif

#ifndef DRM_FORMAT_RG88
#define DRM_FORMAT_RG88 (0x38384752)
#endif

#ifndef DRM_FORMAT_RGBA8888
#define DRM_FORMAT_RGBA8888 (0x34324152)
#endif

typedef struct AppCtx_ AppCtx;

#define MAX_TEXTURES (2)
typedef struct _Surface
{
   int x;
   int y;
   int w;
   int h;
   bool dirty;
   bool haveYUVTextures;
   bool externalImage;
   int frameBufferCapacity;
   int frameWidth;
   int frameHeight;
   unsigned char *frameBuffer;
   int textureCount;
   GLuint textureId[MAX_TEXTURES];
   EGLImageKHR eglImage[MAX_TEXTURES];
   pthread_mutex_t mutex;
} Surface;

typedef struct _GLCtx
{
   AppCtx *appCtx;
   bool initialized;

   PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
   PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
   PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;

   GLuint fragColor;
   GLuint vertColor;
   GLuint progColor;
   GLint locPosColor;
   GLint locColorColor;
   GLint locOffsetColor;
   GLint locMatrixColor;

   bool haveYUVTextures;
   bool haveYUVShaders;
   GLuint fragTex;
   GLuint vertTex;
   GLuint progTex;
   GLint locPosTex;
   GLint locTC;
   GLint locTCUV;
   GLint locResTex;
   GLint locMatrixTex;
   GLint locTexture;
   GLint locTextureUV;

   GLuint fragFill;
   GLuint vertFill;
   GLuint progFill;
   GLint locPosFill;
   GLint locResFill;
   GLint locMatrixFill;
   GLint locColorFill;
} GLCtx;

typedef struct AppCtx_
{
   EssCtx *essCtx;
   int windowWidth;
   int windowHeight;
   int videoX;
   int videoY;
   int videoWidth;
   int videoHeight;
   bool dirty;
   float matrix[16];
   float alpha;
   EGLDisplay eglDisplay;
   bool haveDmaBufImport;
   bool haveExternalImage;
   GLCtx gl;
   Surface surface;
   GLuint shaderVertFill;
   GLuint shaderFragFill;
   GLuint progFill;
   GLuint attrFillVertex;
   GLuint uniFillMatrix;  
   GLuint uniFillTarget;
   GLuint uniFillColor;
   long long frameTimePrev;
   long long frameTime;
   GstElement *pipeline;
   GstElement *playbin;
   GstElement *vidsink;
   GstBus *bus;
   GMainLoop *loop;
   bool quit;
} AppCtx;

static void signalHandler(int signum);
static bool createShaders( AppCtx *ctx,
                           const char *vertSrc, GLuint *vShader,
                           const char *fragSrc, GLuint *fShader );
static bool linkProgram( AppCtx *ctx, GLuint prog );
static bool setupFill( AppCtx *ctx );
static bool setupGL( AppCtx *ctx );
static void fillRect( AppCtx *ctx, int x, int y, int w, int h, unsigned argb );
static void terminated( void * );
static void keyPressed( void *userData, unsigned int key );
static void keyReleased( void *userData, unsigned int );
static void pointerMotion( void *userData, int, int );
static void pointerButtonPressed( void *userData, int button, int x, int y );
static void pointerButtonReleased( void *userData, int, int, int );
static void showUsage();

static bool gRunning;
static int gDisplayWidth;
static int gDisplayHeight;

static long long getMonotonicTimeMicros( void )
{
   int rc;
   struct timespec tm;
   long long timeMicro;
   static bool reportedError= false;
   if ( !reportedError )
   {
      rc= clock_gettime( CLOCK_MONOTONIC, &tm );
   }
   if ( reportedError || rc )
   {
      struct timeval tv;
      if ( !reportedError )
      {
         reportedError= true;
         printf("clock_gettime failed rc %d - using timeofday\n", rc);
      }
      gettimeofday(&tv,0);
      timeMicro= tv.tv_sec*1000000LL+tv.tv_usec;
   }
   else
   {
      timeMicro= tm.tv_sec*1000000LL+(tm.tv_nsec/1000LL);
   }
   return timeMicro;
}

static void signalHandler(int signum)
{
   printf("signalHandler: signum %d\n", signum);
   gRunning= false;
}

static const char *vertFillSrc=
  "uniform mat4 matrix;\n"
  "uniform vec2 targetSize;\n"
  "attribute vec2 vertex;\n"
  "void main()\n"
  "{\n"
  "  vec4 pos= matrix * vec4(vertex, 0, 1);\n"
  "  vec4 scale= vec4( targetSize, 1, 1) * vec4( 0.5, -0.5, 1, 1);\n"
  "  pos= pos / scale;\n"
  "  pos= pos - vec4( 1, -1, 0, 0 );\n"
  "  gl_Position=  pos;\n"
  "}\n";
static const char *fragFillSrc=
  "#ifdef GL_ES\n"
  "precision mediump float;\n"
  "#endif\n"
  "uniform vec4 color;\n"
  "void main()\n"
   "{\n"
   "  gl_FragColor= color;\n"
   "}\n";

static bool createShaders( AppCtx *ctx,
                           const char *vertSrc, GLuint *vShader,
                           const char *fragSrc, GLuint *fShader )
{
   bool result= false;
   int pass;
   GLint status;
   GLuint shader, type, vshader, fshader;
   GLsizei length;
   const char *src, *typeName;
   char log[1000];
   for( pass= 0; pass < 2; ++pass )
   {
      type= (pass == 0) ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
      src= (pass == 0) ? vertSrc : fragSrc;
      typeName= (pass == 0) ? "vertex" : "fragment";
      shader= glCreateShader( type );
      if ( !shader )
      {
         printf("Error: glCreateShader failed for %s shader\n", typeName);
         goto exit;
      }
      glShaderSource( shader, 1, (const char **)&src, NULL );
      glCompileShader( shader );
      glGetShaderiv( shader, GL_COMPILE_STATUS, &status );
      if ( !status )
      {
         glGetShaderInfoLog( shader, sizeof(log), &length, log );
         printf("Error compiling %s shader: %*s\n",
                typeName,
                length,
                log );
      }
      if ( pass == 0 )
         *vShader= shader;
      else
         *fShader= shader;
   }
   result= true;
exit:
   
   return result;
}

static bool linkProgram( AppCtx *ctx, GLuint prog )
{
   bool result= false;
   GLint status;
   glLinkProgram(prog);
   glGetProgramiv(prog, GL_LINK_STATUS, &status);
   if (!status)
   {
      char log[1000];
      GLsizei len;
      glGetProgramInfoLog(prog, 1000, &len, log);
      printf("Error: linking:\n%*s\n", len, log);
      goto exit;
   }
   result= true;
exit:
   return result;
}

static bool setupFill( AppCtx *ctx )
{
   bool result= true;
   if ( !createShaders( ctx, vertFillSrc, &ctx->shaderVertFill, fragFillSrc, &ctx->shaderFragFill ) )
   {
      goto exit;
   }
   ctx->progFill= glCreateProgram();
   glAttachShader( ctx->progFill, ctx->shaderVertFill );
   glAttachShader( ctx->progFill, ctx->shaderFragFill );
   ctx->attrFillVertex= 0;
   glBindAttribLocation(ctx->progFill, ctx->attrFillVertex, "vertex");
   if ( !linkProgram( ctx, ctx->progFill ) )
   {
      goto exit;
   }
   ctx->uniFillMatrix= glGetUniformLocation( ctx->progFill, "matrix" );
   if ( ctx->uniFillTarget == -1 )
   {
      printf("Error: uniform 'natrix' error\n");
      goto exit;
   }
   ctx->uniFillTarget= glGetUniformLocation( ctx->progFill, "targetSize" );
   if ( ctx->uniFillTarget == -1 )
   {
      printf("Error: uniform 'targetSize' error\n");
      goto exit;
   }
   ctx->uniFillColor= glGetUniformLocation( ctx->progFill, "color" );
   if ( ctx->uniFillColor == -1 )
   {
      printf("Error: uniform 'color' error\n");
      goto exit;
   }
exit:
   return result;
}

static bool setupGL( AppCtx *ctx )
{
   bool result= false;
   if ( !setupFill( ctx ) )
   {
      goto exit;
   }
   result= true;
exit:
   return result;
}

static void fillRect( AppCtx *ctx, int x, int y, int w, int h, unsigned argb )
{
   GLfloat verts[4][2]= {
      { (float)x,   (float)y },
      { (float)x+w, (float)y },
      { (float)x,   (float)y+h },
      { (float)x+w, (float)y+h }
   };
   float color[4]=
   {
      ((float)((argb>>16)&0xFF))/255.0f,
      ((float)((argb>>8)&0xFF))/255.0f,
      ((float)((argb)&0xFF))/255.0f,
      ((float)((argb>>24)&0xFF))/255.0f,
   };
   float matrix[4][4]=
   {
      {1, 0, 0, 0},
      {0, 1, 0, 0},
      {0, 0, 1, 0},
      {0, 0, 0, 1}
   };

   glGetError();
   glUseProgram( ctx->progFill );
   glUniformMatrix4fv( ctx->uniFillMatrix, 1, GL_FALSE, (GLfloat*)matrix );
   glUniform2f( ctx->uniFillTarget, ctx->windowWidth, ctx->windowHeight );
   glUniform4fv( ctx->uniFillColor, 1, color );
   glVertexAttribPointer( ctx->attrFillVertex, 2, GL_FLOAT, GL_FALSE, 0, verts );
   glEnableVertexAttribArray( ctx->attrFillVertex );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   glDisableVertexAttribArray( ctx->attrFillVertex );
   int err= glGetError();
   if ( err )
   {
      printf("glError: %d\n", err);
   }
}

static void terminated( void * )
{
   printf("terminated event\n");
   gRunning= false;
}

static EssTerminateListener terminateListener=
{
   terminated
};

static void keyPressed( void *userData, unsigned int key )
{
   AppCtx *ctx= (AppCtx*)userData;
   if ( ctx )
   {
      switch( key )
      {
         default:
            break;
      }
   }
}

static void keyReleased( void *userData, unsigned int key )
{
   AppCtx *ctx= (AppCtx*)userData;
   if ( ctx )
   {
   }
}

static EssKeyListener keyListener=
{
   keyPressed,
   keyReleased
};

static void pointerMotion( void *userData, int, int )
{
   AppCtx *ctx= (AppCtx*)userData;
}

static void pointerButtonPressed( void *userData, int button, int x, int y )
{
   AppCtx *ctx= (AppCtx*)userData;
   if ( ctx )
   {
   }
}

static void pointerButtonReleased( void *userData, int, int, int )
{
   AppCtx *ctx= (AppCtx*)userData;
}

static EssPointerListener pointerListener=
{
   pointerMotion,
   pointerButtonPressed,
   pointerButtonReleased
};

void displaySize( void *userData, int width, int height )
{
   AppCtx *ctx= (AppCtx*)userData;

   if ( (gDisplayWidth != width) || (gDisplayHeight != height) )
   {
      printf("nwvidtex: display size changed: %dx%d\n", width, height);

      gDisplayWidth= width;
      gDisplayHeight= height;

      EssContextResizeWindow( ctx->essCtx, width, height );
      ctx->dirty= true;
   }
}

static EssSettingsListener settingsListener=
{
   displaySize
};

static void showUsage()
{
   printf("usage:\n");
   printf(" nwvidtex [options] <streamURI>\n" );
   printf("  streamURI - input media stream URI\n");
   printf("where [options] are:\n" );
   printf("  --window-size <width>x<height> (eg --window-size 640x480)\n");
   printf("  --video-rect <x>,<y>,<w>,<h> (eg --video-rect 100,100,320,200)\n");
   printf("  -? : show usage\n" );
   printf("\n" );   
}

static const char *vertColor=
   "uniform mat4 u_matrix;\n"
   "uniform vec4 u_offset;\n"
   "attribute vec4 pos;\n"
   "attribute vec4 color;\n"
   "varying vec4 v_color;\n"
   "void main()\n"
   "{\n"
   "  gl_Position= u_matrix * pos + u_offset;\n"
   "  v_color= color;\n"
   "}\n";

static const char *fragColor=
  "#ifdef GL_ES\n"
  "precision mediump float;\n"
  "#endif\n"
  "varying vec4 v_color;\n"
  "void main()\n"
  "{\n"
  "  gl_FragColor= v_color;\n"
  "}\n";

static const char *vertTexture=
  "attribute vec2 pos;\n"
  "attribute vec2 texcoord;\n"
  "uniform mat4 u_matrix;\n"
  "uniform vec2 u_resolution;\n"
  "varying vec2 tx;\n"
  "void main()\n"
  "{\n"
  "  vec4 v1= u_matrix * vec4(pos, 0, 1);\n"
  "  vec4 v2= v1 / vec4(u_resolution, u_resolution.x, 1);\n"
  "  vec4 v3= v2 * vec4(2.0, 2.0, 1, 1);\n"
  "  vec4 v4= v3 - vec4(1.0, 1.0, 0, 0);\n"
  "  v4.w= 1.0+v4.z;\n"
  "  gl_Position=  v4 * vec4(1, -1, 1, 1);\n"
  "  tx= texcoord;\n"
  "}\n";

static const char *fragTexture=
  "#ifdef GL_ES\n"
  "precision mediump float;\n"
  "#endif\n"
  "uniform sampler2D texture;\n"
  "varying vec2 tx;\n"
  "void main()\n"
  "{\n"
  "  gl_FragColor= texture2D(texture, tx);\n"
  "}\n";

static const char *fragTextureExternal=
  "#extension GL_OES_EGL_image_external : require\n"
  "#ifdef GL_ES\n"
  "precision mediump float;\n"
  "#endif\n"
  "uniform samplerExternalOES texture;\n"
  "varying vec2 tx;\n"
  "void main()\n"
  "{\n"
  "  gl_FragColor= texture2D(texture, tx);\n"
  "}\n";

static const char *vertTextureYUV=
  "attribute vec2 pos;\n"
  "attribute vec2 texcoord;\n"
  "attribute vec2 texcoorduv;\n"
  "uniform mat4 u_matrix;\n"
  "uniform vec2 u_resolution;\n"
  "varying vec2 tx;\n"
  "varying vec2 txuv;\n"
  "void main()\n"
  "{\n"
  "  vec4 v1= u_matrix * vec4(pos, 0, 1);\n"
  "  vec4 v2= v1 / vec4(u_resolution, u_resolution.x, 1);\n"
  "  vec4 v3= v2 * vec4(2.0, 2.0, 1, 1);\n"
  "  vec4 v4= v3 - vec4(1.0, 1.0, 0, 0);\n"
  "  v4.w= 1.0+v4.z;\n"
  "  gl_Position=  v4 * vec4(1, -1, 1, 1);\n"
  "  tx= texcoord;\n"
  "  txuv= texcoorduv;\n"
  "}\n";

static const char *fragTextureYUV=
  "#ifdef GL_ES\n"
  "precision mediump float;\n"
  "#endif\n"
  "uniform sampler2D texture;\n"
  "uniform sampler2D textureuv;\n"
  "const vec3 cc_r= vec3(1.0, -0.8604, 1.59580);\n"
  "const vec4 cc_g= vec4(1.0, 0.539815, -0.39173, -0.81290);\n"
  "const vec3 cc_b= vec3(1.0, -1.071, 2.01700);\n"
  "varying vec2 tx;\n"
  "varying vec2 txuv;\n"
  "void main()\n"
  "{\n"
  "   vec4 y_vec= texture2D(texture, tx);\n"
  "   vec4 c_vec= texture2D(textureuv, txuv);\n"
  "   vec4 temp_vec= vec4(y_vec.r, 1.0, c_vec.r, c_vec.g);\n"
  "   gl_FragColor= vec4( dot(cc_r,temp_vec.xyw), dot(cc_g,temp_vec), dot(cc_b,temp_vec.xyz), 1 );\n"
  "}\n";

static bool initGL( GLCtx *ctx )
{
   bool result= false;
   GLint status;
   GLsizei length;
   char infoLog[512];
   const char *fragSrc, *vertSrc;

   ctx->eglCreateImageKHR= (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
   if ( !ctx->eglCreateImageKHR )
   {
      printf("Error: initGL: no eglCreateImageKHR\n");
      goto exit;
   }

   ctx->eglDestroyImageKHR= (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
   if ( !ctx->eglDestroyImageKHR )
   {
      printf("Error: initGL: no eglDestroyImageKHR\n");
      goto exit;
   }

   ctx->glEGLImageTargetTexture2DOES= (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
   if ( !ctx->glEGLImageTargetTexture2DOES )
   {
      printf("Error: initGL: no glEGLImageTargetTexture2DOES\n");
      goto exit;
   }


   ctx->fragColor= glCreateShader( GL_FRAGMENT_SHADER );
   if ( !ctx->fragColor )
   {
      printf("Error: initGL: failed to create color fragment shader\n");
      goto exit;
   }

   glShaderSource( ctx->fragColor, 1, (const char **)&fragColor, NULL );
   glCompileShader( ctx->fragColor );
   glGetShaderiv( ctx->fragColor, GL_COMPILE_STATUS, &status );
   if ( !status )
   {
      glGetShaderInfoLog( ctx->fragColor, sizeof(infoLog), &length, infoLog );
      printf("Error: initGL: compiling color fragment shader: %*s\n", length, infoLog );
      goto exit;
   }

   ctx->vertColor= glCreateShader( GL_VERTEX_SHADER );
   if ( !ctx->vertColor )
   {
      printf("Error: initGL: failed to create color vertex shader\n");
      goto exit;
   }

   glShaderSource( ctx->vertColor, 1, (const char **)&vertColor, NULL );
   glCompileShader( ctx->vertColor );
   glGetShaderiv( ctx->vertColor, GL_COMPILE_STATUS, &status );
   if ( !status )
   {
      glGetShaderInfoLog( ctx->vertColor, sizeof(infoLog), &length, infoLog );
      printf("Error: initGL: compiling color vertex shader: \n%*s\n", length, infoLog );
      goto exit;
   }

   ctx->progColor= glCreateProgram();
   glAttachShader(ctx->progColor, ctx->fragColor);
   glAttachShader(ctx->progColor, ctx->vertColor);

   ctx->locPosColor= 0;
   ctx->locColorColor= 1;
   glBindAttribLocation(ctx->progColor, ctx->locPosColor, "pos");
   glBindAttribLocation(ctx->progColor, ctx->locColorColor, "color");

   glLinkProgram(ctx->progColor);
   glGetProgramiv(ctx->progColor, GL_LINK_STATUS, &status);
   if (!status)
   {
      glGetProgramInfoLog(ctx->progColor, sizeof(infoLog), &length, infoLog);
      printf("Error: initGL: linking:\n%*s\n", length, infoLog);
      goto exit;
   }

   ctx->locOffsetColor= glGetUniformLocation(ctx->progColor, "u_offset");
   ctx->locMatrixColor= glGetUniformLocation(ctx->progColor, "u_matrix");



   if ( ctx->haveYUVShaders )
   {
      fragSrc= fragTextureYUV;
      vertSrc= vertTextureYUV;
   }
   else
   {
      fragSrc= (ctx->appCtx->haveExternalImage ? fragTextureExternal : fragTexture);
      vertSrc= vertTexture;
   }

   ctx->fragTex= glCreateShader( GL_FRAGMENT_SHADER );
   if ( !ctx->fragTex )
   {
      printf("Error: initGL: failed to create texture fragment shader\n");
      goto exit;
   }

   glShaderSource( ctx->fragTex, 1, (const char **)&fragSrc, NULL );
   glCompileShader( ctx->fragTex );
   glGetShaderiv( ctx->fragTex, GL_COMPILE_STATUS, &status );
   if ( !status )
   {
      glGetShaderInfoLog( ctx->fragTex, sizeof(infoLog), &length, infoLog );
      printf("Error: initGL: compiling texture fragment shader: %*s\n", length, infoLog );
      goto exit;
   }

   ctx->vertTex= glCreateShader( GL_VERTEX_SHADER );
   if ( !ctx->vertTex )
   {
      printf("Error: initGL: failed to create texture vertex shader\n");
      goto exit;
   }

   glShaderSource( ctx->vertTex, 1, (const char **)&vertSrc, NULL );
   glCompileShader( ctx->vertTex );
   glGetShaderiv( ctx->vertTex, GL_COMPILE_STATUS, &status );
   if ( !status )
   {
      glGetShaderInfoLog( ctx->vertTex, sizeof(infoLog), &length, infoLog );
      printf("Error: initGL: compiling texture vertex shader: \n%*s\n", length, infoLog );
      goto exit;
   }

   ctx->progTex= glCreateProgram();
   glAttachShader(ctx->progTex, ctx->fragTex);
   glAttachShader(ctx->progTex, ctx->vertTex);

   ctx->locPosTex= 0;
   ctx->locTC= 1;
   glBindAttribLocation(ctx->progTex, ctx->locPosTex, "pos");
   glBindAttribLocation(ctx->progTex, ctx->locTC, "texcoord");
   if ( ctx->haveYUVShaders )
   {
      ctx->locTCUV= 2;
      glBindAttribLocation(ctx->progTex, ctx->locTCUV, "texcoorduv");
   }

   glLinkProgram(ctx->progTex);
   glGetProgramiv(ctx->progTex, GL_LINK_STATUS, &status);
   if (!status)
   {
      glGetProgramInfoLog(ctx->progTex, sizeof(infoLog), &length, infoLog);
      printf("Error: initGL: linking:\n%*s\n", length, infoLog);
      goto exit;
   }

   ctx->locResTex= glGetUniformLocation(ctx->progTex,"u_resolution");
   ctx->locMatrixTex= glGetUniformLocation(ctx->progTex,"u_matrix");
   ctx->locTexture= glGetUniformLocation(ctx->progTex,"texture");
   if ( ctx->haveYUVShaders )
   {
      ctx->locTextureUV= glGetUniformLocation(ctx->progTex,"textureuv");
   }

   result= true;

exit:

   return result;
}

static void termGL( GLCtx *glCtx )
{
   if ( glCtx->fragTex )
   {
      glDeleteShader( glCtx->fragTex );
      glCtx->fragTex= 0;
   }
   if ( glCtx->vertTex )
   {
      glDeleteShader( glCtx->vertTex );
      glCtx->vertTex= 0;
   }
   if ( glCtx->progTex )
   {
      glDeleteProgram( glCtx->progTex );
      glCtx->progTex= 0;
   }
}

static void drawSurface( GLCtx *glCtx, Surface *surface )
{
   AppCtx *appCtx= glCtx->appCtx;
   int x, y, w, h;
   GLenum glerr;

   x= surface->x;
   y= surface->y;
   w= surface->w;
   h= surface->h;
 
   const float verts[4][2]=
   {
      { float(x), float(y) },
      { float(x+w), float(y) },
      { float(x),  float(y+h) },
      { float(x+w), float(y+h) }
   };
 
   const float uv[4][2]=
   {
      { 0,  0 },
      { 1,  0 },
      { 0,  1 },
      { 1,  1 }
   };

   const float identityMatrix[4][4]=
   {
      {1, 0, 0, 0},
      {0, 1, 0, 0},
      {0, 0, 1, 0},
      {0, 0, 0, 1}
   };

   if ( glCtx->haveYUVShaders != surface->haveYUVTextures )
   {
      termGL( glCtx );
      glCtx->haveYUVShaders= surface->haveYUVTextures;
      if ( !initGL( glCtx ) )
      {
         printf("Error: drawSurface: initGL failed while changing shaders\n");
      }
   }

   if ( (surface->textureId[0] == GL_NONE) || surface->dirty || surface->externalImage )
   {
      for( int i= 0; i < surface->textureCount; ++i )
      {
         if ( surface->textureId[i] == GL_NONE )
         {
            glGenTextures(1, &surface->textureId[i] );
         }
       
         glActiveTexture(GL_TEXTURE0+i);
         glBindTexture(GL_TEXTURE_2D, surface->textureId[i] );
         if ( surface->eglImage[i] )
         {
            if ( surface->externalImage )
            {
               glCtx->glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, surface->eglImage[i]);
            }
            else
            {
               glCtx->glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, surface->eglImage[i]);
            }
         }
         else if ( surface->frameBuffer )
         {
            #ifdef GL_BGRA_EXT
            glTexImage2D( GL_TEXTURE_2D, 0, GL_BGRA_EXT, surface->frameWidth, surface->frameHeight,
                          0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, surface->frameBuffer );
            #else
            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, surface->frameWidth, surface->frameHeight,
                          0, GL_RGBA, GL_UNSIGNED_BYTE, surface->frameBuffer );
            #endif
            surface->dirty= false;
                          
         }
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
   }

   glUseProgram(glCtx->progTex);
   glUniform2f(glCtx->locResTex, appCtx->windowWidth, appCtx->windowHeight);
   glUniformMatrix4fv(glCtx->locMatrixTex, 1, GL_FALSE, (GLfloat*)identityMatrix);

   glActiveTexture(GL_TEXTURE0); 
   glBindTexture(GL_TEXTURE_2D, surface->textureId[0]);
   glUniform1i(glCtx->locTexture, 0);
   glVertexAttribPointer(glCtx->locPosTex, 2, GL_FLOAT, GL_FALSE, 0, verts);
   glVertexAttribPointer(glCtx->locTC, 2, GL_FLOAT, GL_FALSE, 0, uv);
   glEnableVertexAttribArray(glCtx->locPosTex);
   glEnableVertexAttribArray(glCtx->locTC);
   if ( surface->haveYUVTextures )
   {
      glActiveTexture(GL_TEXTURE1); 
      glBindTexture(GL_TEXTURE_2D, surface->textureId[1]);
      glUniform1i(glCtx->locTextureUV, 1);
      glVertexAttribPointer(glCtx->locTCUV, 2, GL_FLOAT, GL_FALSE, 0, uv);
      glEnableVertexAttribArray(glCtx->locTCUV);
   }
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
   glDisableVertexAttribArray(glCtx->locPosTex);
   glDisableVertexAttribArray(glCtx->locTC);
   if ( surface->haveYUVTextures )
   {
      glDisableVertexAttribArray(glCtx->locTCUV);
   }

   glerr= glGetError();
   if ( glerr != GL_NO_ERROR )
   {
      printf("Warning: drawSurface: glGetError: %X\n", glerr);
   }
}

static void newVideoTexture( GstElement *elmnt, guint format, guint width, guint height,
                             gint fd0, guint p0l, guint p0s, gpointer p0d,
                             gint fd1, guint p1l, guint p1s, gpointer p1d,
                             gint fd2, guint p2l, guint p2s, gpointer p2d,
                             void *userData )
{
   AppCtx *appCtx= (AppCtx*)userData;
   GLCtx *gl= &appCtx->gl;
   Surface *surface= &appCtx->surface;
   EGLint attr[28];
   pthread_mutex_lock( &surface->mutex );
   if ( fd0 >= 0 )
   {
      #ifdef EGL_LINUX_DMA_BUF_EXT
      for( int i= 0; i < MAX_TEXTURES; ++i )
      {
         if ( surface->eglImage[i] )
         {
            gl->eglDestroyImageKHR( appCtx->eglDisplay, surface->eglImage[i] );
            surface->eglImage[i]= 0;
         }
      }

      #ifdef GL_OES_EGL_image_external
      int i= 0;
      attr[i++]= EGL_WIDTH;
      attr[i++]= width;
      attr[i++]= EGL_HEIGHT;
      attr[i++]= height;
      attr[i++]= EGL_LINUX_DRM_FOURCC_EXT;
      attr[i++]= format;
      attr[i++]= EGL_DMA_BUF_PLANE0_FD_EXT;
      attr[i++]= fd0;
      attr[i++]= EGL_DMA_BUF_PLANE0_OFFSET_EXT;
      attr[i++]= 0;
      attr[i++]= EGL_DMA_BUF_PLANE0_PITCH_EXT;
      attr[i++]= p0s;
      attr[i++]= EGL_DMA_BUF_PLANE1_FD_EXT;
      attr[i++]= fd1;
      attr[i++]= EGL_DMA_BUF_PLANE1_OFFSET_EXT;
      attr[i++]= (fd0 != fd1 ? 0 : width*height);
      attr[i++]= EGL_DMA_BUF_PLANE1_PITCH_EXT;
      attr[i++]= p1s;
      attr[i++]= EGL_YUV_COLOR_SPACE_HINT_EXT;
      attr[i++]= EGL_ITU_REC709_EXT;
      attr[i++]= EGL_SAMPLE_RANGE_HINT_EXT;
      attr[i++]= EGL_YUV_FULL_RANGE_EXT;
      attr[i++]= EGL_NONE;

      surface->eglImage[0]= gl->eglCreateImageKHR( appCtx->eglDisplay,
                                              EGL_NO_CONTEXT,
                                              EGL_LINUX_DMA_BUF_EXT,
                                              (EGLClientBuffer)NULL,
                                              attr );
      if ( surface->eglImage[0] == 0 )
      {
        printf("Error: updateFrame: eglCreateImageKHR failed for fd %d: errno %X\n", fd0, eglGetError());
      }
      if ( surface->textureId[0] != GL_NONE )
      {
         glDeleteTextures( 1, &surface->textureId[0] );
         surface->textureId[0]= GL_NONE;
      }

      surface->textureCount= 1;
      surface->haveYUVTextures= false;
      surface->externalImage= true;
      #else
      attr[0]= EGL_WIDTH;
      attr[1]= width;
      attr[2]= EGL_HEIGHT;
      attr[3]= height;
      attr[4]= EGL_LINUX_DRM_FOURCC_EXT;
      attr[5]= DRM_FORMAT_R8;
      attr[6]= EGL_DMA_BUF_PLANE0_FD_EXT;
      attr[7]= fd0;
      attr[8]= EGL_DMA_BUF_PLANE0_OFFSET_EXT;
      attr[9]= 0;
      attr[10]= EGL_DMA_BUF_PLANE0_PITCH_EXT;
      attr[11]= p0s;
      attr[12]= EGL_NONE;

      surface->eglImage[0]= gl->eglCreateImageKHR( appCtx->eglDisplay,
                                              EGL_NO_CONTEXT,
                                              EGL_LINUX_DMA_BUF_EXT,
                                              (EGLClientBuffer)NULL,
                                              attr );
      if ( surface->eglImage[0] == 0 )
      {
        printf("Error: updateFrame: eglCreateImageKHR failed for fd %d: errno %X\n", fd0, eglGetError());
      }
      if ( surface->textureId[0] != GL_NONE )
      {
         glDeleteTextures( 1, &surface->textureId[0] );
         surface->textureId[0]= GL_NONE;
      }

      attr[0]= EGL_WIDTH;
      attr[1]= width/2;
      attr[2]= EGL_HEIGHT;
      attr[3]= height/2;
      attr[4]= EGL_LINUX_DRM_FOURCC_EXT;
      attr[5]= DRM_FORMAT_GR88;
      attr[6]= EGL_DMA_BUF_PLANE0_FD_EXT;
      attr[7]= fd0;
      attr[8]= EGL_DMA_BUF_PLANE0_OFFSET_EXT;
      attr[9]= width*height;
      attr[10]= EGL_DMA_BUF_PLANE0_PITCH_EXT;
      attr[11]= p1s;
      attr[12]= EGL_NONE;

      surface->eglImage[1]= gl->eglCreateImageKHR( appCtx->eglDisplay,
                                              EGL_NO_CONTEXT,
                                              EGL_LINUX_DMA_BUF_EXT,
                                              (EGLClientBuffer)NULL,
                                              attr );
      if ( surface->eglImage[1] == 0 )
      {
        printf("Error: updateFrame: eglCreateImageKHR failed for fd %d: errno %X\n", fd0, eglGetError());
      }
      if ( surface->textureId[1] != GL_NONE )
      {
         glDeleteTextures( 1, &surface->textureId[1] );
         surface->textureId[1]= GL_NONE;
      }
      surface->textureCount= 2;
      surface->haveYUVTextures= true;
      surface->externalImage= false;
      #endif
      #endif
   }
   else
   {
      if ( format == DRM_FORMAT_RGBA8888 )
      {
         if ( !surface->frameBuffer || 
              (width != surface->frameWidth) ||
              (height != surface->frameHeight) )
         {
            if ( surface->frameBuffer )
            {
               free( surface->frameBuffer );
               surface->frameBuffer= 0;
            }
            surface->frameBuffer= (unsigned char*)malloc( p0l );
            if ( surface->frameBuffer )
            {
               surface->frameBufferCapacity= p0l;
               surface->frameWidth= width;
               surface->frameHeight= height;
            }
         }
         if ( surface->frameBuffer )
         {
            #ifdef GL_BGRA_EXT
            memcpy( surface->frameBuffer, p0d, p0l );
            #else
            unsigned char a, r, g, b;
            unsigned char *s= (unsigned char *)p0d;
            unsigned char *d= surface->frameBuffer;
            for( int i= 0; i < height; ++i )
            {
               for( int j= 0; j < width; ++j )
               {
                  b= *(s++);
                  g= *(s++);
                  r= *(s++);
                  a= *(s++);
                  *(d++)= r;
                  *(d++)= g;
                  *(d++)= b;
                  *(d++)= a;
               }
            }
            #endif
            surface->textureCount= 1;
            surface->haveYUVTextures= false;
            surface->externalImage= false;
            surface->dirty= true;
         }
      }
   }
   appCtx->dirty= true;
   pthread_mutex_unlock( &surface->mutex );
}

static gboolean busCallback(GstBus *bus, GstMessage *message, gpointer data)
{
   AppCtx *ctx= (AppCtx*)data;
   
   switch ( GST_MESSAGE_TYPE(message) ) 
   {
      case GST_MESSAGE_ERROR: 
         {
            GError *error;
            gchar *debug;
            
            gst_message_parse_error(message, &error, &debug);
            g_print("Error: %s\n", error->message);
            if ( debug )
            {
               g_print("Debug info: %s\n", debug);
            }
            g_error_free(error);
            g_free(debug);
            ctx->quit= true;
            g_main_loop_quit( ctx->loop );
         }
         break;
     case GST_MESSAGE_EOS:
         g_print( "EOS ctx %p\n", ctx );
         ctx->quit= true;
         g_main_loop_quit( ctx->loop );
         break;
     default:
         break;
    }
    return TRUE;
}

static bool createPipeline( AppCtx *ctx )
{
   bool result= false;
   int argc= 0;
   char **argv= 0;

   gst_init( &argc, &argv );

   ctx->pipeline= gst_pipeline_new("pipeline");
   if ( !ctx->pipeline )
   {
      printf("Error: unable to create pipeline instance\n" );
      goto exit;
   }

   ctx->bus= gst_pipeline_get_bus( GST_PIPELINE(ctx->pipeline) );
   if ( !ctx->bus )
   {
      printf("Error: unable to get pipeline bus\n");
      goto exit;
   }
   gst_bus_add_watch( ctx->bus, busCallback, ctx );

   ctx->playbin= gst_element_factory_make( "playbin", "playbin" );
   if ( !ctx->playbin )
   {
      printf("Error: unable to create playbin instance\n" );
      goto exit;
   }
   gst_object_ref( ctx->playbin );

   if ( !gst_bin_add( GST_BIN(ctx->pipeline), ctx->playbin) )
   {
      printf("Error: unable to add playbin to pipeline\n");
      goto exit;
   }

   ctx->vidsink= gst_element_factory_make( "westerossink", "vidsink" );
   if ( !ctx->vidsink )
   {
      printf("Error: unable to create westerossink instance\n" );
      goto exit;
   }
   gst_object_ref( ctx->vidsink );

   g_object_set(G_OBJECT(ctx->playbin), "video-sink", ctx->vidsink, NULL );

   result= true;   

exit:
   
   return result;
}

static void destroyPipeline( AppCtx *ctx )
{
   if ( ctx->pipeline )
   {
      gst_element_set_state(ctx->pipeline, GST_STATE_NULL);
   }
   if ( ctx->vidsink )
   {
      gst_object_unref( ctx->vidsink );
      ctx->vidsink= 0;
   }
   if ( ctx->playbin )
   {
      gst_object_unref( ctx->playbin );
      ctx->playbin= 0;
   }
   if ( ctx->bus )
   {
      gst_object_unref( ctx->bus );
      ctx->bus= 0;
   }
   if ( ctx->pipeline )
   {
      gst_object_unref( GST_OBJECT(ctx->pipeline) );
      ctx->pipeline= 0;
   }
}

void render( AppCtx *ctx )
{
   Surface *surface;
   int boundsX, boundsY, boundsW, boundsH;

   glViewport( 0, 0, gDisplayWidth, gDisplayHeight );
   //glClearColor(0.2f, 0.2f, 0.5f, 1.0f);
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   glClear(GL_COLOR_BUFFER_BIT);
   glFrontFace( GL_CW );
   glBlendFuncSeparate( GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE );
   glEnable(GL_BLEND);

   boundsX= 0;
   boundsY= 0;
   boundsW= ctx->windowWidth;
   boundsH= ctx->windowHeight;
   fillRect( ctx, boundsX, boundsY, boundsW, WINDOW_BORDER_THICKNESS, WINDOW_BORDER_COLOR );
   fillRect( ctx, boundsX, boundsY+boundsH-WINDOW_BORDER_THICKNESS, boundsW, WINDOW_BORDER_THICKNESS, WINDOW_BORDER_COLOR );
   fillRect( ctx, boundsX, boundsY, WINDOW_BORDER_THICKNESS, boundsH, WINDOW_BORDER_COLOR );
   fillRect( ctx, boundsX+boundsW-WINDOW_BORDER_THICKNESS, boundsY, WINDOW_BORDER_THICKNESS, boundsH, WINDOW_BORDER_COLOR );

   surface= &ctx->surface;
   pthread_mutex_lock( &surface->mutex );               
   if ( surface->eglImage[0] || surface->frameBuffer )
   {
      drawSurface( &ctx->gl, surface );
   }
   pthread_mutex_unlock( &surface->mutex );               
}

int main( int argc, char **argv )
{
   int nRC= 0;
   AppCtx *ctx= 0;
   int argidx, len;
   bool error= false;
   const char *streamURI= 0;

   printf("nwvidtex v1.0\n");
   ctx= (AppCtx*)calloc( 1, sizeof(AppCtx) );
   if ( !ctx )
   {
      printf("Error: no memory for app context\n");
      goto exit;
   }

   ctx->windowWidth= DEFAULT_WIDTH;
   ctx->windowHeight= DEFAULT_HEIGHT;
   ctx->videoX= 0;
   ctx->videoY= 0;
   ctx->videoWidth= DEFAULT_WIDTH;
   ctx->videoHeight= DEFAULT_HEIGHT;
   ctx->dirty= true;

   ctx->essCtx= EssContextCreate();
   if ( !ctx->essCtx )
   {
      printf("Error: EssContextCreate failed\n");
      goto exit;
   }

   argidx= 1;   
   while ( argidx < argc )
   {
      if ( argv[argidx][0] == '-' )
      {
         len= strlen(argv[argidx]);
         if ( (len == 13) && !strncmp( argv[argidx], "--window-size", len) )
         {
            if ( argidx+1 < argc )
            {
               int w, h;
               ++argidx;
               if ( sscanf( argv[argidx], "%dx%d", &w, &h ) == 2 )
               {
                  if ( (w > 0) && (h > 0) )
                  {
                     ctx->windowWidth= w;
                     ctx->windowHeight= h;
                  }
               }
            }
         }
         else if ( (len == 12) && !strncmp( argv[argidx], "--video-rect", len) )
         {
            ++argidx;
            if ( argidx < argc )
            {
               int x, y, w, h;
               if ( sscanf( argv[argidx], "%d,%d,%d,%d", &x, &y, &w, &h ) == 4 )
               {
                  ctx->videoX= x;
                  ctx->videoY= y;
                  ctx->videoWidth= w;
                  ctx->videoHeight= h;
               }
            }
         }
         else if ( (len == 2) && !strncmp( (const char*)argv[argidx], "-?", len ) )
         {
            showUsage();
         }
         else
         {
            printf("Unrecognized option: (%s)\n", argv[argidx] );
         }
      }
      else
      {
         if ( !streamURI )
         {
            streamURI= argv[argidx];
         }
         else
         {
            printf( "ignoring extra argument: %s\n", argv[argidx] );
         }
      }
      ++argidx;
   }

   if ( !EssContextSetInitialWindowSize( ctx->essCtx, ctx->windowWidth, ctx->windowHeight) )
   {
      error= true;
   }

   if ( !EssContextSetName( ctx->essCtx, "nwvidtex" ) )
   {
      error= true;
   }

   if ( !EssContextSetTerminateListener( ctx->essCtx, ctx, &terminateListener ) )
   {
      error= true;
   }

   if ( !EssContextSetKeyListener( ctx->essCtx, ctx, &keyListener ) )
   {
      error= true;
   }

   if ( !EssContextSetPointerListener( ctx->essCtx, ctx, &pointerListener ) )
   {
      error= true;
   }

   if ( !EssContextSetSettingsListener( ctx->essCtx, ctx, &settingsListener ) )
   {
      error= true;
   }

   if ( !error )
   {
      struct sigaction sigint;

      sigint.sa_handler= signalHandler;
      sigemptyset(&sigint.sa_mask);
      sigint.sa_flags= SA_RESETHAND;
      sigaction(SIGINT, &sigint, NULL);

      if ( !EssContextStart( ctx->essCtx ) )
      {
         error= true;
      }
      else
      if ( !EssContextGetDisplaySize( ctx->essCtx, &gDisplayWidth, &gDisplayHeight ) )
      {
         error= true;
      }
      else
      if ( !setupGL( ctx ) )
      {
         error= true;
      }

      if ( !error )
      {
         char work[128];
         const char *eglExtensions= 0;
         const char *glExtensions= 0;
         Surface *surface;

         ctx->eglDisplay= eglGetCurrentDisplay();
         if ( ctx->eglDisplay == EGL_NO_DISPLAY )
         {
            printf("unable to get EGL display\n");
            goto exit;
         }

         eglExtensions= eglQueryString( ctx->eglDisplay, EGL_EXTENSIONS );
         if ( eglExtensions )
         {
            if ( strstr( eglExtensions, "EGL_EXT_image_dma_buf_import" ) )
            {
               ctx->haveDmaBufImport= true;
            }
         }

         glExtensions= (const char *)glGetString(GL_EXTENSIONS);
         if ( glExtensions )
         {
            #ifdef GL_OES_EGL_image_external
            if ( strstr( glExtensions, "GL_OES_EGL_image_external" ) )
            {
               ctx->haveExternalImage= ctx->haveDmaBufImport;
            }
            #endif
         }

         printf("Have dmabuf import: %d\n", ctx->haveDmaBufImport );
         printf("Have external image: %d\n", ctx->haveExternalImage );

         surface= &ctx->surface;
         memset( surface, 0, sizeof(Surface) );
         surface->x= ctx->videoX;
         surface->y= ctx->videoY;
         surface->w= ctx->videoWidth;
         surface->h= ctx->videoHeight;
         pthread_mutex_init( &surface->mutex, 0 );

         ctx->gl.appCtx= ctx;
         if ( !initGL( &ctx->gl ) )
         {
            printf("Error: failed to setup GL\n");
            goto exit;
         }

         ctx->alpha= 1.0;
         memset( ctx->matrix, 0, sizeof(ctx->matrix) );
         ctx->matrix[0]= ctx->matrix[5]= ctx->matrix[10]= ctx->matrix[15]= 1.0;

         if ( !createPipeline( ctx ) )
         {
            goto exit;
         }

         printf("pipeline created\n");

         g_object_set(G_OBJECT(ctx->playbin), "uri", streamURI, NULL );

         sprintf( work, "%d,%d,%d,%d", ctx->videoX, ctx->videoY, ctx->videoWidth, ctx->videoHeight );
         g_object_set(G_OBJECT(ctx->vidsink), "rectangle", work, NULL );

         g_signal_connect( G_OBJECT(ctx->vidsink), "new-video-texture-callback", G_CALLBACK(newVideoTexture), ctx );

         g_object_set(G_OBJECT(ctx->vidsink), "enable-texture", 1, NULL );

         g_object_set(G_OBJECT(ctx->vidsink), "show-video-window", 0, NULL );

         ctx->loop= g_main_loop_new(NULL,FALSE);
         if ( ctx->loop )
         {
            if ( GST_STATE_CHANGE_FAILURE != gst_element_set_state(ctx->pipeline, GST_STATE_PLAYING) )
            {
               gRunning= true;
               while( gRunning )
               {
                  g_main_context_iteration( NULL, FALSE );

                  EssContextRunEventLoopOnce( ctx->essCtx );

                  if ( ctx->dirty )
                  {
                     ctx->dirty= false;

                     render( ctx );

                     EssContextUpdateDisplay( ctx->essCtx );
                  }
                  if ( ctx->quit )
                  {
                     gRunning= false;
                  }
               }
            }
         }
      }
   }

   if ( error )
   {
      const char *detail= EssContextGetLastErrorDetail( ctx->essCtx );
      printf("Essos error: (%s)\n", detail );
   }

exit:

   if ( ctx )
   {
      Surface *surface;

      destroyPipeline( ctx );

      if ( ctx->loop )
      {
         g_main_loop_unref(ctx->loop);
         ctx->loop= 0;
      }

      surface= &ctx->surface;
      for( int i= 0; i < MAX_TEXTURES; ++i )
      {
         if ( surface->eglImage[i] )
         {
            ctx->gl.eglDestroyImageKHR( ctx->eglDisplay, surface->eglImage[i] );
            surface->eglImage[i]= 0;
         }
      }

      termGL( &ctx->gl );

      if ( ctx->essCtx )
      {
         EssContextDestroy( ctx->essCtx );
      }

      pthread_mutex_destroy( &ctx->surface.mutex );

      free( ctx );
   }

   return nRC;
}

