/*--------------------------------------------------------------------------*/
/* WINDOW : message boxes (windows) which save and restore what they cover  */
/*                                                                          */
/* Rebuilt from the Plain Design GUI library ver 1.01 (BC_GUI.LIB, module   */
/* WINDOW).  Coordinates are text cells; in graphics modes the pixels under */
/* the window are saved too.                                                */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "gui.h"
#include "gport.h"

int             xwin      [MAX_WINDOWS];
int             ywin      [MAX_WINDOWS];
int             xwinsize  [MAX_WINDOWS];
int             ywinsize  [MAX_WINDOWS];
int             wincolor  [MAX_WINDOWS];
int             winshadow [MAX_WINDOWS];
int             winborder [MAX_WINDOWS];
unsigned char  *save_box  [MAX_WINDOWS + 5];  /* initdisplay clears 25 of them */
unsigned char  *save_box2 [MAX_WINDOWS];
char GUI_FAR   *gsave_box [MAX_WINDOWS];
int             open_win  [MAX_WINDOWS];
int             show_win_num;                 /* total displayed windows      */

void show_win(int h);
void hide_win(int h);
void activate_win(int h);

/*--------------------------------------------------------------------------*/
/* is the text cell x,y inside window h ?                                   */
/*--------------------------------------------------------------------------*/
static int in_win(int h, int x, int y)
{
  return xwin[h] <= x && x < xwin[h] + xwinsize[h] &&
         ywin[h] <= y && y < ywin[h] + ywinsize[h];
}
/*--------------------------------------------------------------------------*/
/* is the text cell x,y on the top row of window h ?  (where it is dragged) */
/*--------------------------------------------------------------------------*/
static int on_win_top(int h, int x, int y)
{
  return xwin[h] <= x && x < xwin[h] + xwinsize[h] && ywin[h] == y;
}
/*--------------------------------------------------------------------------*/
/* copy the characters of a block of text cells to memory, and back         */
/*--------------------------------------------------------------------------*/
static void save_cells(unsigned char *p, int x, int y, int xs, int ys)
{
  int i, j;

  for (j = y; j < y + ys; j++)
  {
    for (i = x; i < x + xs; i++)
    {
      *p++ = readchar(i, j);
      *p++ = (unsigned char) readcolor(i, j);
    }
  }
}

