#include<SDL.h>

int main(int argc,char*argv[]){
  auto window = SDL_CreateWindow("PGR2024",0,0,1024,768,SDL_WINDOW_OPENGL);
  auto context = SDL_GL_CreateContext(window);


  using GLCLEARCOLOR = void(*)(float,float,float,float);
  using GLCLEAR      = void(*)(uint32_t);
#define GL_COLOR_BUFFER_BIT			0x00004000

  GLCLEARCOLOR glClearColor = (GLCLEARCOLOR)SDL_GL_GetProcAddress("glClearColor");
  GLCLEAR      glClear      = (GLCLEAR     )SDL_GL_GetProcAddress("glClear"     );

  bool running = true;
  while(running){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
      if(event.type == SDL_QUIT)
        running = false;
    }

    glClearColor(0,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(window);
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  return 0;

}
