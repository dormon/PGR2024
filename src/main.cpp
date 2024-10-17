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

  void main(){
  
    uint indices[] = uint[](
      0u,1u,2u,2u,1u,3u,
      4u,5u,6u,6u,5u,7u,
      0u,4u,2u,2u,4u,6u,
      1u,5u,3u,3u,5u,7u,
      0u,1u,4u,4u,1u,5u,
      2u,3u,6u,6u,3u,7u
    );

    if(gl_VertexID>=indices.length())return;

    vec3 pos;
    for(uint i=0u;i<3u;++i)
      pos[i] = float((indices[gl_VertexID]>>i)&1u);

    mat4 model = mat4(1);
    gl_Position = proj*view*model*vec4(pos*2.0f-1.f,1.f);
    vColor = vec3(1,0,0);
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

  GLuint vao;
  glCreateVertexArrays(1,&vao);

  glEnable(GL_DEPTH_TEST);

  auto timer = Timer<float>();

  float cameraYAngle   = 0.f;
  float cameraXAngle   = 0.f;
  float cameraDistance = 2.f;
  float sensitivity    = 0.1f;
  auto cameraPosition = glm::vec3(0.f);


  bool running = true;
  while(running){
    auto CT = glm::translate(glm::mat4(1.f),cameraPosition);
    auto CRX = glm::rotate(glm::mat4(1.f),cameraXAngle,glm::vec3(1.f,0.f,0.f));
    auto CRY = glm::rotate(glm::mat4(1.f),cameraYAngle,glm::vec3(0.f,1.f,0.f));
    auto CR = CRX * CRY;

    auto view = CR * CT;

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

    auto CCR = glm::transpose(CR);
    cameraPosition -= glm::vec3(CCR[2])*(float)keyDown[SDLK_s];
    cameraPosition += glm::vec3(CCR[2])*(float)keyDown[SDLK_w];
    cameraPosition += glm::vec3(CCR[0])*(float)keyDown[SDLK_a];
    cameraPosition -= glm::vec3(CCR[0])*(float)keyDown[SDLK_d];

    glClearColor(0,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);

    float aspect = (float)width / (float)height;
    auto proj = glm::perspective(glm::half_pi<float>(),aspect,0.1f,1000.f);


    prg->use();
    prg->set1f       ("iTime",timer.elapsedFromStart());
    prg->setMatrix4fv("proj" ,(float*)&proj);
    prg->setMatrix4fv("view" ,(float*)&view);

    glDrawArrays(GL_TRIANGLES,0,36);

    SDL_GL_SwapWindow(window);
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  return 0;

}
