/*--------------------------------------------------------------------------*/
/* SCR_BAR : a vertical scroll bar that sets a number between 0 and maxnum  */
/*                                                                          */
/* Rebuilt from the Plain Design GUI library ver 1.01 (BC_GUI.LIB, module   */
/* SCR_BAR).  One scroll bar at a time: its state is kept here.             */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include "gui.h"
#include "gport.h"

static int bw;                    /* the size of the square buttons         */
static int by;                    /* where the sliding button is            */
static int top;                   /* the track: from under the up arrow     */
static int bot;                   /* to above the down arrow                */
static int last;                  /* the number that is shown               */
static int active;

int  show_button(int action, int x1, int y1, int x2, int y2, int color);
void cms_frect(int x1, int y1, int x2, int y2);
int  calc_ypos(int num, int maxnum, int from, int to);
int  direct_button_move(int x1, int y1, int x2, int y2, int y, int size);

/*--------------------------------------------------------------------------*/
static void frame(int x1, int y1, int x2, int y2, int border, int back, int arrows)
{
  gfillrect(x1, y1, x2, y2, back);
  grect(x1, y1, x2, y2, border);

  bw = x2 - x1 - 1;
  cms_frect(x1, y1, x2, y1 + bw);
  cms_frect(x1, y2 - bw, x2, y2);

  gwritechar2(x1 + (bw - 8) / 2 + 1, y1 + (bw - 16) / 2 + 1, 0x18, arrows);
  gwritechar2(x1 + (bw - 8) / 2 + 1, y2 - (bw - 16) / 2 - 15, 0x19, arrows);
}

