/*--------------------------------------------------------------------------*/
/* DISPFUNC : the text screen that is kept on top of the graphics screen    */
/*                                                                          */
/* Rebuilt from the Plain Design GUI library ver 1.01 (BC_GUI.LIB, module   */
/* DISPFUNC).  Every character written through writechar() is drawn and     */
/* also remembered, with its color, so that it can be read back.            */
/*                                                                          */
/* The original kept an 80 x 30 screen; here the screen is as many          */
/* characters as the mode has room for (100 x 37 at 800x600, 128 x 48 at    */
/* 1024x768, 160 x 64 at 1280x1024).                                        */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gui.h"
#include "gport.h"

int             font_width;       /* font width & height in graphics        */
int             font_height;
int             gr_scr_xsize;     /* current video mode screen size         */
int             gr_scr_ysize;
int             txt_scr_xsize;    /* current mode text screen size          */
int             txt_scr_ysize;
unsigned char  *sc;               /* the characters and their colors        */
int             text_show;        /* writechar draws (1) or only remembers  */
int             graphic_curs_x;
int             graphic_curs_y;
int             graphic_curs_state;

void init_mouse(void);
int  init_buttons(void);
void draw_curs(int x, int y);

/* the mouse pointer: 10 across, 16 down; 0x14 is "nothing here" */
#define CURS_W 10
#define CURS_H 16
#define T 0x14
static unsigned char curs_shape[CURS_H][CURS_W] =
{
  { 0, 0, T, T, T, T, T, T, T, T },
  { 0,15, 0, T, T, T, T, T, T, T },
  { 0,15,15, 0, T, T, T, T, T, T },
  { 0,15,15,15, 0, T, T, T, T, T },
  { 0,15,15,15,15, 0, T, T, T, T },
  { 0,15,15,15,15,15, 0, T, T, T },
  { 0,15,15,15,15,15,15, 0, T, T },
  { 0,15,15,15,15,15,15,15, 0, T },
  { 0,15,15,15,15,15,15,15,15, 0 },
  { 0,15,15,15,15,15, 0, 0, 0, 0 },
  { 0,15,15, 0,15,15, 0, T, T, T },
  { 0,15, 0, T, 0,15,15, 0, T, T },
  { 0, 0, 0, T, 0,15,15, 0, T, T },
  { T, T, T, T, T, 0,15,15, 0, T },
  { T, T, T, T, T, 0,15,15, 0, T },
  { T, T, T, T, T, T, 0, 0, 0, T }
};
#undef T

static char GUI_FAR *curs_save;   /* what the pointer covers                */
static int           curs_on;     /* is it drawn                            */
static int           curs_x;      /* where                                  */
static int           curs_y;

/*--------------------------------------------------------------------------*/
static void not_initialized(void)
{
  if (initvar == 0)
  {
    printf("\a \n Display borad not initialized");
    getkey_wait();
    exit(-1);
  }
}

/* where the character at x,y is remembered; NULL off the screen */
static unsigned char *cell(int x, int y)
{
  if (x < 0 || y < 0 || x >= txt_scr_xsize || y >= txt_scr_ysize) return NULL;
  return sc + ((long) y * txt_scr_xsize + x) * 2;
}
/*--------------------------------------------------------------------------*/
/* write a character at X,Y                                                 */
/*--------------------------------------------------------------------------*/
void writechar(int x, int y, unsigned char c, int color)
{
  unsigned char *p;

  if (font_width == 1)
  {
    if (x >= 0 && y >= 0 && x < 80 && y < 50)
    {
#if defined(__DJGPP__)
      volatile unsigned char *t = (volatile unsigned char *) (0xB8000UL + __djgpp_conventional_base) + y * 160 + x * 2;
#elif defined(GUI32)
      volatile unsigned char *t = (volatile unsigned char *) 0xB8000UL + y * 160 + x * 2;
#else
      volatile unsigned char far *t = (volatile unsigned char far *) MK_FP(0xB800, y * 160 + x * 2);
#endif
      t[0] = c;
      t[1] = (unsigned char) color;
    }
  }
  else if (text_show == 1)
  {
    DISPLAYCHAR(c, x * font_width, y * font_height, color & 0xf, color >> 4);
  }

  p = cell(x, y);
  if (p != NULL)
  {
    p[0] = c;
    p[1] = (unsigned char) color;
  }
}
/*--------------------------------------------------------------------------*/
/* write a character at graphics coordinates; it is not remembered          */
/*--------------------------------------------------------------------------*/
void gwritechar(int x, int y, unsigned char c, int color)
{
  if (font_width == 1) return;
  DISPLAYCHAR(c, x, y, color & 0xf, color >> 4);
}
/*--------------------------------------------------------------------------*/
/* write a character in window handle h                                     */
/*--------------------------------------------------------------------------*/
void writecharh(int x, int y, unsigned char c, int color, int h)
{
  if (h < 0 || h >= MAX_WINDOWS) return;
  if (x >= 0 && x < xwinsize[h] && y >= 0 && y < ywinsize[h])
  {
    writechar(x + xwin[h], y + ywin[h], c, color);
  }
}
/*--------------------------------------------------------------------------*/
/* the library is started here, after the video mode has been set           */
/*--------------------------------------------------------------------------*/
void initdisplay(void)
{
  int  mode;
  int  i;
  long cells;

  mode = get_video_mode();

  if (gfx_setup(mode))
  {
    /* a graphics mode: gfx_setup() has set the font and the screen size */
    txt_scr_xsize = gr_scr_xsize / font_width;
    txt_scr_ysize = gr_scr_ysize / font_height;
  }
  else
  {
    /* text: 80 x 25, a "pixel" is a character */
    font_width    = 1;
    font_height   = 1;
    gr_scr_xsize  = txt_scr_xsize = 80;
    gr_scr_ysize  = txt_scr_ysize = 25;
  }

  for (i = 0; i < MAX_WINDOWS + 5; i++)
  {
    save_box[i] = NULL;
  }

  if (sc != NULL) free(sc);
  cells = (long) txt_scr_xsize * txt_scr_ysize;
  sc = (unsigned char *) malloc((size_t) (cells * 2));
  if (sc == NULL)
  {
    printf("\a not enough memory for character layer memory");
    exit(-1);
  }
  memset(sc, 0, (size_t) (cells * 2));

  show_win_num = 0;
  initvar      = 1;
  text_show    = 1;

  txt_curs(INIT);
  init_mouse();
  gcurs(INIT);
  frect_char(' ');
  init_buttons();
}
/*--------------------------------------------------------------------------*/
/* read a character from location X,Y, and its color                        */
/*--------------------------------------------------------------------------*/
unsigned char readchar(int x, int y)
{
  unsigned char *p = cell(x, y);

  return (p != NULL) ? p[0] : 0;
}

