#include<SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

SDL_Texture* add_file_tex[3],
* add_folder_tex[3],
* export_file_tex[3],
* trash_tex[3],
* file_icon[3],
* folder_icon[3],
*loading_file_tex;


int file_icon_w=104, file_icon_h=127;
SDL_Rect add_file_rect, add_folder_rect, export_file_rect, trash_rect;

void load_textures(SDL_Renderer*renderer) {
	SDL_Surface* surf_temp = IMG_Load("res/button/add_file_off.png");
	*add_file_tex=SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);



	surf_temp = IMG_Load("res/button/add_file_hoover.png");
	add_file_tex[1] = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);



	surf_temp = IMG_Load("res/button/add_file_on.png");
	add_file_tex[2] = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);

	add_file_rect.x = 237;
	add_file_rect.y = 0;
	add_file_rect.w = 100; //234-135+1
	add_file_rect.h = 80;


	surf_temp = IMG_Load("res/file_icons/load.png");
	loading_file_tex = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);


	surf_temp = IMG_Load("res/button/add_folder_off.png");
	*add_folder_tex = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);



	surf_temp = IMG_Load("res/button/add_folder_hoover.png");
	add_folder_tex[1] = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);



	surf_temp = IMG_Load("res/button/add_folder_on.png");
	add_folder_tex[2] = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);


	add_folder_rect.x = 339;
	add_folder_rect.y = 0;
	add_folder_rect.w = 100; //234-135+1
	add_folder_rect.h = 80;






	surf_temp = IMG_Load("res/button/export_file_off.png");
	*export_file_tex = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);



	surf_temp = IMG_Load("res/button/export_file_hoover.png");
	export_file_tex[1] = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);



	surf_temp = IMG_Load("res/button/export_file_on.png");
	export_file_tex[2] = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);


	export_file_rect.x = 135;
	export_file_rect.y = 0;
	export_file_rect.w = 100; //234-135+1
	export_file_rect.h = 80;








	surf_temp = IMG_Load("res/button/trash_off.png");
	*trash_tex = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);



	surf_temp = IMG_Load("res/button/trash_hoover.png");
	trash_tex[1] = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);



	surf_temp = IMG_Load("res/button/trash_on.png");
	trash_tex[2] = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);


	trash_rect.x = 441;
	trash_rect.y = 0;
	trash_rect.w = 100; //234-135+1
	trash_rect.h = 80;



	surf_temp = IMG_Load("res/file_icons/file.png");
	file_icon[0] = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);


	surf_temp = IMG_Load("res/file_icons/file_hoover.png");
	file_icon[1] = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);


	surf_temp = IMG_Load("res/file_icons/file_selected.png");
	file_icon[2] = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);



	surf_temp = IMG_Load("res/file_icons/folder.png");
	folder_icon[0] = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);


	surf_temp = IMG_Load("res/file_icons/folder_hoover.png");
	folder_icon[1] = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);


	surf_temp = IMG_Load("res/file_icons/folder_selected.png");
	folder_icon[2] = SDL_CreateTextureFromSurface(renderer, surf_temp);
	SDL_FreeSurface(surf_temp);

	/*
	SDL_Rect rect;
	rect.x = (2 * 104) + 81;
	rect.y = (2 * 127) + 80;
	rect.w = 104;
	rect.h = 127;
	SDL_RenderCopy(renderer, file_icon[0], NULL, &rect);

	rect.x = (3 * 104) + 81;
	SDL_RenderCopy(renderer, file_icon[1], NULL, &rect);

	rect.x = (4 * 104) + 81;
	SDL_RenderCopy(renderer, file_icon[2], NULL, &rect);
	*/

}
int last_mousepos[2];
#define UINT64 unsigned long long
int last_button_hoovered = 0;
#include<stdio.h>
int ginterface_refresh=0;


void unhoover_last(SDL_Renderer* renderer,int last_button_hoovered) {

	switch (last_button_hoovered) {
	case 1:
		SDL_RenderCopy(renderer, *export_file_tex, NULL, &export_file_rect);
		break;
	case 2:
		SDL_RenderCopy(renderer, *add_file_tex, NULL, &add_file_rect);
		break;
	case 3:
		SDL_RenderCopy(renderer, *add_folder_tex, NULL, &add_folder_rect);
		break;
	case 4:
		SDL_RenderCopy(renderer, *trash_tex, NULL, &trash_rect);
		break;
	}
}

#include"xaf.h"

int interface_loop(int* mousepos,int mouse_click,SDL_Renderer* renderer) {		//,SDL_Window * window
	int wind_w, wind_h;
	

	
	if ((mousepos[1] < 80)&& (mousepos[0] > 134)) {

		
		if (mousepos[0] < 235) {
			unhoover_last(renderer, last_button_hoovered);
			last_button_hoovered = 1;
			SDL_RenderCopy(renderer, export_file_tex[1 + mouse_click], NULL, &export_file_rect);
			//SDL_Delay(100);
		}
		else if (mousepos[0] < 337) {
			unhoover_last(renderer, last_button_hoovered);
			last_button_hoovered =2;
			SDL_RenderCopy(renderer, add_file_tex[1 + mouse_click], NULL, &add_file_rect);
			//SDL_Delay(100);
		}
		else if (mousepos[0] < 439) {
			unhoover_last(renderer, last_button_hoovered);
			last_button_hoovered = 3;
			SDL_RenderCopy(renderer, add_folder_tex[1 + mouse_click], NULL, &add_folder_rect);
			//SDL_Delay(100);
		}
		else if (mousepos[0] < 541) {
			unhoover_last(renderer, last_button_hoovered);
			last_button_hoovered = 4;
			SDL_RenderCopy(renderer, trash_tex[1 + mouse_click], NULL, &trash_rect);
			//SDL_Delay(100);
		}
		else {
			unhoover_last(renderer, last_button_hoovered);
			last_button_hoovered = 0;
		}
	}
	else {
		unhoover_last(renderer, last_button_hoovered);
		last_button_hoovered = 0;

	}
	ginterface_refresh = 1;
	
	return mouse_click ?last_button_hoovered:-1;
}

