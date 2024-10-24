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
#include<iostream>

#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>

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


  out vec3 vColor;

  layout(location=0)in vec3 position;
  layout(location=1)in vec3 normal  ;

  void main(){

    mat4 model = mat4(1);
    gl_Position = model*vec4(position,1.f);
    vColor = normal;
  }
  ).";

  char const*csSrc = R".(
  #version 430

  in vec3 vColor[];
  out vec3 cColor[];

  layout(vertices=3)out;

  void main(){
    if(gl_InvocationID == 0){
      gl_TessLevelInner[0] = 10;
      gl_TessLevelInner[1] = 10;
      gl_TessLevelOuter[0] = 10;
      gl_TessLevelOuter[1] = 10;
      gl_TessLevelOuter[2] = 10;
      gl_TessLevelOuter[3] = 10;
    }
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    cColor[gl_InvocationID] = vColor[gl_InvocationID];
  }
  ).";

  char const*esSrc = R".(
  #version 430

  layout(triangles)in;

  in vec3 cColor[];
  out vec3 eColor;

  uniform mat4 proj = mat4(1);
  uniform mat4 view = mat4(1);

  out vec3 ePosition;

  void main(){
    eColor = 
      cColor[0] * gl_TessCoord.x + 
      cColor[1] * gl_TessCoord.y + 
      cColor[2] * gl_TessCoord.z ;

    
    float f = (1-length(vec3(0.3)-gl_TessCoord.xyz))*gl_TessCoord.x*gl_TessCoord.y*gl_TessCoord.z;
   

    gl_Position = 
      gl_in[0].gl_Position * gl_TessCoord.x + 
      gl_in[1].gl_Position * gl_TessCoord.y + 
      gl_in[2].gl_Position * gl_TessCoord.z ;

    gl_Position.xyz += eColor * f;

    ePosition = gl_Position.xyz;
    gl_Position = proj*view*gl_Position;

  }
  ).";



  char const*gsSrc = R".(
  #version 430
  
  layout(triangles)in;
  layout(line_strip,max_vertices=6)out;

  in vec3 vColor[];
  out vec3 gColor;


  void main(){
    vec4 center = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position)/3;
    vec4 ac = (gl_in[0].gl_Position + gl_in[1].gl_Position)/2.f;
    vec4 bc = (gl_in[1].gl_Position + gl_in[2].gl_Position)/2.f;
    vec4 cc = (gl_in[2].gl_Position + gl_in[0].gl_Position)/2.f;

    gColor = vec3(1);
    gl_Position = center;
    EmitVertex();
    gl_Position = ac;
    EmitVertex();
    EndPrimitive();

    gColor = vec3(1);
    gl_Position = center;
    EmitVertex();
    gl_Position = bc;
    EmitVertex();
    EndPrimitive();

    gColor = vec3(1);
    gl_Position = center;
    EmitVertex();
    gl_Position = cc;
    EmitVertex();
    EndPrimitive();


  }
  ).";

  char const*fsSrc = R".(
  #version 430

  layout(binding=0)uniform sampler2D tex;
  in vec3 eColor;
  in vec3 ePosition;

  out vec4 fColor;
  void main(){
    vec4 texColor = texture(tex,vec2(ePosition.yz));
    fColor = vec4(eColor,1);

    fColor = texColor;
  }
  ).";

  auto vs = std::make_shared<Shader>(GL_VERTEX_SHADER         ,vsSrc);
  auto cs = std::make_shared<Shader>(GL_TESS_CONTROL_SHADER   ,csSrc);
  auto es = std::make_shared<Shader>(GL_TESS_EVALUATION_SHADER,esSrc);
  auto gs = std::make_shared<Shader>(GL_GEOMETRY_SHADER       ,gsSrc);
  auto fs = std::make_shared<Shader>(GL_FRAGMENT_SHADER       ,fsSrc);
  auto prg = std::make_shared<Program>(vs,cs,es,fs);


  auto prg2 = std::make_shared<Program>(
      std::make_shared<Shader>(GL_VERTEX_SHADER,R".(
      #version 430


      out vec2 vCoord;
      void main(){
        if(gl_VertexID == 0)gl_Position = vec4(0,0,0,1);
        if(gl_VertexID == 1)gl_Position = vec4(1,0,0,1);
        if(gl_VertexID == 2)gl_Position = vec4(0,1,0,1);
        if(gl_VertexID == 3)gl_Position = vec4(1,1,0,1);
        vCoord = gl_Position.xy;
      }
      )."),
      std::make_shared<Shader>(GL_FRAGMENT_SHADER,R".(
      #version 430
  
      layout(binding=0)uniform sampler2D tex;

      in vec2 vCoord;
      out vec4 fColor;

      void main(){
        fColor = texture(tex,vCoord);
      }

      )."));


  glEnable(GL_DEPTH_TEST);

  auto timer = Timer<float>();

  float cameraYAngle   = 0.f;
  float cameraXAngle   = 0.f;
  float cameraDistance = 2.f;
  float sensitivity    = 0.01f;
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

  int x,y,n;
  unsigned char *data = stbi_load("../resources/brno.jpg", &x, &y, &n, 0);
  if(data == nullptr)
    std::cerr << "ERROR: loading image failed" << std::endl;

  std::cerr << "channales: " << n << std::endl;

  GLuint tex;
  glCreateTextures(GL_TEXTURE_2D,1,&tex);
  glTextureStorage2D(tex,1,GL_RGB8,x,y);

  glTextureSubImage2D(tex,0,0,0,x,y,GL_RGB,GL_UNSIGNED_BYTE,data);
  glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER,GL_NEAREST);

  stbi_image_free(data);


  std::map<int,int>keyDown;
  bool running = true;
  while(running){
    SDL_Event event;
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
    cameraPosition -= glm::vec3(CCR[2])*(float)keyDown[SDLK_s]*0.01f;
    cameraPosition += glm::vec3(CCR[2])*(float)keyDown[SDLK_w]*0.01f;
    cameraPosition += glm::vec3(CCR[0])*(float)keyDown[SDLK_a]*0.01f;
    cameraPosition -= glm::vec3(CCR[0])*(float)keyDown[SDLK_d]*0.01f;
    cameraPosition -= glm::vec3(CCR[1])*(float)keyDown[SDLK_SPACE]*0.01f;

    glClearColor(0,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


    float aspect = (float)width / (float)height;
    auto proj = glm::perspective(glm::half_pi<float>(),aspect,0.1f,1000.f);


    prg->use();
    //prg->set1f       ("iTime",timer.elapsedFromStart());
    prg->setMatrix4fv("proj" ,(float*)&proj);
    prg->setMatrix4fv("view" ,(float*)&view);

    glBindVertexArray(vao);

    glBindTextureUnit(0,tex);
    //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

    glPatchParameteri(GL_PATCH_VERTICES,3);
    glDrawElements(GL_PATCHES,sizeof(bunnyIndices)/sizeof(uint32_t),GL_UNSIGNED_INT,nullptr);
    if(auto x=glGetError())std::cerr << x << std::endl;

    prg2->use();
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    SDL_GL_SwapWindow(window);
  }

  SDL_DestroyWindow(window);
  return 0;

}
