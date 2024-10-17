#include "geGL/Program.h"
#include "geGL/Shader.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/matrix.hpp"
#include<SDL.h>
#include<geGL/geGL.h>
#include<geGL/StaticCalls.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include<timer.hpp>
#include<bunny.hpp>

using namespace ge::gl;

int main(int argc,char*argv[]){
  int width  = 1024;
  int height = 768 ;

  auto window = SDL_CreateWindow("PGR2024",0,0,width,height,SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  auto context = SDL_GL_CreateContext(window);


  ge::gl::init();

  
  char const*vsSrc = R".(
  #version 430
  #line 37

  uniform float iTime = 0;

  uniform mat4 proj = mat4(1);
  uniform mat4 view = mat4(1);

  out vec3 vColor;

  layout(location=0)in vec3 position;
  layout(location=1)in vec3 normal  ;

  void main(){

    mat4 model = mat4(1);
    gl_Position = proj*view*model*vec4(position,1.f);
    vColor = normal;
  }
  ).";

  char const*fsSrc = R".(
  #version 430

  in vec3 vColor;

  out vec4 fColor;
  void main(){
    fColor = vec4(vColor,1);
  }
  ).";

  auto vs = std::make_shared<Shader>(GL_VERTEX_SHADER  ,vsSrc);
  auto fs = std::make_shared<Shader>(GL_FRAGMENT_SHADER,fsSrc);
  auto prg = std::make_shared<Program>(vs,fs);


  glEnable(GL_DEPTH_TEST);

  auto timer = Timer<float>();

  float cameraYAngle   = 0.f;
  float cameraXAngle   = 0.f;
  float cameraDistance = 2.f;
  float sensitivity    = 0.1f;
  auto cameraPosition = glm::vec3(0.f);

  GLuint ind;
  glCreateBuffers(1,&ind);
  glNamedBufferData(ind,sizeof(bunnyIndices),bunnyIndices,GL_DYNAMIC_COPY);

  GLuint ver;
  glCreateBuffers(1,&ver);
  glNamedBufferData(ver,sizeof(bunnyVertices),bunnyVertices,GL_DYNAMIC_COPY);


  GLuint vao;
  glCreateVertexArrays(1,&vao);

  glVertexArrayElementBuffer(vao,ind);
  glVertexArrayAttribBinding(vao,0,0);
  glEnableVertexArrayAttrib(vao,0);
  glVertexArrayAttribFormat(vao,0,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayVertexBuffer(vao,0,ver,0,sizeof(float)*6);

  glVertexArrayElementBuffer(vao,ind);
  glVertexArrayAttribBinding(vao,1,1);
  glEnableVertexArrayAttrib(vao,1);
  glVertexArrayAttribFormat(vao,1,3,GL_FLOAT,GL_FALSE,0);
  glVertexArrayVertexBuffer(vao,1,ver,3*sizeof(float),sizeof(float)*6);


  bool running = true;
  while(running){
    SDL_Event event;
    std::map<int,int>keyDown;
    while(SDL_PollEvent(&event)){
      if(event.type == SDL_QUIT)
        running = false;
      if(event.type == SDL_MOUSEMOTION){
        if(event.motion.state & SDL_BUTTON_RMASK){
          cameraYAngle += event.motion.xrel * sensitivity;
          cameraXAngle += event.motion.yrel * sensitivity;
        }
        if(event.motion.state & SDL_BUTTON_RMASK){
          cameraDistance += event.motion.yrel;
        }
      }
      if(event.type == SDL_KEYUP){
        keyDown[event.key.keysym.sym] = 0;
      }
      if(event.type == SDL_KEYDOWN){
        keyDown[event.key.keysym.sym] = 1;
      }
      if(event.type == SDL_WINDOWEVENT){
        if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
          width  = event.window.data1;
          height = event.window.data2;
          glViewport(0,0,width,height);
        }
      }
    }

    auto CT = glm::translate(glm::mat4(1.f),cameraPosition);
    auto CRX = glm::rotate(glm::mat4(1.f),cameraXAngle,glm::vec3(1.f,0.f,0.f));
    auto CRY = glm::rotate(glm::mat4(1.f),cameraYAngle,glm::vec3(0.f,1.f,0.f));
    auto CR = CRX * CRY;
    auto view = CR * CT;
    auto CCR = glm::transpose(CR);
    cameraPosition -= glm::vec3(CCR[2])*(float)keyDown[SDLK_s];
    cameraPosition += glm::vec3(CCR[2])*(float)keyDown[SDLK_w];
    cameraPosition += glm::vec3(CCR[0])*(float)keyDown[SDLK_a];
    cameraPosition -= glm::vec3(CCR[0])*(float)keyDown[SDLK_d];

    glClearColor(0,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


    float aspect = (float)width / (float)height;
    auto proj = glm::perspective(glm::half_pi<float>(),aspect,0.1f,1000.f);


    prg->use();
    //prg->set1f       ("iTime",timer.elapsedFromStart());
    prg->setMatrix4fv("proj" ,(float*)&proj);
    prg->setMatrix4fv("view" ,(float*)&view);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES,sizeof(bunnyIndices)/sizeof(uint32_t),GL_UNSIGNED_INT,nullptr);

    SDL_GL_SwapWindow(window);
  }

  SDL_DestroyWindow(window);
  return 0;

}
