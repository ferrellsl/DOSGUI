/*--------------------------------------------------------------------------*/
/* BASE : text rectangles, keyboard, mouse, text cursor, video mode, and    */
/*        the lines and rectangles of graphics modes                        */
/*                                                                          */
/* Rebuilt from the Plain Design GUI library ver 1.01 (BC_GUI.LIB: the      */
/* general functions of module MENU, and module PIX).                       */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gui.h"
#include "gport.h"

int            mousefound;        /* YES or NO  exist ?                     */
int            mouse_x;           /* mouse x,y in text mode coordinates     */
int            mouse_y;
int            txt_curs_x;        /* text cursor coordinates                */
int            txt_curs_y;
int            ms;                /* mouse state varaible                   */
int            dc;                /* mouse double click state               */
unsigned char  back_block = ' ';  /* character for fillrect in text mode    */
int            initvar;           /* initdisplay has been called            */
int            mouse_display;     /* the mouse pointer is shown             */
int            curs_start_line = 7;
int            curs_end_line   = 8;
char           xor_var;           /* 0x18 while lines are to be XORed       */

static GREGS   iReg;

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

static void mouse_int(int function)
{
  iReg.ax = (unsigned short) function;
  gp_int(0x33, &iReg);
}
/*--------------------------------------------------------------------------*/
/*  Draw a rectangle (text mode line characters)                            */
/*--------------------------------------------------------------------------*/
void rect(int x1, int y1, int x2, int y2, int color)
{
  int md;
  int i, j;

  md = mouse_display;
  if (mouse_display == 1) hide_mouse_cursor();

  if (x1 <= x2 && y1 <= y2)
  {
    for (i = x1; i < x2; i++)
    {
      writechar(i, y1, 0xC4, color);
      writechar(i, y2, 0xC4, color);
    }
    for (j = y1 + 1; j < y2; j++)
    {
      writechar(x1, j, 0xB3, color);
      writechar(x2, j, 0xB3, color);
    }
    writechar(x1, y1, 0xDA, color);
    writechar(x2, y1, 0xBF, color);
    writechar(x1, y2, 0xC0, color);
    writechar(x2, y2, 0xD9, color);
  }

  if (md == 1) show_mouse_cursor();
}
/*--------------------------------------------------------------------------*/
/*  Draw a double line rectangle                                            */
/*--------------------------------------------------------------------------*/
void dl_rect(int x1, int y1, int x2, int y2, int color)
{
  int md;
  int i, j;

  md = mouse_display;
  if (mouse_display == 1) hide_mouse_cursor();

  if (x1 <= x2 && y1 <= y2)
  {
    for (i = x1; i < x2; i++)
    {
      writechar(i, y1, 0xCD, color);
      writechar(i, y2, 0xCD, color);
    }
    for (j = y1 + 1; j < y2; j++)
    {
      writechar(x1, j, 0xBA, color);
      writechar(x2, j, 0xBA, color);
    }
    writechar(x1, y1, 0xC9, color);
    writechar(x2, y1, 0xBB, color);
    writechar(x1, y2, 0xC8, color);
    writechar(x2, y2, 0xBC, color);
  }

  if (md == 1) show_mouse_cursor();
}
/*--------------------------------------------------------------------------*/
/*  Draw a filled rectangle, with a shadow if shadow is a color             */
/*--------------------------------------------------------------------------*/
void frect(int x1, int y1, int x2, int y2, int color, int shadow)
{
  int md;
  int i, j;

  /* the whole 80 x 25 screen, in a graphics mode: paint all of the screen */
  if (font_width > 1 && shadow == 0 && x1 == 0 && y1 == 0 && x2 == 79 && y2 == 24)
  {
    gfillrect(0, 0, gr_scr_xsize - 1, gr_scr_ysize - 1, color & 0xf);
    return;
  }

  not_initialized();

  md = mouse_display;
  if (mouse_display == 1) hide_mouse_cursor();

  for (j = y1; j <= y2; j++)
  {
    for (i = x1; i <= x2; i++)
    {
      writechar(i, j, back_block, color);
    }
  }

  if (shadow != 0)
  {
    chcolor(x1 + 2, y2 + 1, x2 - x1 + 1, 1, shadow);
    chcolor(x2 + 1, y1 + 1, 2, y2 - y1, shadow);
  }

  if (md == 1) show_mouse_cursor();
}
/*--------------------------------------------------------------------------*/
/*  Return the key pressed, EOF when no action                              */
/*  (keys without a character are their scan code + 256)                    */
/*--------------------------------------------------------------------------*/
int getkey(void)
{
  unsigned k;
  int      ch, scan;

  if (!gp_kbhit()) return -1;

  k    = gp_readkey();
  ch   = k & 0xff;
  scan = (k & 0xff00) >> 8;

  if (ch == 0) return scan + 0x100;
  return ch;
}
/*--------------------------------------------------------------------------*/
/*  change color of the character block                                     */
/*--------------------------------------------------------------------------*/
void chcolor(int x, int y, int xsize, int ysize, int color)
{
  int md;
  int i, j;

  md = mouse_display;
  if (mouse_display == 1) hide_mouse_cursor();

  for (j = y; j < y + ysize; j++)
  {
    for (i = x; i < x + xsize; i++)
    {
      writechar(i, j, readchar(i, j), color);
    }
  }

  if (md == 1) show_mouse_cursor();
}
/*--------------------------------------------------------------------------*/
/*  the same for a menu's bar: in graphics modes the bar is a little wider  */
/*  than its characters                                                     */
/*--------------------------------------------------------------------------*/
void mchcolor(int x, int y, int xsize, int ysize, int color)
{
  int md;
  int i, j;

  md = mouse_display;
  if (mouse_display == 1) hide_mouse_cursor();

  for (j = y; j < y + ysize; j++)
  {
    for (i = x; i < x + xsize; i++)
    {
      writechar(i, j, readchar(i, j), color);
    }
  }

  if (font_width > 1)
  {
    gfillrect((x - 1) * font_width + 1, y * font_height,
              x * font_width - 1, (y + 1) * font_height - 1, color >> 4);
    x += xsize + 1;
    gfillrect((x - 1) * font_width, y * font_height,
              x * font_width - 2, (y + 1) * font_height - 1, color >> 4);
  }

  if (md == 1) show_mouse_cursor();
}
/*--------------------------------------------------------------------------*/
void mchcolorh(int x, int y, int xsize, int ysize, int color, int h)
{
  int md;
  int i, j;

  md = mouse_display;
  if (mouse_display == 1) hide_mouse_cursor();

  for (j = y; j < y + ysize; j++)
  {
    for (i = x; i < x + xsize; i++)
    {
      writecharh(i, j, readchar(i + xwin[h], j + ywin[h]), color, h);
    }
  }

  if (font_width > 1)
  {
    gfillrect((xwin[h] + x - 1) * font_width + 1, (y + ywin[h]) * font_height,
              (x + xwin[h]) * font_width - 1, (y + ywin[h] + 1) * font_height - 1, color >> 4);
    x += xsize + 1;
    gfillrect((xwin[h] + x - 1) * font_width, (y + ywin[h]) * font_height,
              (x + xwin[h]) * font_width - 2, (y + ywin[h] + 1) * font_height - 1, color >> 4);
  }

  if (md == 1) show_mouse_cursor();
}
/*--------------------------------------------------------------------------*/
/*                                 MOUSE                                    */
/*--------------------------------------------------------------------------*/
void init_mouse(void)
{
  unsigned      off, seg;
  unsigned char first;

  /* is there a mouse driver behind interrupt 33h ? */
  off   = gp_peekw(0x33 * 4UL);
  seg   = gp_peekw(0x33 * 4UL + 2);
  first = gp_peekb(((unsigned long) seg << 4) + off);

  if ((off == 0 && seg == 0) || first == 0xCF)
  {
    mousefound = 0;
    ms         = RELEASED;
    return;
  }

  memset(&iReg, 0, sizeof(iReg));
  mouse_int(0);
  if (iReg.ax == 0)
  {
    mousefound = 0;
    ms         = RELEASED;
    return;
  }
  mousefound = 1;

  /* the driver does not know the size of a VESA mode's screen */
  if (font_width > 1)
  {
    iReg.cx = 0;
    iReg.dx = (unsigned short) (gr_scr_xsize - 1);
    mouse_int(7);
    iReg.cx = 0;
    iReg.dx = (unsigned short) (gr_scr_ysize - 1);
    mouse_int(8);
    iReg.cx = (unsigned short) (gr_scr_xsize / 2);
    iReg.dx = (unsigned short) (gr_scr_ysize / 2);
    mouse_int(4);
  }

  dc = -1;
  mouse_state();
  if (mouse_left_pressed() == 1) ms = PRESSED;
  else                           ms = RELEASED;
}
/*--------------------------------------------------------------------------*/
void show_mouse_cursor(void)
{
  if (font_width == 1)
  {
    if (mousefound == 0) return;
    mouse_int(1);
  }
  else
  {
    gcurs(1);
  }
  mouse_display = 1;
}
/*--------------------------------------------------------------------------*/
void hide_mouse_cursor(void)
{
  if (mousefound == 0) return;

  if (font_width == 1)
  {
    mouse_int(2);
    mouse_display = 0;
  }
  else
  {
    gcurs(0);
  }
}
/*--------------------------------------------------------------------------*/
/* the mouse's place in text coordinates, to mouse_x and mouse_y            */
/*--------------------------------------------------------------------------*/
void get_mouse_location(void)
{
  if (mousefound == 0) return;

  mouse_int(3);
  mouse_x = (short) iReg.cx;
  mouse_y = (short) iReg.dx;

  if (font_width > 1)
  {
    mouse_x /= font_width;
    mouse_y /= font_height;
  }
  else
  {
    mouse_x /= 8;
    mouse_y /= 8;
  }
}
/*--------------------------------------------------------------------------*/
/* the mouse's place in graphics coordinates                                */
/*--------------------------------------------------------------------------*/
void get_ml(int *x, int *y)
{
  if (mousefound == 0) return;

  mouse_int(3);
  *x = (short) iReg.cx;
  *y = (short) iReg.dx;
}
/*--------------------------------------------------------------------------*/
void set_ml(int x, int y)
{
  if (mousefound == 0) return;

  iReg.cx = (unsigned short) x;
  iReg.dx = (unsigned short) y;
  mouse_int(4);
}
/*--------------------------------------------------------------------------*/
/* returns YES or NO for the status of the mouse left key                   */
/*--------------------------------------------------------------------------*/
int mouse_left_pressed(void)
{
  if (mousefound == 0) return 0;

  mouse_int(3);
  if (iReg.bx == 1) return 1;
  return 0;
}
/*--------------------------------------------------------------------------*/
/* sets ms : PRESSED, RELEASED or PRESSEDRELEASED                           */
/* and  dc : the same, or DOUBLECLICKED for a second press that comes       */
/*           within DOUBLECLICKDELAY milliseconds of the first              */
/*--------------------------------------------------------------------------*/
int mouse_state(void)
{
  static long press_time;
  int         lp;
  long        elapsed;

  if (dc == -1)
  {
    press_time = gp_ms();
    ms = RELEASED;
    dc = RELEASED;
    return -1;
  }

  lp = mouse_left_pressed();

  if (lp == 1)
  {
    ms = PRESSED;
  }
  else if (ms == PRESSED && lp == 0)
  {
    ms = PRESSEDRELEASED;
  }

  if (lp == 1)
  {
    switch (dc)
    {
      case RELEASED:
       press_time = gp_ms();
       dc = PRESSED;
       break;
      case PRESSEDRELEASED:
       elapsed = gp_ms() - press_time;
       if (elapsed < DOUBLECLICKDELAY) dc = DOUBLECLICKED;
       else                            dc = PRESSED;
       break;
      case DOUBLECLICKED:
       dc = RELEASED;
       break;
    }
  }
  else
  {
    switch (dc)
    {
      case PRESSED:
       elapsed = gp_ms() - press_time;
       if (elapsed < DOUBLECLICKDELAY) dc = PRESSEDRELEASED;
       else                            dc = RELEASED;
       break;
      case PRESSEDRELEASED:
       elapsed = gp_ms() - press_time;
       if (elapsed > DOUBLECLICKDELAY) dc = RELEASED;
       break;
      case DOUBLECLICKED:
       dc = RELEASED;
       break;
    }
  }

  return ms;
}
/*--------------------------------------------------------------------------*/
/*  limit mouse movement (text coordinates)                                 */
/*--------------------------------------------------------------------------*/
void limit_mouse(int x1, int y1, int x2, int y2)
{
  if (mousefound == 0) return;

  if (font_width > 1)
  {
    iReg.cx = (unsigned short) (x1 * font_width);
    /* up to the last column means up to the edge of the screen */
    if (x2 >= txt_scr_xsize - 1) iReg.dx = (unsigned short) (gr_scr_xsize - 1);
    else                         iReg.dx = (unsigned short) (x2 * font_width);
  }
  else
  {
    iReg.cx = (unsigned short) (x1 << 3);
    iReg.dx = (unsigned short) (x2 << 3);
  }
  mouse_int(7);

  if (font_width > 1)
  {
    iReg.cx = (unsigned short) (y1 * font_height);
    if (y2 >= txt_scr_ysize - 1) iReg.dx = (unsigned short) (gr_scr_ysize - 1);
    else                         iReg.dx = (unsigned short) (y2 * font_height);
  }
  else
  {
    iReg.cx = (unsigned short) (y1 << 3);
    iReg.dx = (unsigned short) (y2 << 3);
  }
  mouse_int(8);
}
/*--------------------------------------------------------------------------*/
/* the text cursor                                                          */
/*                                                                          */
/* in text mode:   0 off, 1 on, 2 a block, 3 half a block, 4 a line,        */
/*                 5 move it to txt_curs_x, txt_curs_y                      */
/* in graphics:    INIT; 1 on, 0 off; 5 move it to txt_curs_x, txt_curs_y;  */
/*                 8 let it blink (call this in loops); 7 blink off now     */
/*--------------------------------------------------------------------------*/
void txt_curs(int action)
{
  static long blink_time;
  static int  cx, cy;             /* where the cursor is                    */
  static int  on;                 /* there is one                           */
  static int  phase = 7;          /* 8 while its line is drawn, 7 while not */
  static int  under_ch, under_color;
  GREGS r;
  int   mx, my;
  int   over;                     /* the mouse pointer is on the cursor     */
  int   x, row;
  long  elapsed;

  if (font_width == 1)
  {
    memset(&r, 0, sizeof(r));
    switch (action)
    {
      case 0:
       r.ax = 0x0100;
       r.cx = 0x0100;                         /* start after end: no cursor */
       break;
      case 2:
       curs_start_line = 0; curs_end_line = 8;
       r.ax = 0x0100;
       r.cx = (unsigned short) ((curs_start_line << 8) | curs_end_line);
       break;
      case 3:
       curs_start_line = 4; curs_end_line = 8;
       r.ax = 0x0100;
       r.cx = (unsigned short) ((curs_start_line << 8) | curs_end_line);
       break;
      case 4:
       curs_start_line = 8; curs_end_line = 8;
       r.ax = 0x0100;
       r.cx = (unsigned short) ((curs_start_line << 8) | curs_end_line);
       break;
      case 1:
       r.ax = 0x0100;
       r.cx = (unsigned short) ((curs_start_line << 8) | curs_end_line);
       break;
      case 5:
       r.ax = 0x0200;
       r.bx = 0;
       r.dx = (unsigned short) ((txt_curs_y << 8) | (txt_curs_x & 0xff));
       break;
      default:
       return;
    }
    gp_int(0x10, &r);
    return;
  }

  mx = my = -1000;
  get_ml(&mx, &my);
  over = 1;
  if (!(txt_curs_x * font_width <= mx + 10 && txt_curs_x * font_width + font_width - 1 >= mx &&
        txt_curs_y * font_height <= my + 16 && txt_curs_y * font_height + font_height - 1 >= my))
  {
    over = 0;
  }

  switch (action)
  {
    case INIT:
     blink_time = gp_ms();
     on = 0;
     break;

    case 1:
     if (on == 1) break;
     on    = 1;
     phase = 7;
     break;

    case 0:
     if (on == 0) break;
     if (phase == 8)
     {
       if (over == 1) gcurs(0);
       writechar(cx, cy, (unsigned char) under_ch, under_color);
       if (over == 1) gcurs(1);
     }
     on = 0;
     break;

    case 7:
     if (on == 0) break;
     if (phase == 7) break;
     phase = 7;
     if (over == 1) gcurs(0);
     writechar(cx, cy, (unsigned char) under_ch, under_color);
     if (over == 1) gcurs(1);
     break;

    case 5:
     if (on == 0) break;
     txt_curs(0);
     cy = txt_curs_y;
     cx = txt_curs_x;
     txt_curs(1);
     break;

    case 8:
     if (on == 0) break;
     elapsed = gp_ms() - blink_time;
     if (elapsed < 0) elapsed = 1000;         /* (midnight) */

     if (phase == 8)
     {
       if (elapsed >= 240)
       {
         blink_time = gp_ms();
         txt_curs(7);
       }
     }
     else if (phase == 7)
     {
       if (elapsed >= 160)
       {
         blink_time  = gp_ms();
         under_ch    = readchar(cx, cy);
         under_color = readcolor(cx, cy);

         if (over == 1) gcurs(0);
         for (x = cx * font_width; x < cx * font_width + font_width - 1; x++)
         {
           for (row = font_height - 2; row >= font_height - 4; row--)
           {
             SETPIX(x, cy * font_height + row, (char) (under_color & 0xf));
           }
         }
         if (over == 1) gcurs(1);
         phase = 8;
       }
     }
     break;
  }
}
/*--------------------------------------------------------------------------*/
void cls(void)
{
  if (initvar == 0)
  {
    system("cls");
    printf("\a \n Display borad not initialized");
    getkey_wait();
    exit(-1);
  }
  frect(0, 0, txt_scr_xsize - 1, txt_scr_ysize - 1, 7, 0);
}
/*--------------------------------------------------------------------------*/
/* Returns YES when alt key is pressed                                      */
/*--------------------------------------------------------------------------*/
int alt_pressed(void)
{
  if (gp_shiftstate() & 8) return 1;
  return 0;
}
/*--------------------------------------------------------------------------*/
void frect_char(unsigned char c)
{
  back_block = c;
}
/*--------------------------------------------------------------------------*/
/* read a string starting at X,Y and size length                            */
/*--------------------------------------------------------------------------*/
void readstring(int x, int y, int size, char *string)
{
  int i;

  for (i = x; i < x + size; i++)
  {
    string[i - x] = (char) readchar(i, y);
  }
  string[size] = 0;
}
/*--------------------------------------------------------------------------*/
/* Set the video mode: a BIOS mode number, or a VESA one (0x100 and up)     */
/* returns the mode, or -1 if a VESA mode could not be set                  */
/*--------------------------------------------------------------------------*/
static int current_mode = -1;