static void place_button(int x1, int x2, int maxnum, int num, int color)
{
  by = calc_ypos(num, maxnum, top, bot - bw);
  show_button(1, x1 + 1, by, x2 - 1, by + bw, color);
}
/*--------------------------------------------------------------------------*/
/* action : INIT    draw it                                                 */
/*          ACTION  ch is a key (or EOF); the mouse is looked at; *num is   */
/*                  changed and the bar follows.  Returns ch (UPKEY or      */
/*                  DOWNKEY if an arrow was clicked)                        */
/*          0       draw it as not in use                                   */
/* key_sw : -1 the arrows do not change *num                                */
/*--------------------------------------------------------------------------*/
int scr_bar(int action, int ch, int maxnum, int *num,
            int x1, int y1, int x2, int y2,
            int border_color, int bar_color, int key_sw)
{
  int mx, my;
  int r;

  if (action == 0)    active = 0;
  if (action == INIT) active = 1;

  if (action == 0)
  {
    frame(x1, y1, x2, y2, border_color, 8, 8);
    return ch;
  }

  if (action == INIT)
  {
    frame(x1, y1, x2, y2, border_color, bar_color, border_color);

    top  = y1 + bw + 1;
    bot  = y2 - bw - 1;
    last = *num;

    show_button(-1, 0, 0, 0, 0, 0);
    place_button(x1, x2, maxnum, *num, bar_color);
    return -1;
  }

  if (active == 0) return ch;

  if (*num != last)
  {
    /* the number was changed by the program */
    if (*num > maxnum)
    {
      printf("\a\n num=%d was greater than maxnum=%d from scrbar", *num, maxnum);
      exit(-1);
    }
    place_button(x1, x2, maxnum, *num, bar_color);
  }

  gcurs(MOVE);

  if (mouse_left_pressed() == 1)
  {
    get_ml(&mx, &my);

    if (mx >= x1 && my >= y1 && mx <= x2 && my <= top)
    {
      ch = UPKEY;                                   /* the up arrow        */
    }
    else if (mx >= x1 && my >= bot && mx <= x2 && my <= y2)
    {
      ch = DOWNKEY;                                 /* the down arrow      */
    }
    else if (mx >= x1 && my >= by && mx <= x2 && by + bw >= my)
    {
      /* the sliding button is dragged */
      r = direct_button_move(x1, top, x2, bot, by, bw);
      if (r != -1)
      {
        by = r;
        if (bot - bw - top != 0)
        {
          *num = (int) ((long) (by - top) * maxnum / (bot - bw - top));
        }
        gcurs(0);
        place_button(x1, x2, maxnum, *num, bar_color);
        gcurs(1);
      }
    }
    else if (mx >= x1 && my > top && mx <= x2 && my < bot)
    {
      /* the track above or below the button */
      if (by + 2 > my)            ch = UPKEY;
      else if (by + bw + 2 < my)  ch = DOWNKEY;
    }
  }

  switch (ch)
  {
    case DOWNKEY:
     if (key_sw == -1)    break;
     if (*num == maxnum)  break;
     (*num)++;
     place_button(x1, x2, maxnum, *num, bar_color);
     break;

    case UPKEY:
     if (key_sw == -1)    break;
     if (*num == 0)       break;
     (*num)--;
     place_button(x1, x2, maxnum, *num, bar_color);
     break;
  }

  last = *num;
  return ch;
}
/*--------------------------------------------------------------------------*/
/* the sliding button: -1 forgets where it was, 1 moves it                  */
/*--------------------------------------------------------------------------*/
int show_button(int action, int x1, int y1, int x2, int y2, int color)
{
  static int shown;
  static int sx1, sy1, sx2, sy2;

  switch (action)
  {
    case -1:
     shown = 0;
     break;

    case 1:
     gcurs(0);
     if (shown == 1)
     {
       cgfillrect(sx1, sy1, sx2, sy2, color);
     }
     cms_frect(x1, y1, x2, y2);
     sx1 = x1;
     sy1 = y1;
     sx2 = x2;
     sy2 = y2;
     shown = 1;
     gcurs(1);
     break;
  }
  return 0;
}
/*--------------------------------------------------------------------------*/
/* a raised gray square                                                     */
/*--------------------------------------------------------------------------*/
void cms_frect(int x1, int y1, int x2, int y2)
{
  cgfillrect(x1, y1, x2, y2, 7);
  cgrect(x1, y1, x2, y2, 0);
  cgrect(x1 + 1, y1 + 1, x2 - 1, y2 - 1, 15);
  cgrect(x1 + 2, y1 + 2, x2 - 2, y2 - 2, 15);
  cgr_line(x1 + 1, y2 - 1, x2 - 1, y2 - 1, 8);
  cgr_line(x2 - 1, y1 + 1, x2 - 1, y2 - 1, 8);
  cgr_line(x1 + 2, y2 - 2, x2 - 2, y2 - 2, 8);
  cgr_line(x2 - 2, y1 + 2, x2 - 2, y2 - 2, 8);
}
/*--------------------------------------------------------------------------*/
int calc_ypos(int num, int maxnum, int from, int to)
{
  if (maxnum == 0) return from;
  return (int) ((long) num * (to - from) / maxnum) + from;
}
/*--------------------------------------------------------------------------*/
/* the button follows the mouse as an outline while its key is held.        */
/* returns the new place, or -1 if the mouse was let go off the bar         */
/*--------------------------------------------------------------------------*/
int direct_button_move(int x1, int y1, int x2, int y2, int y, int size)
{
  int color = 3;
  int start = y;
  int ox, oy;
  int mx, my;

  xor_rect(1, x1 + 1, y, x2 - 1, y + size, color);
  ox = oy = 0;
  get_ml(&ox, &oy);
  mx = ox;
  my = oy;

  while (mouse_left_pressed() == 1)
  {
    gcurs(MOVE);
    get_ml(&mx, &my);

    if (my != oy && mx >= x1 && mx <= x2)
    {
      xor_rect(1, x1 + 1, y, x2 - 1, y + size, color);
      y = my - size / 2;
      if (y < y1)              y = y1;
      else if (y2 - size < y)  y = y2 - size;
      xor_rect(1, x1 + 1, y, x2 - 1, y + size, color);
      oy = my;
    }
    else if ((mx < x1 || mx > x2) && y != start)
    {
      /* off the bar: back to where it was */
      xor_rect(1, x1 + 1, y, x2 - 1, y + size, color);
      y = start;
      xor_rect(1, x1 + 1, y, x2 - 1, y + size, color);
    }
  }

  xor_rect(1, x1 + 1, y, x2 - 1, y + size, color);

  if (mx >= x1 && mx <= x2 && my >= y1 && my <= y2) return y;
  return -1;
}
/*--------------------------------------------------------------------------*/