int scroll_count=0;

size_t last_file_hoovered =(size_t) -1;
size_t file_selected = (size_t)-1;
SDL_Texture** file_txt = NULL,
			**old_file_txt=NULL;

size_t file_gui_iter(SDL_Window* wind, SDL_Renderer* renderer,
	nc::arr<nc::xaf_reader2::file_cell>* arr,int* mousepos,
	SDL_Texture* tex_bg, int mouse_click){
	int wind_w, wind_h;
	
	SDL_GetWindowSize(wind, &wind_w, &wind_h);

	SDL_Rect rect;


	size_t elem_per_l = (wind_w- 81) / file_icon_w,
		elem_per_col =( wind_h- 80) / file_icon_h;
	size_t posx, posy;
	//std::wstring wstr;
	//TTF_RenderUNICODE_Solid()

	//printf("%zu\n", arr->size);
	//SDL_RenderCopy(renderer, *file_icon, NULL, &rect);
	//puts("huj");
	size_t ret = (size_t)-1;
	int max_x, max_y;
	int pos;
	for (size_t i = 0; i != arr->size; i++) {
		posy = i / elem_per_l;
		posx = i % elem_per_l;
		rect.x = (posx * 104) + 81;
		rect.y = (posy * 127) + 80;
		rect.w = 104;
		rect.h = 127;
		max_x = rect.x + rect.w;
		max_y = rect.y + rect.h;
		pos = (mousepos[0] > rect.x && mousepos[0]< max_x
			&& mousepos[1]> rect.y && mousepos[1] < max_y);
		
		if (posy < elem_per_col) {
			file_txt = arr->data[i].type == nc::XAF_FILE ? file_icon : folder_icon;
			SDL_RenderCopy(renderer, tex_bg, &rect, &rect);
			if ((last_file_hoovered!=i) && (last_file_hoovered != (size_t)-1)) {
				posy = last_file_hoovered / elem_per_l;
				posx = last_file_hoovered % elem_per_l;
				rect.x = (posx * file_icon_w) + 81;
				rect.y = (posy * file_icon_h) + 80;
				rect.w = 104;
				rect.h = 127;
				SDL_RenderCopy(renderer, old_file_txt[0], NULL, &rect);
				last_file_hoovered = i;
				old_file_txt = file_txt;
			}
			else {
				if (pos) {
					if (mouse_click) {
						SDL_RenderCopy(renderer, file_txt[2], NULL, &rect);
						ret = i;
					}
					else {
						SDL_RenderCopy(renderer, file_txt[1], NULL, &rect);
						
					}
				}
				else {
					SDL_RenderCopy(renderer, file_txt[0], NULL, &rect);
				}
			}
			ginterface_refresh = 1;
		}
	}
	if (file_selected !=(size_t)-1) {

		posy = file_selected / elem_per_l;
		posx = file_selected % elem_per_l;
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

		int posx_start = (posx * file_icon_w) + 81,
			posy_start = (posy * file_icon_h) + 80;
		SDL_RenderDrawLine(renderer, posx_start, posy_start, posx_start + file_icon_w - 1, posy_start);



		SDL_RenderDrawLine(renderer, posx_start, posy_start + file_icon_h - 1, posx_start + file_icon_w - 1, posy_start + file_icon_h - 1);


		SDL_RenderDrawLine(renderer, posx_start, posy_start, posx_start, posy_start + file_icon_h - 1);


		SDL_RenderDrawLine(renderer, posx_start + file_icon_w - 1, posy_start, posx_start + file_icon_w - 1, posy_start + file_icon_h - 1);


	}
	return ret;
}






void normal_map(SDL_Surface* texture, SDL_Surface* normal_map,int power) {
	int* tex_pixel =(int*) texture->pixels,
		tex_w=texture->w,tex_h=texture->h,
		*nor_pixel=(int*)normal_map->pixels;
	int normal;
	size_t pos;
	size_t sub = 127* power;
	size_t py, px;
	for (int y = 0; y != tex_h; y++) {
		for (int x = 0; x != tex_w; x++) {
			pos = y * tex_w + x;
			normal = nor_pixel[pos];
			px = x + ((normal & 0x0000FF) >> 16)* power;
			py = y + ((normal & 0x00FF) >> 8)* power;
			px -= sub;
			py -= sub;
			if (px < tex_w && py < tex_h) {
				tex_pixel[pos] = tex_pixel[py * tex_w + px];
			}
		}
	}



}
