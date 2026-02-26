#pragma once
#include<SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "xaf.h"
SDL_Texture* add_file_tex[3],
* add_folder_tex[3],
* export_folder_tex[3],
* trash_tex[3]
;
SDL_Rect add_file_rect, add_folder_rect, export_folder_rect, trash_rect;

void load_textures(SDL_Renderer* renderer);
#define UINT64 unsigned long long
int last_button_hoovered = -1;

int interface_loop(int* mousepos, int mouse_click, SDL_Renderer* renderer);



size_t file_gui_iter(SDL_Window* wind, SDL_Renderer* renderer,
	nc::arr<nc::xaf_reader2::file_cell>* arr, int* mousepos,
	SDL_Texture* tex_bg, int mouse_click);