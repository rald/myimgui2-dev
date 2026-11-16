#ifndef IMGUI_H
#define IMGUI_H

#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

// If you're going to render widgets to the same
// UI from different source files, you can avoid
// ID collisions by defining IMGUI_SRC_ID before
// this define block:
#ifdef IMGUI_SRC_ID
#define GEN_ID ((IMGUI_SRC_ID) + (__LINE__))
#else
#define GEN_ID (__LINE__)
#endif

struct UIState {
  int mousex;
  int mousey;
  int mousedown;

  int hotitem;
  int activeitem;

  int kbditem;
  int keyentered;
  int keymod;
  char keychar;                 // Changed to char for SDL2 text input

  int lastwidget;
};

extern SDL_Window *gWindow;
extern SDL_Surface *gScreen;
extern SDL_Surface *gFont;
extern struct UIState uistate;

void drawchar(char ch, int x, int y);
void drawstring(char *string, int x, int y);
void drawrect(int x, int y, int w, int h, Uint32 color);
int regionhit(int x, int y, int w, int h);
int button(int id, int x, int y);
int slider(int id, int x, int y, int max, int *value);
int textfield(int id, int x, int y, char *buffer);
void imgui_prepare();
void imgui_finish();



#ifdef IMGUI_IMPLEMENTATION

// Window pointer
SDL_Window *gWindow = NULL;

// Screen surface
SDL_Surface *gScreen = NULL;

// Font surface
SDL_Surface *gFont = NULL;

struct UIState uistate = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// Draw a single character.
void drawchar(char ch, int x, int y) {
  SDL_Rect src, dst;
  src.w = 14;
  src.h = 24;
  src.x = 0;
  src.y = (ch - 32) * 24;
  dst.w = 14;
  dst.h = 24;
  dst.x = x;
  dst.y = y;
  SDL_BlitSurface(gFont, &src, gScreen, &dst);
}

// Draw the string. Characters are fixed width.
void drawstring(char *string, int x, int y) {
  while(*string) {
    drawchar(*string, x, y);
    x += 14;
    string++;
  }
}

// Simplified interface to SDL's fillrect call
void drawrect(int x, int y, int w, int h, Uint32 color) {
  SDL_Rect r;
  r.x = x;
  r.y = y;
  r.w = w;
  r.h = h;
  SDL_FillRect(gScreen, &r, color);
}

// Check whether current mouse position is within a rectangle
int regionhit(int x, int y, int w, int h) {
  if(uistate.mousex < x ||
     uistate.mousey < y || uistate.mousex >= x + w || uistate.mousey >= y + h)
    return 0;
  return 1;
}

// Simple button IMGUI widget
int button(int id, int x, int y) {
  if(regionhit(x, y, 64, 48)) {
    uistate.hotitem = id;
    if(uistate.activeitem == 0 && uistate.mousedown)
      uistate.activeitem = id;
  }

  if(uistate.kbditem == 0)
    uistate.kbditem = id;

  if(uistate.kbditem == id)
    drawrect(x - 6, y - 6, 84, 68, 0xff0000);

  drawrect(x + 8, y + 8, 64, 48, 0);
  if(uistate.hotitem == id) {
    if(uistate.activeitem == id) {
      drawrect(x + 2, y + 2, 64, 48, 0xffffff);
    }
    else {
      drawrect(x, y, 64, 48, 0xffffff);
    }
  }
  else {
    drawrect(x, y, 64, 48, 0xaaaaaa);
  }

  if(uistate.kbditem == id) {
    switch (uistate.keyentered) {
    case SDLK_TAB:
      uistate.kbditem = 0;
      if(uistate.keymod & KMOD_SHIFT)
        uistate.kbditem = uistate.lastwidget;
      uistate.keyentered = 0;
      break;
    case SDLK_RETURN:
      return 1;
    }
  }

  uistate.lastwidget = id;

  if(uistate.mousedown == 0 &&
     uistate.hotitem == id && uistate.activeitem == id)
    return 1;

  return 0;
}

