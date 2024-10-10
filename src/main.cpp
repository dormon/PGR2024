#include<SDL.h>
#include<geGL/geGL.h>
#include<geGL/StaticCalls.h>

#include<timer.hpp>

using namespace ge::gl;

std::string shaderToStr(GLenum type){
  if(type == GL_VERTEX_SHADER         )return "GL_VERTEX_SHADER"         ;
  if(type == GL_FRAGMENT_SHADER       )return "GL_FRAGMENT_SHADER"       ;
  if(type == GL_GEOMETRY_SHADER       )return "GL_GEOMETRY_SHADER"       ;
  if(type == GL_TESS_CONTROL_SHADER   )return "GL_TESS_CONTROL_SHADER"   ;
  if(type == GL_TESS_EVALUATION_SHADER)return "GL_TESS_EVALUATION_SHADER";
  if(type == GL_COMPUTE_SHADER        )return "GL_COMPUTE_SHADER"        ;
  return "";
}

GLuint createShader(GLenum type,std::string const&src){
  char const*srcs[]={
    src.c_str()
  };
  GLuint s = glCreateShader(type);
  glShaderSource(s,1,srcs,nullptr);
  glCompileShader(s);

  GLint status;
  glGetShaderiv(s,GL_COMPILE_STATUS,&status);
  if(status != GL_TRUE){
    char buffer[1024] = {0};
    glGetShaderInfoLog(s,1024,nullptr,buffer);
    std::cerr << "ERROR: " << shaderToStr(type) << " shader compilation failed" << std::endl;
    std::cerr << std::string(buffer) << std::endl;
  }

  return s;
}

int main(int argc,char*argv[]){
  auto window = SDL_CreateWindow("PGR2024",0,0,1024,768,SDL_WINDOW_OPENGL);
  auto context = SDL_GL_CreateContext(window);


  ge::gl::init();

  
  char const*vsSrc = R".(
  #version 430
  #line 37

  uniform float iTime = 0;
  uniform float cameraYAngle = 0;
  uniform float cameraXAngle = 0;
  uniform float cameraDistance = 2;

  out vec3 vColor;

  mat4 T(float x,float y,float z){
    mat4 r = mat4(1);
    r[3] = vec4(x,y,z,1);
    return r;
  }

  mat4 S(float x,float y,float z){
    mat4 r = mat4(1);
    r[0][0] = x;
    r[1][1] = y;
    r[2][2] = z;
    return r;
  }

  mat4 Rx(float a){
    mat4 r = mat4(1);
    r[1][1] =  cos(a);
    r[2][2] =  cos(a);
    r[1][2] =  sin(a);
    r[2][1] = -sin(a);
    return r;
  }
  mat4 Ry(float a){
    mat4 r = mat4(1);
    r[0][0] =  cos(a);
    r[2][2] =  cos(a);
    r[0][2] =  sin(a);
    r[2][0] = -sin(a);
    return r;
  }
  mat4 Rz(float aa){
    float a = radians(aa);
    mat4 r = mat4(1);
    r[0][0] =  cos(a);
    r[1][1] =  cos(a);
    r[0][1] =  sin(a);
    r[1][0] = -sin(a);
    return r;
  }

  mat4 frustum(float l,float r,float b,float t,float n,float f){
    mat4 R = mat4(1);
    R[0][0] =    2*n/(r-l);
    R[2][0] =  (r+l)/(r-l);
    R[1][1] =    2*n/(t-b);
    R[2][1] =  (t+b)/(t-b);
    R[2][2] = -(f+n)/(f-n);
    R[3][2] = -2*f*n/(f-n);
    R[2][3] =           -1;
    return R;
  }

  mat4 perspective(float fovy,float a,float n,float f){
    float R = n*tan(fovy/2.);
    float L = -R;
    float T = R/a;
    float B = -T;
    return frustum(L,R,B,T,n,f);
  }

  void main(){
    mat4 model = mat4(1);
    float fovy = radians(90);
    float w = 1024;
    float h = 768;
    float aspectRatio = w/h;
    float n = 0.1;
    float f = 1000;
    mat4 proj = perspective(fovy,aspectRatio,n,f);
  
    mat4 view = mat4(1);

    view = T(0,0,-cameraDistance)*Rx(cameraXAngle)*Ry(cameraYAngle);

    mat4 mvp = proj * view * model;
    if(gl_VertexID == 0){vColor = vec3(1,0,0);gl_Position = mvp*vec4(0,0,0,1);}
    if(gl_VertexID == 1){vColor = vec3(0,1,0);gl_Position = mvp*vec4(1,0,0,1);}
    if(gl_VertexID == 2){vColor = vec3(0,0,1);gl_Position = mvp*vec4(0,1,0,1);}
  }
  ).";
  auto vs = createShader(GL_VERTEX_SHADER,vsSrc);

  char const*fsSrc = R".(
  #version 430

  in vec3 vColor;

  out vec4 fColor;
  void main(){
    fColor = vec4(vColor,1);
  }
  ).";

  auto fs = createShader(GL_FRAGMENT_SHADER,fsSrc);


  GLuint prg = glCreateProgram();
  glAttachShader(prg,vs);
  glAttachShader(prg,fs);
  glLinkProgram(prg);

  GLuint vao;
  glCreateVertexArrays(1,&vao);

  glEnable(GL_DEPTH_TEST);


  auto timer = Timer<float>();

  GLuint iTimeLocation          = glGetUniformLocation(prg,"iTime"         );
  GLuint cameraYAngleLocation   = glGetUniformLocation(prg,"cameraYAngle"  );
  GLuint cameraXAngleLocation   = glGetUniformLocation(prg,"cameraXAngle"  );
  GLuint cameraDistanceLocation = glGetUniformLocation(prg,"cameraDistance");

  float cameraYAngle   = 0.f;
  float cameraXAngle   = 0.f;
  float cameraDistance = 2.f;
  float sensitivity    = 0.1f;

  bool running = true;
  while(running){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
      if(event.type == SDL_QUIT)
        running = false;
      if(event.type == SDL_MOUSEMOTION){
        if(event.motion.state & SDL_BUTTON_MMASK){
          cameraYAngle += event.motion.xrel * sensitivity;
          cameraXAngle += event.motion.yrel * sensitivity;
        }
        if(event.motion.state & SDL_BUTTON_RMASK){
          cameraDistance += event.motion.yrel;
        }
      }
    }

    glClearColor(0,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    glUseProgram(prg);

    glProgramUniform1f(prg,iTimeLocation,timer.elapsedFromStart());
    glProgramUniform1f(prg,cameraYAngleLocation  ,cameraYAngle  );
    glProgramUniform1f(prg,cameraXAngleLocation  ,cameraXAngle  );
    glProgramUniform1f(prg,cameraDistanceLocation,cameraDistance);

    glDrawArrays(GL_TRIANGLES,0,3);


    SDL_GL_SwapWindow(window);
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  return 0;

}
