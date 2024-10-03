#include<SDL.h>

int main(int argc,char*argv[]){
  auto window = SDL_CreateWindow("PGR2024",0,0,1024,768,SDL_WINDOW_OPENGL);
  auto context = SDL_GL_CreateContext(window);

  bool running = true;
  while(running){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
      if(event.type == SDL_QUIT)
        running = false;
    }

    SDL_GL_SwapWindow(window);
  }

  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  return 0;

}