// Simple scroll bar IMGUI widget
int slider(int id, int x, int y, int max, int *value) {
  int ypos = ((256 - 16) * (*value)) / max;

  if(regionhit(x + 8, y + 8, 16, 255)) {
    uistate.hotitem = id;
    if(uistate.activeitem == 0 && uistate.mousedown)
      uistate.activeitem = id;
  }

  if(uistate.kbditem == 0)
    uistate.kbditem = id;

  if(uistate.kbditem == id)
    drawrect(x - 4, y - 4, 40, 280, 0xff0000);

  drawrect(x, y, 32, 256 + 16, 0x777777);

  if(uistate.activeitem == id || uistate.hotitem == id) {
    drawrect(x + 8, y + 8 + ypos, 16, 16, 0xffffff);
  }
  else {
    drawrect(x + 8, y + 8 + ypos, 16, 16, 0xaaaaaa);
  }

  if(uistate.kbditem == id) {
    switch (uistate.keyentered) {
    case SDLK_TAB:
      uistate.kbditem = 0;
      if(uistate.keymod & KMOD_SHIFT)
        uistate.kbditem = uistate.lastwidget;
      uistate.keyentered = 0;
      break;
    case SDLK_UP:
      if((*value) > 0) {
        (*value)--;
        return 1;
      }
      break;
    case SDLK_DOWN:
      if((*value) < max) {
        (*value)++;
        return 1;
      }
      break;
    }
  }

  uistate.lastwidget = id;

  if(uistate.activeitem == id) {
    int mousepos = uistate.mousey - (y + 8);
    if(mousepos < 0)
      mousepos = 0;
    if(mousepos > 255)
      mousepos = 255;
    int v = (mousepos * max) / 255;
    if(v != (*value)) {
      (*value) = v;
      return 1;
    }
  }

  return 0;
}

int textfield(int id, int x, int y, char *buffer) {
  int max_visible_chars = 30;   // Number of characters visible in the box at once
  int max_buffer_len = 79;      // Max buffer size to prevent buffer overflow (for 80-char buffers)
  int len = strlen(buffer);
  int changed = 0;

  if(regionhit(x - 4, y - 4, max_visible_chars * 14 + 8, 24 + 8)) {
    uistate.hotitem = id;
    if(uistate.activeitem == 0 && uistate.mousedown)
      uistate.activeitem = id;
  }

  if(uistate.kbditem == 0)
    uistate.kbditem = id;

  if(uistate.kbditem == id)
    drawrect(x - 6, y - 6, max_visible_chars * 14 + 12, 24 + 12, 0xff0000);

  if(uistate.activeitem == id || uistate.hotitem == id) {
    drawrect(x - 4, y - 4, max_visible_chars * 14 + 8, 24 + 8, 0xaaaaaa);
  }
  else {
    drawrect(x - 4, y - 4, max_visible_chars * 14 + 8, 24 + 8, 0x777777);
  }

  // Calculate scroll offset so the view shifts as text exceeds the visible window
  int scroll_offset = 0;
  if(len > max_visible_chars) {
    scroll_offset = len - max_visible_chars;
  }

  // Draw only the visible slice of the string starting from scroll_offset
  drawstring(buffer + scroll_offset, x, y);

  // Position the cursor at the end of the visible text window
  int cursor_pos_x =
    x + ((len < max_visible_chars) ? len : max_visible_chars) * 14;
  if(uistate.kbditem == id && (SDL_GetTicks() >> 8) & 1)
    drawstring("_", cursor_pos_x, y);

  if(uistate.kbditem == id) {
    switch (uistate.keyentered) {
    case SDLK_TAB:
      uistate.kbditem = 0;
      if(uistate.keymod & KMOD_SHIFT)
        uistate.kbditem = uistate.lastwidget;
      uistate.keyentered = 0;
      break;
    case SDLK_BACKSPACE:
      if(len > 0) {
        len--;
        buffer[len] = 0;
        changed = 1;
      }
      break;
    }

    // Allow typing up to max_buffer_len instead of capping at 30
    if(uistate.keychar != 0 && len < max_buffer_len) {
      buffer[len] = uistate.keychar;
      len++;
      buffer[len] = 0;
      changed = 1;
      uistate.keychar = 0;      // Consume character
    }
  }

  if(uistate.mousedown == 0 &&
     uistate.hotitem == id && uistate.activeitem == id)
    uistate.kbditem = id;

  uistate.lastwidget = id;

  return changed;
}

void imgui_prepare() {
  uistate.hotitem = 0;
}

void imgui_finish() {
  if(uistate.mousedown == 0) {
    uistate.activeitem = 0;
  }
  else {
    if(uistate.activeitem == 0)
      uistate.activeitem = -1;
  }
  if(uistate.keyentered == SDLK_TAB)
    uistate.kbditem = 0;
  uistate.keyentered = 0;
  uistate.keychar = 0;
}

#endif

#endif
