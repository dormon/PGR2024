#include<SDL.h>
#include<geGL/geGL.h>
#include<geGL/StaticCalls.h>

using namespace ge::gl;

int main(int argc,char*argv[]){
  auto window = SDL_CreateWindow("PGR2024",0,0,1024,768,SDL_WINDOW_OPENGL);
  auto context = SDL_GL_CreateContext(window);


  ge::gl::init();

  
  char const*vsSrc = R".(
  #version 430

  out vec3 vColor;

  void main(){
    if(gl_VertexID == 0){vColor = vec3(1,0,0);gl_Position = vec4(0,0,0,1);}
    if(gl_VertexID == 1){vColor = vec3(0,1,0);gl_Position = vec4(1,0,0,1);}
    if(gl_VertexID == 2){vColor = vec3(0,0,1);gl_Position = vec4(0,1,0,1);}
  }
  ).";
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs,1,&vsSrc,nullptr);
  glCompileShader(vs);

  char const*fsSrc = R".(
  #version 430

  in vec3 vColor;

  out vec4 fColor;
  void main(){
    fColor = vec4(vColor,1);
  }
  ).";
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs,1,&fsSrc,nullptr);
  glCompileShader(fs);


  GLuint prg = glCreateProgram();
  glAttachShader(prg,vs);
  glAttachShader(prg,fs);
  glLinkProgram(prg);

  GLuint vao;
  glCreateVertexArrays(1,&vao);


  bool running = true;
  while(running){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
      if(event.type == SDL_QUIT)
        running = false;
    }

    glClearColor(0,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao);
    glUseProgram(prg);
    glDrawArrays(GL_TRIANGLES,0,3);

    SDL_GL_SwapWindow(window);
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  return 0;

}
