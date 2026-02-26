#include<windows.h>
#include <direct.h>
//#define NC_DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "xaf.h"
#include"ginterface.h"
#define FILESIZE 100
SDL_Event test_event;
int mouse_pos[2];
extern size_t file_selected;
extern int ginterface_refresh;
UINT64 event_handler() {
    UINT64 ret = 0;
    SDL_PollEvent(&test_event);

    SDL_GetMouseState(mouse_pos, mouse_pos + 1);
    switch (test_event.type) {
    case SDL_QUIT:
        exit(EXIT_SUCCESS);
    case SDL_MOUSEBUTTONDOWN:
        return 2;
    case SDL_MOUSEBUTTONUP:
        break;
    case SDL_KEYDOWN:
        ret = test_event.key.keysym.sym;
        ret <<= 8;
        ret |= (3 + (test_event.key.type == SDL_KEYDOWN));
        return ret;
    }
    return 1;
}
using namespace nc;
#include<windows.h>

extern SDL_Texture* loading_file_tex;

extern int file_icon_w, file_icon_h ;



int SDL_main(int argc, char** argv){
     SDL_Init(SDL_INIT_VIDEO);
     int flags = IMG_INIT_JPG | IMG_INIT_PNG;
     if (IMG_Init(flags) != flags) {
         puts("IMG_Init()");
         puts(IMG_GetError());
         exit(-555);
     }
     
     argc--;
     argv++;
     xaf_reader2* arc_arr = (xaf_reader2*)mecalloc(sizeof(xaf_reader2) * argc);






     /*
     xaf_22 temp,
         *temp2=(xaf_22*)memalloc(sizeof(xaf_22));

     xaf_creator xc = temp,*xc2=temp2;
*/




     for (int i = 0; i != argc; i++) {
         arc_arr[i] = nc::xaf_reader2();
         arc_arr[i].read_xaf(argv[i]);
     }
        


     HWND h_console = GetConsoleWindow();
     //ShowWindow(h_console, SW_HIDE);
     //SDL_WINDOW_RESIZABLE
     SDL_Window* wind = SDL_CreateWindow("NXAF", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, 0);
     //system("pause");
     SDL_Renderer* renderer = SDL_CreateRenderer(wind, 0, SDL_RENDERER_ACCELERATED|  SDL_RENDERER_PRESENTVSYNC);
     SDL_Surface* temp_surf = IMG_Load("res/bg.png");
     SDL_Texture* tex_bg = SDL_CreateTextureFromSurface(renderer,temp_surf);
     SDL_FreeSurface(temp_surf);
     SDL_RenderCopy(renderer, tex_bg, NULL, NULL);
     SDL_RenderPresent(renderer);
    xaf arc;

    int ret ;
    //scanf("%i", &ret);
    /*
    file f;
    char* data=(char*) memalloc(FILESIZE);
    memset(data, 1, FILESIZE);
    f.build(data, FILESIZE, L"fileA", 5);
    memfree(data);
    arc.add(&f);
    f.destroy();
   

    data = (char*)memalloc(FILESIZE);
    memset(data, 2, FILESIZE);
    f.build(data, FILESIZE, L"fileB", 5);
    memfree(data);
    arc.add(&f);
    f.destroy();
 

    data = (char*)memalloc(FILESIZE);
    memset(data, 3, FILESIZE);
    f.build(data, FILESIZE, L"fileC", 5);
    memfree(data);
    arc.add(&f);
    f.destroy();



    data = (char*)memalloc(FILESIZE);
    memset(data, 4, FILESIZE);
    arc.add(data, FILESIZE, L"fileD", 5);
    memfree(data);
   
    //12 allocs
    //arc.print();

  
    printf("_______________\n\n\n");



    folder fold;
   
    fold.build(L"folderA", 7);

    data = (char*)memalloc(FILESIZE);
    memset(data, 'a', FILESIZE);
    fold.add(data, FILESIZE, L"folderA_FileA", 13);
    memfree(data);
    data = (char*)memalloc(FILESIZE);
    memset(data, 'b', FILESIZE);
    fold.add(data, FILESIZE, L"folderA_FileB", 13);
    memfree(data);
    data = (char*)memalloc(FILESIZE);
    memset(data, 'c', FILESIZE);
    fold.add(data, FILESIZE, L"folderA_FileC", 13);
    memfree(data);
 
    //22 allocs  (+10 allocs)
    arc.add(&fold);
    //33 allocs

    fold.destroy();
    //23
   

  

    fold.build(L"folderB", 7);
    data = (char*)memalloc(FILESIZE);
    memset(data, 'd', FILESIZE);
    fold.add(data, FILESIZE, L"folderB_FileA", 13);
    memfree(data);
    data = (char*)memalloc(FILESIZE);
    memset(data, 'e', FILESIZE);
    fold.add(data, FILESIZE, L"folderB_FileB", 13);
    memfree(data);
    data = (char*)memalloc(FILESIZE);
    memset(data, 'f', FILESIZE);
    fold.add(data, FILESIZE, L"folderB_FileC", 13);
    memfree(data);
    arc.add(&fold);
    //fold.print();
    fold.destroy();
    printf("\n\n\n");
    //34
  

    //file* pf=arc.travel(L"folderA\\folderA_FileA",21);
    //wprintf(L"\n%.*s %u %p\n",(unsigned int) pf->name_size,pf->name, (unsigned int)pf->name_size, pf);

 

    
  
    
    //printf("ret : %i\n",arc.save(L"huj"));

    //arc.print();
    //arc.save(L"out.xaf");
   
    arc.destroy();
    
    //0

    //arc.read(L"out.xaf");
    //34
    //arc.destroy();
    //0
   
    xaf_password xp;

    xp.set((const byte*)"huj123", 6);

    puts("___arc.folder_import_no_logs___");
    arc.folder_import_no_logs(L"C:\\Users\\Dawid\\Desktop\\NP\\*");

    arc.print();
    puts("________arc.save________");
    arc.save(L"D:\\temp_encrypted.xaf",&xp);
    arc.save(L"D:\\temp4_goal.xaf");
    puts("\n\n");
    arc.print();
    puts("\n\n");
    arc.destroy();


    
    puts("___________arc.read_____________");
    arc.read(L"D:\\temp_encrypted.xaf",&xp);
    
    arc.print();

    puts("___________arc.export_to___________");
    arc.export_to(L"D:\\Test3");
    arc.destroy();

    puts("___________export_xaf_file___________");
    export_xaf_file(L"D:\\temp.xaf", L"D:\\Test2");

    
    xp.destroy();
    puts("___________folder_to_xaf___________");
    folder_to_xaf(L"C:\\Users\\Dawid\\Desktop\\NP\\*", L"D:\\Test4.xaf");

    */
    OPENFILENAME ofn;       // common dialog box structure
    wchar_t szFile[260] = { 0 };       // if using TCHAR macros

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = h_console;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"All\0*.*\0";
    //ofn.lpstrFilter = L"All\0*.*\0XAF files\0*.XAF\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;


    /*
    UINT* screen_buffer = (UINT*)memalloc(1280*720*4);
    HDC console_dc = GetDC(h_console);
    HDC screen_dc = GetDC(NULL),buffer_dc=CreateCompatibleDC(screen_dc);
    HBITMAP hbmp = CreateBitmap(1280,720,1,32, screen_buffer);
    BITMAPINFO bmi;
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = 1280;
    bmi.bmiHeader.biHeight = 720;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;
    bmi.bmiHeader.biClrUsed = 0;
    bmi.bmiHeader.biClrImportant = 0;
    SelectObject(buffer_dc, hbmp);
    */
    
    // use ofn.lpstrFile
    nc::folder folder,*PTRFOLDER;
    nc::folder_cell * PTRFOLDERCELL=NULL;
  
    int eh;
    nc::xaf *cur_arc=&arc;
    load_textures(renderer);
   time_t start, end;
   int draw_files = argc ? 1 : 0;
   SDL_SysWMinfo wmInfo;
   SDL_VERSION(&wmInfo.version);
   SDL_GetWindowWMInfo(wind, &wmInfo);
   HWND hwnd = wmInfo.info.win.window;

   HDC wind_dc = GetDC(hwnd);

   SDL_Rect rect_file_bg;
   rect_file_bg.x = 81;
   rect_file_bg.y = 80;
   rect_file_bg.w = 1199;
   rect_file_bg.h = 640;
   if (draw_files) {
       file_gui_iter(wind, renderer, &arc_arr[0].file_arr, mouse_pos, tex_bg, 0);
   }
    int click;
    int wind_x, wind_y;
    size_t open_folder = (size_t)-1;
    int button_clicked=0,last_button_clicked=0;
    while (eh = event_handler()) {
        //e = clock();
        //printf("%lli\n",e-s);
       // s = e;
        click = eh == 2 ? 1 : 0;
        button_clicked =interface_loop(mouse_pos, click,renderer);
        if (draw_files) {
            open_folder=file_gui_iter(wind, renderer, &arc_arr[0].file_arr, mouse_pos, tex_bg, click);
            if (open_folder != (size_t)-1) {
                if (arc_arr[0].file_arr[open_folder].type == XAF_FOLDER) {
                    arc_arr[0].open_folder(open_folder);
                    SDL_RenderCopy(renderer, tex_bg, &rect_file_bg, &rect_file_bg);
                    file_selected = (size_t)-1;
                }
                else {
                    file_selected = open_folder;
                }
                ginterface_refresh = 1;
            }
        }
        
        if (ginterface_refresh) {
            SDL_RenderPresent(renderer);
            ginterface_refresh = 0;
        }
        if (GetAsyncKeyState(VK_ESCAPE)) {
            file_selected = (size_t)-1;
        }
        if (button_clicked == 1) {
            if (file_selected != (size_t)-1) {
                //arc_arr[0].export_file(file_selected,renderer, loading_file_tex, 12, file_icon_w, file_icon_h);
                start = clock();
                arc_arr[0].export_file(file_selected);
                end = clock();
                printf("Took %lli ms\n", end - start);
            }
        }

        if (GetAsyncKeyState(VK_F1)|| (button_clicked==2)) {
            GetOpenFileName(&ofn);
            start = clock();
            //draw_files++;
            wprintf(L"Opening %s\n", ofn.lpstrFile);
            cur_arc->add(ofn.lpstrFile);
            memset(ofn.lpstrFile, 0, sizeof(wchar_t)*260);
            end = clock();
            printf("Took %lli ms\n",end-start);
        }
        if (GetAsyncKeyState(VK_F2) || (button_clicked == 3)) {
            folder.build(L"huj", 3);
            cur_arc->add(&folder,&PTRFOLDERCELL);
            folder.destroy();
            if (PTRFOLDERCELL) {
                puts("Created and entered folder huj");
                cur_arc = PTRFOLDERCELL->data.subfiles();
            }
            
            //printf(" %p %p\n",  PTRFOLDERCELL->data.parent, cur_arc);
            SDL_Delay(250);
        }
        if (GetAsyncKeyState(VK_F3) || (button_clicked == 4)) {
           ret=cur_arc->remove(L"huj", 3);
            
           switch (ret) {

           case 0:
               puts("No folder huj found");
               break;
           case 1:
               puts("The file huj has been removed");
               break;
           case 2:
               puts("The folder huj has been removed");
               break;

          }
           SDL_Delay(250);
        }
        if (GetAsyncKeyState('A')) {
           // nc_debug::allocs.print();
           // printf("%llu\n", nc_debug::allocs.size);
            /*
            SDL_GetWindowPosition(wind,&wind_x, &wind_y);
            BitBlt(buffer_dc, 0, 0, 1280, 720, screen_dc, wind_x, wind_y, SRCCOPY);
            //GetDIBits(buffer_dc, hbmp, 0, 720, screen_buffer, &bmi, DIB_RGB_COLORS);
            BitBlt(wind_dc, 0, 0, 1280, 720, buffer_dc, 0, 0, SRCCOPY);
            BitBlt(console_dc, 0, 0, 1280, 720, buffer_dc, 0, 0, SRCCOPY);
            */
            SDL_Delay(250);
        }
        
        if (GetAsyncKeyState('P')) {   
            /*
            PTRFOLDER = cur_arc->get_folder();
            if (PTRFOLDER) {
                //cur_arc = PTRFOLDER->parent->subfiles();
                //printf("Found parent %p %p %p", PTRFOLDER, PTRFOLDER->parent, cur_arc);
                if (PTRFOLDER->parent) {
                    cur_arc = PTRFOLDER->parent;
                    puts("Found parent");
                }
                else {
                    puts("Found no parent ; going to the root");
                    cur_arc = &arc;
                }
            }
            else {
                puts("Found no parent ; already at the root");
            }
            */
            ret = arc_arr[0].go_back_to_parent();
            switch(ret){
                case 0:
                    puts("No parent");
                    break;
                case 1:
                    SDL_RenderCopy(renderer, tex_bg, &rect_file_bg, &rect_file_bg);
                    file_gui_iter(wind, renderer, &arc_arr[0].file_arr, mouse_pos, tex_bg, click);
                    puts("At root level");
                    break;
                case 2:
                    SDL_RenderCopy(renderer, tex_bg, &rect_file_bg, &rect_file_bg);
                    file_gui_iter(wind, renderer, &arc_arr[0].file_arr, mouse_pos, tex_bg, click);
                    puts("Going up");
                    break;
            }
            SDL_RenderPresent(renderer);
            SDL_Delay(250);
        }
        if (GetAsyncKeyState('D')) {
            cur_arc->print();
            SDL_Delay(250);
        }
        
    }
    //nc_debug::allocs.destroy();
    //scanf("%i", &ret);
    return 0;
}