int set_video_mode(int mode)
{
  GREGS r;

  memset(&r, 0, sizeof(r));
  if (mode >= 0x100)
  {
    r.ax = 0x4F02;
    r.bx = (unsigned short) mode;
    gp_int(0x10, &r);
    if (r.ax != 0x004F) return -1;
  }
  else
  {
    r.ax = (unsigned short) mode;
    gp_int(0x10, &r);
  }
  current_mode = mode;
  return mode;
}
/*--------------------------------------------------------------------------*/
/* Get the video mode                                                       */
/*--------------------------------------------------------------------------*/
int get_video_mode(void)
{
  GREGS r;

  memset(&r, 0, sizeof(r));
  r.ax = 0x4F03;                              /* a VESA mode ?              */
  gp_int(0x10, &r);
  if (r.ax == 0x004F && (r.bx & 0x3FFF) >= 0x100) return r.bx & 0x3FFF;

  memset(&r, 0, sizeof(r));
  r.ax = 0x0F00;
  gp_int(0x10, &r);
  if (current_mode >= 0x100 && (r.ax & 0x7f) > 0x13) return current_mode;
  return r.ax & 0x7f;
}
/*--------------------------------------------------------------------------*/
/*                   lines and rectangles in graphics modes                 */
/* (the functions that start with 'c' do not take the mouse pointer away)   */
/*--------------------------------------------------------------------------*/
void cgrect(int x1, int y1, int x2, int y2, int color)
{
  GLINE(x1, y1, x2, y1, color, xor_var);
  GLINE(x1, y2, x2, y2, color, xor_var);
  GLINE(x1, y1, x1, y2, color, xor_var);
  GLINE(x2, y1, x2, y2, color, xor_var);
}