static void restore_cells(unsigned char *p, int x, int y, int xs, int ys)
{
  int i, j;

  for (j = y; j < y + ys; j++)
  {
    for (i = x; i < x + xs; i++)
    {
      writechar(i, j, p[0], p[1]);
      p += 2;
    }
  }
}
/*--------------------------------------------------------------------------*/
/* the outline shown while a window is dragged                              */
/*--------------------------------------------------------------------------*/
static void drag_outline(int x, int y, int xs, int ys)
{
  xor_rect(3, x * font_width, y * font_height,
           (x + xs) * font_width - 1, (y + ys) * font_height, 7);
}
/*--------------------------------------------------------------------------*/
int messagebox(int x, int y, int xsize, int ysize, int color, int shadow, int border)
{
  int  h, i;
  int  xs, ys;
  long size;

  if (initvar == 0)
  {
    gui_fatal("\a \n Display borad not initialized");
  }

  if (font_width > 1) shadow = 0;      /* no text shadows in graphics modes */

  if (shadow == 0)
  {
    ys = ysize;
    xs = xsize;
  }
  else
  {
    ys = ysize + 1;
    xs = xsize + 2;
  }

  h = -1;
  for (i = 0; i < MAX_WINDOWS; i++)
  {
    if (save_box[i] == NULL)
    {
      h = i;
      break;
    }
  }
  if (h == -1)
  {
    gui_fatal("\n out of window handles");
  }

  xwin[h]      = x;
  ywin[h]      = y;
  xwinsize[h]  = xs;
  ywinsize[h]  = ys;
  wincolor[h]  = color;
  winshadow[h] = shadow;
  winborder[h] = border;

  save_box[h] = (unsigned char *) malloc(ys * xs * 2);
  if (save_box[h] == NULL)
  {
    cls();
    printf("\n not enough memory for message box # %d", h);
    getkey_wait();
    exit(-1);
  }

  if (font_width > 1)
  {
    size = get_block_size(x * font_width, y * font_height,
                          (x + xs) * font_width, (y + ys) * font_height + 1);
    gsave_box[h] = (char GUI_FAR *) gui_farmalloc(size);
    if (gsave_box[h] == NULL)
    {
      cls();
      printf("\n not enough memory for graphics message box # %d", h);
      printf("\n tryed to allocate %ld bytes", size);
      getkey_wait();
      exit(-1);
    }
  }

  show_win(h);
  return h;
}
/*--------------------------------------------------------------------------*/
void destroy_messagebox(int h)
{
  hide_win(h);

  free(save_box[h]);
  save_box[h] = NULL;

  if (font_width > 1)
  {
    gui_farfree(gsave_box[h]);
    gsave_box[h] = NULL;
  }
}
/*--------------------------------------------------------------------------*/
/* save what is under window h and paint it                                 */
/*--------------------------------------------------------------------------*/
void show_win(int h)
{
  int x      = xwin[h];
  int y      = ywin[h];
  int xs     = xwinsize[h];        /* size without the shadow              */
  int ys     = ywinsize[h];
  int fxs    = xwinsize[h];        /* full size                            */
  int fys    = ywinsize[h];
  int color  = wincolor[h];
  int shadow = winshadow[h];
  int md;

  if (winshadow[h] != 0)
  {
    xs -= 2;
    ys--;
  }

  md = mouse_display;
  if (mouse_display == 1) hide_mouse_cursor();
  gcurs(0);

  save_cells(save_box[h], x, y, fxs, fys);

  if (font_width > 1)
  {
    GET_IMAGE(x * font_width, y * font_height,
              (x + fxs) * font_width, (y + fys) * font_height, gsave_box[h]);
  }

  if (color != -1)
  {
    if (font_width > 1) text_show = 0;
    frect(x, y, x + xs - 1, y + ys - 1, color, shadow);

    if (font_width > 1)
    {
      text_show = 1;
      gfillrect(x * font_width, y * font_height,
                (x + xs) * font_width - 1, (y + ys) * font_height, color >> 4);
      grect(x * font_width, y * font_height,
            (x + xs) * font_width - 1, (y + ys) * font_height, color & 0xff);
    }

    if (winborder[h] == 1 && font_width == 1)
    {
      rect(x, y, x + xs - 1, y + ys - 1, color);
    }
  }

  if (md == 1) show_mouse_cursor();
  gcurs(1);

  open_win[show_win_num] = h;
  show_win_num++;
}
/*--------------------------------------------------------------------------*/
/* put back what window h covers                                            */
/*--------------------------------------------------------------------------*/
void hide_win(int h)
{
  int md;

  md = mouse_display;
  if (mouse_display == 1) hide_mouse_cursor();
  gcurs(0);

  if (font_width > 1) text_show = 0;
  restore_cells(save_box[h], xwin[h], ywin[h], xwinsize[h], ywinsize[h]);
  if (font_width > 1) text_show = 1;

  if (font_width > 1)
  {
    RESTORE_IMAGE(gsave_box[h], xwin[h] * font_width, ywin[h] * font_height);
  }

  if (md == 1) show_mouse_cursor();
  gcurs(1);

  show_win_num--;
}
/*--------------------------------------------------------------------------*/
/* bring window h to the top: the windows over it are taken away, h is      */
/* taken away, the others are put back and h is shown last                  */
/*--------------------------------------------------------------------------*/
void activate_win(int h)
{
  int above[MAX_WINDOWS];
  int found = 0;
  int n     = show_win_num;
  int i, k;

  if (in_menu == 1) return;

  for (i = show_win_num - 1; i >= 0; i--)
  {
    if (open_win[i] != h)
    {
      above[i] = open_win[i];
      hide_win(open_win[i]);
    }
    else
    {
      hide_win(h);
      found = 1;
      break;
    }
  }

  if (found == 0)
  {
    cls();
    printf("\n cannot activate window handle %d", h);
    getkey_wait();
    exit(-1);
  }

  for (k = i + 1; k < n; k++)
  {
    show_win(above[k]);
  }
  show_win(h);
}
/*--------------------------------------------------------------------------*/
/* window housekeeping, called from getkey:                                 */
/*   a click on a window that is under others brings it to the top          */
/*   (returns its place in the pile);                                       */
/*   the top window is moved by dragging its top row (returns -2);          */
/*   otherwise returns -1                                                   */
/*--------------------------------------------------------------------------*/
int process_windows(void)
{
  static int     drag_ok;
  int            lp;
  int            i, h;
  int            ox, oy;             /* where the mouse was                */
  int            nx, ny, nxs, nys;   /* where the outline is               */
  int            w2, h2;
  unsigned char *tbuf;
  char GUI_FAR  *gbuf = NULL;
  long           size;

  if (show_win_num == 0) return -1;

  lp = mouse_left_pressed();
  if (lp == 1)
  {
    get_mouse_location();
    h = open_win[show_win_num - 1];

    if (!((show_win_num <= 1 || xwin[h] <= mouse_x) &&
          mouse_x < xwin[h] + xwinsize[h] &&
          ywin[h] <= mouse_y && mouse_y < ywin[h] + ywinsize[h]))
    {
      for (i = show_win_num - 2; i >= 0; i--)
      {
        if (in_win(open_win[i], mouse_x, mouse_y))
        {
          activate_win(open_win[i]);
          return i;
        }
      }
    }
  }

  mouse_state();
  get_mouse_location();
  h = open_win[show_win_num - 1];

  /* a drag starts only with a press that is made on the top row */
  if (ms == PRESSED)
  {
    if (!on_win_top(h, mouse_x, mouse_y)) drag_ok = 0;
  }
  if (ms == RELEASED || ms == PRESSEDRELEASED) drag_ok = 1;

  if (menu_move_lock != 0)  return -1;
  if (lp != 1)              return -1;
  if (drag_ok != 1)         return -1;
  if (!on_win_top(h, mouse_x, mouse_y)) return -1;

  ox  = mouse_x;
  oy  = mouse_y;
  nx  = xwin[h];
  ny  = ywin[h];
  nxs = xwinsize[h];
  nys = ywinsize[h];

  /* the window must stay on the screen */
  limit_mouse(mouse_x - xwin[h],
              mouse_y - ywin[h] + 1,
              txt_scr_xsize - (xwin[h] + xwinsize[h] - mouse_x),
              txt_scr_ysize - (ywin[h] + ywinsize[h] - mouse_y));

  tbuf = (unsigned char *) malloc(xwinsize[h] * ywinsize[h] * 2);
  if (tbuf == NULL)
  {
    gui_fatal("\n cannot allocate memory to move the text window");
  }

  if (font_width > 1)
  {
    size = get_block_size(0, 0, xwinsize[h] * font_width, ywinsize[h] * font_height);
    gbuf = (char GUI_FAR *) gui_farmalloc(size);
    if (gbuf == NULL)
    {
      gui_fatal("\n cannot allocate memory to move the graphics window");
    }

    /* the window's own picture goes with it */
    save_cells(tbuf, xwin[h], ywin[h], xwinsize[h], ywinsize[h]);

    hide_mouse_cursor();
    GET_IMAGE(xwin[h] * font_width, ywin[h] * font_height,
              (xwin[h] + xwinsize[h]) * font_width - 1,
              (ywin[h] + ywinsize[h]) * font_height, gbuf);
    show_mouse_cursor();

    drag_outline(nx, ny, nxs, nys);
  }

  while (mouse_left_pressed() == 1)
  {
    get_mouse_location();
    if (mouse_x == ox && mouse_y == oy) continue;

    if (font_width > 1)
    {
      drag_outline(nx, ny, nxs, nys);
      nx += mouse_x - ox;
      ny += mouse_y - oy;
      drag_outline(nx, ny, nxs, nys);
    }
    else
    {
      /* text mode: the window itself follows the mouse */
      if (winshadow[h] != 0)
      {
        h2 = ywinsize[h] - 1;
        w2 = xwinsize[h] - 2;
      }
      else
      {
        h2 = ywinsize[h];
        w2 = xwinsize[h];
      }
      save_cells(tbuf, xwin[h], ywin[h], w2, h2);

      hide_win(h);
      xwin[h] += mouse_x - ox;
      ywin[h] += mouse_y - oy;
      show_win(h);

      hide_mouse_cursor();
      restore_cells(tbuf, xwin[h], ywin[h], w2, h2);
      show_mouse_cursor();
    }

    ox = mouse_x;
    oy = mouse_y;
  }

  limit_mouse(0, 0, txt_scr_xsize - 1, txt_scr_ysize - 1);
  ms = RELEASED;

  if (font_width > 1)
  {
    drag_outline(nx, ny, nxs, nys);

    hide_win(h);
    xwin[h] = nx;
    ywin[h] = ny;
    show_win(h);

    hide_mouse_cursor();
    RESTORE_IMAGE(gbuf, xwin[h] * font_width, ywin[h] * font_height);

    text_show = 0;
    restore_cells(tbuf, xwin[h], ywin[h], xwinsize[h], ywinsize[h]);
    text_show = 1;
    show_mouse_cursor();

    gui_farfree(gbuf);
  }

  free(tbuf);
  return -2;
}
/*--------------------------------------------------------------------------*/
