#define IMGUI_IMPLEMENTATION
#include "imgui.h"

// Rendering function
void render() {
  static int bgcolor = 0x77;
  static char sometext[80] = "Some text";

  // Refresh the window surface reference each frame in SDL2
  gScreen = SDL_GetWindowSurface(gWindow);

  // clear screen
  drawrect(0, 0, 640, 480, bgcolor);

  imgui_prepare();

  button(GEN_ID, 50, 50);
  button(GEN_ID, 150, 50);

  if(button(GEN_ID, 50, 150))
    bgcolor = (SDL_GetTicks() * 0xc0cac01a) | 0x77;

  if(button(GEN_ID, 150, 150))
    exit(0);

  textfield(GEN_ID, 50, 250, sometext);

  int slidervalue = ((bgcolor >> 16) & 0xff);
  if(slider(GEN_ID, 500, 40, 255, &slidervalue)) {
    bgcolor = (bgcolor & 0x00ffff) | (slidervalue << 16);
  }

  slidervalue = ((bgcolor >> 8) & 0xff);
  if(slider(GEN_ID, 550, 40, 255, &slidervalue)) {
    bgcolor = (bgcolor & 0xff00ff) | (slidervalue << 8);
  }

  slidervalue = bgcolor & 0xff;
  if(slider(GEN_ID, 600, 40, 255, &slidervalue)) {
    bgcolor = (bgcolor & 0xffff00) | slidervalue;
  }

  imgui_finish();

  // update the window surface
  SDL_UpdateWindowSurface(gWindow);

  // don't take all the cpu time
  SDL_Delay(10);
}

// Entry point
int main(int argc, char *argv[]) {
  // Initialize SDL's video subsystem.
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }

  // Create a 640x480 window using SDL2
  gWindow = SDL_CreateWindow("SDL2 IMGUI",
                             SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED,
                             640, 480, SDL_WINDOW_SHOWN);
  if(gWindow == NULL) {
    fprintf(stderr, "Unable to create window: %s\n", SDL_GetError());
    SDL_Quit();
    exit(1);
  }

  gScreen = SDL_GetWindowSurface(gWindow);
  if(gScreen == NULL) {
    fprintf(stderr, "Unable to get window surface: %s\n", SDL_GetError());
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
    exit(1);
  }

  // Enable text input for SDL2 text fields
  SDL_StartTextInput();

  SDL_Surface *temp = SDL_LoadBMP("images/font14x24.bmp");
  if(!temp) {
    fprintf(stderr, "Unable to load font image: %s\n", SDL_GetError());
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
    exit(1);
  }

  gFont = SDL_ConvertSurface(temp, gScreen->format, 0);
  SDL_FreeSurface(temp);

  // Seeing through the black areas in the font
  SDL_SetColorKey(gFont, SDL_TRUE, 0);

  // Main loop
  while(1) {
    render();

    // Poll for events, and handle the ones we care about.
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_MOUSEMOTION:
        uistate.mousex = event.motion.x;
        uistate.mousey = event.motion.y;
        break;
      case SDL_MOUSEBUTTONDOWN:
        if(event.button.button == SDL_BUTTON_LEFT)
          uistate.mousedown = 1;
        break;
      case SDL_MOUSEBUTTONUP:
        if(event.button.button == SDL_BUTTON_LEFT)
          uistate.mousedown = 0;
        break;
      case SDL_KEYDOWN:
        uistate.keyentered = event.key.keysym.sym;
        uistate.keymod = event.key.keysym.mod;
        break;
      case SDL_TEXTINPUT:
        // SDL2 handles text input separately via textinput events
        if(event.text.text[0] != '\0') {
          uistate.keychar = event.text.text[0];
        }
        break;
      case SDL_KEYUP:
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
          SDL_StopTextInput();
          SDL_DestroyWindow(gWindow);
          SDL_Quit();
          return 0;
        default:
          break;
        }
        break;
      case SDL_QUIT:
        SDL_StopTextInput();
        SDL_DestroyWindow(gWindow);
        SDL_Quit();
        return 0;
      default:
        break;
      }
    }
  }

  SDL_StopTextInput();
  SDL_DestroyWindow(gWindow);
  SDL_Quit();
  return 0;
}