int readcolor(int x, int y)
{
  unsigned char *p = cell(x, y);

  return (p != NULL) ? p[1] : 0;
}
/*--------------------------------------------------------------------------*/
/* the mouse pointer in graphics modes: the library draws it itself         */
/*                                                                          */
/*   INIT   get memory for what the pointer covers                          */
/*   1      show it where the mouse is                                      */
/*   0      take it away                                                    */
/*   MOVE   move it if the mouse has moved (call this in loops)             */
/*   REPORT copy its place and state to graphic_curs_x, _y, _state          */
/*--------------------------------------------------------------------------*/
void gcurs(int action)
{
  int x, y;

  if (mousefound == 0) return;
  if (font_width == 1) return;

  if (action != 1 && action != 0 && action != INIT && action != MOVE && action != REPORT)
  {
    printf("error action=%d", action);
    exit(-1);
  }

  switch (action)
  {
    case INIT:
     curs_on = 0;
     if (curs_save != NULL) gui_farfree(curs_save);
     curs_save = (char GUI_FAR *) gui_farmalloc(get_block_size(0, 0, CURS_W - 1, CURS_H - 1));
     if (curs_save == NULL)
     {
       printf("\n farmalloc failed for cursor memory");
       exit(-1);
     }
     break;

    case MOVE:
     get_ml(&x, &y);
     if (x == curs_x && y == curs_y) break;
     gcurs(0);
     curs_x = x;
     curs_y = y;
     /* and show it at the new place */

    case 1:
     if (curs_on == 1) break;
     if (action != MOVE)
     {
       get_ml(&x, &y);
       curs_x = x;
       curs_y = y;
     }
     GET_IMAGE(curs_x, curs_y, curs_x + CURS_W - 1, curs_y + CURS_H - 1, curs_save);
     draw_curs(curs_x, curs_y);
     curs_on = 1;
     break;

    case 0:
     if (curs_on == 0) break;
     RESTORE_IMAGE(curs_save, curs_x, curs_y);
     curs_on = 0;
     break;

    case REPORT:
     graphic_curs_x     = curs_x;
     graphic_curs_y     = curs_y;
     graphic_curs_state = curs_on;
     break;
  }
}
/*--------------------------------------------------------------------------*/
void draw_curs(int x, int y)
{
  int i, j;

  for (j = 0; j < CURS_H; j++)
  {
    for (i = 0; i < CURS_W; i++)
    {
      if (curs_shape[j][i] != 0x14 && x + i < gr_scr_xsize && y + j < gr_scr_ysize)
      {
        SETPIX(x + i, y + j, curs_shape[j][i]);
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
/* write a string starting at X,Y of window h                               */
/*--------------------------------------------------------------------------*/
void plsh(int x, int y, unsigned char string[], int color, int h)
{
  int md;
  int i;

  if (h == -1)
  {
    pls(x, y, string, color);
    return;
  }

  not_initialized();

  md = mouse_display;
  if (mouse_display == 1) hide_mouse_cursor();

  for (i = 0; (unsigned) i < strlen((char *) string); i++)
  {
    writecharh(x + i, y, string[i], color, h);
  }

  if (md == 1) show_mouse_cursor();
}
/*--------------------------------------------------------------------------*/
/* write a string starting at X,Y                                           */
/*--------------------------------------------------------------------------*/
void pls(int x, int y, unsigned char string[], int color)
{
  int md;
  int i;

  not_initialized();

  md = mouse_display;
  if (mouse_display == 1) hide_mouse_cursor();

  for (i = 0; (unsigned) i < strlen((char *) string); i++)
  {
    writechar(x + i, y, string[i], color);
  }

  if (md == 1) show_mouse_cursor();
}
/*--------------------------------------------------------------------------*/
/* write a string at graphics coordinates                                   */
/*--------------------------------------------------------------------------*/
void gpls(int x, int y, unsigned char string[], int color)
{
  int md;
  int i;

  not_initialized();

  md = mouse_display;
  if (mouse_display == 1) hide_mouse_cursor();

  for (i = 0; (unsigned) i < strlen((char *) string); i++)
  {
    gwritechar(x + i * font_width, y, string[i], color);
  }

  if (md == 1) show_mouse_cursor();
}
/*--------------------------------------------------------------------------*/
/* the same, without taking the mouse pointer away                          */
/*--------------------------------------------------------------------------*/
void cgpls(int x, int y, unsigned char string[], int color)
{
  int i;

  for (i = 0; (unsigned) i < strlen((char *) string); i++)
  {
    gwritechar(x + i * font_width, y, string[i], color);
  }
}
/*--------------------------------------------------------------------------*/