void grect(int x1, int y1, int x2, int y2, int color)
{
  gcurs(0);
  cgrect(x1, y1, x2, y2, color);
  gcurs(1);
}
/*--------------------------------------------------------------------------*/
void cgfillrect(int x1, int y1, int x2, int y2, int color)
{
  int y;

  if (y1 > y2)
  {
    y  = y1;
    y1 = y2;
    y2 = y;
  }
  for (y = y1; y <= y2; y++)
  {
    gfx_hline(x1, x2, y, color, xor_var);
  }
}

void gfillrect(int x1, int y1, int x2, int y2, int color)
{
  gcurs(0);
  cgfillrect(x1, y1, x2, y2, color);
  gcurs(1);
}
/*--------------------------------------------------------------------------*/
void cgr_line(int x1, int y1, int x2, int y2, int color)
{
  GLINE(x1, y1, x2, y2, color, xor_var);
}

void gr_line(int x1, int y1, int x2, int y2, int color)
{
  gcurs(0);
  GLINE(x1, y1, x2, y2, color, xor_var);
  gcurs(1);
}
/*--------------------------------------------------------------------------*/
/* Draw a rectangle with Width 'w' pixels, xor the color                    */
/*--------------------------------------------------------------------------*/
void xor_rect(int w, int x1, int y1, int x2, int y2, int color)
{
  int i;

  xor_var = ROP_XOR;
  for (i = 0; i < w; i++)
  {
    grect(x1 + i, y1 + i, x2 - i, y2 - i, color);
  }
  xor_var = 0;
}

void xor_frect(int x1, int y1, int x2, int y2, int color)
{
  xor_var = ROP_XOR;
  gfillrect(x1, y1, x2, y2, color);
  xor_var = 0;
}

void xor_line(int x1, int y1, int x2, int y2, int color)
{
  GLINE(x1, y1, x2, y2, color, ROP_XOR);
}
/*--------------------------------------------------------------------------*/
