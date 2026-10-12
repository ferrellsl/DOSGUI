/*--------------------------------------------------------------------------*/
/* BUTTON : push buttons, and choosing them with TAB (alt_control)          */
/*                                                                          */
/* Rebuilt from the Plain Design GUI library ver 1.01 (BC_GUI.LIB, module   */
/* BUTTON).                                                                 */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "gui.h"
#include "gport.h"

int b_backcolor  = 7;
int b_border1    = 0;
int b_border2    = 8;
int b_border3    = 15;
int b_textcolor  = 0x70;
int b_blinkcolor = 4;
int b_shadow     = 0x18;

int on_off_sw;
int num_buttons;
int button_acc    [MAX_BUTTONS];
int button_id     [MAX_BUTTONS];
int button_ysize  [MAX_BUTTONS];
int button_xsize  [MAX_BUTTONS];
int button_y      [MAX_BUTTONS];
int button_x      [MAX_BUTTONS];
int button_sw     [MAX_BUTTONS];     /* 1: active                          */
int button_handle [MAX_BUTTONS];     /* -1: free                           */

static int num_fields;               /* fields that share TAB with buttons */
static int tab_button = -1;          /* the button TAB has chosen          */
static int tab_on;                   /* TAB is among the buttons           */
static int tab_control;              /* buttons have TAB (CONTROL)         */
static int outlined   = -1;          /* the button with the outline        */
static int ox1, oy1, ox2, oy2;       /* and the outline                    */

int select_button(int action, int n);

/*--------------------------------------------------------------------------*/
int init_buttons(void)
{
  int i;

  for (i = 0; i < MAX_BUTTONS; i++)
  {
    button_handle[i] = -1;
    button_sw[i]     = 1;
  }
  num_buttons = 0;
  return 0;
}
/*--------------------------------------------------------------------------*/
/* Draw a button at location XY with size xsize, ysize.                     */
/* x,y are text coordinates, or 1000 + graphics coordinates;                */
/* a ^ in the string underlines the letter after it                         */
/*--------------------------------------------------------------------------*/
int draw_button(char *bstring, int x, int y, int xsize, int ysize, int id, int acc)
{
  char name[80];
  int  h   = -1;
  int  j   = 0;
  int  hot = -1;
  int  len = (int) strlen(bstring);
  int  i, gx = x, gy = y, tx, ty, ly;

  memset(name, 0, sizeof(name));

  num_buttons++;
  assert(num_buttons < MAX_BUTTONS);

  for (i = 0; (unsigned) i < strlen(bstring); i++)
  {
    if (bstring[i] == '^')
    {
      hot = i;
      len = (int) strlen(bstring) - 1;
    }
    else
    {
      name[j++] = bstring[i];
      assert(j < 80);
    }
  }

  for (i = 0; i < MAX_BUTTONS; i++)
  {
    if (button_handle[i] == -1)
    {
      button_handle[i] = 0;
      button_sw[i]     = 1;
      h = i;
      break;
    }
  }
  if (h == -1) return -1;

  if (font_width > 1)
  {
    gx = (x > 1000) ? x - 1000 : x * font_width;
    gy = (y > 1000) ? y - 1000 : y * font_height;

    gfillrect(gx, gy, gx + xsize - 1, gy + ysize - 1, b_backcolor);
    grect(gx, gy, gx + xsize - 1, gy + ysize - 1, b_border1);
    grect(gx + 1, gy + 1, gx + xsize - 2, gy + ysize - 2, b_border3);
    grect(gx + 2, gy + 2, gx + xsize - 3, gy + ysize - 3, b_border3);
    hide_mouse_cursor();

    /* the dark right and bottom edges */
    gr_line(gx + xsize - 2, gy,     gx + xsize - 2, gy + ysize - 2, b_border2);
    gr_line(gx + xsize - 3, gy + 2, gx + xsize - 3, gy + ysize - 3, b_border2);
    gr_line(gx + 1, gy + ysize - 2, gx + xsize - 2, gy + ysize - 2, b_border2);
    gr_line(gx + 2, gy + ysize - 3, gx + xsize - 3, gy + ysize - 3, b_border2);
    hide_mouse_cursor();

    tx = gx + (xsize - 6 - len * 8) / 2 + 4;
    ty = gy + (ysize - 16) / 2 + 1;
    gpls(tx, ty, (unsigned char *) name, b_textcolor);

    if (hot != -1)
    {
      hide_mouse_cursor();
      /* under the letter; lower for the ones with a tail */
      if (name[hot] == 'q' || name[hot] == 'p' || name[hot] == 'j') ly = gy + (ysize - 16) / 2 + 17;
      else                                                         ly = gy + (ysize - 16) / 2 + 14;
      gr_line(tx + hot * font_width, ly, tx + hot * font_width + 6, ly, b_textcolor & 0xf);
      show_mouse_cursor();
    }

    gr_line(gx + 2, gy + ysize - 3, gx + xsize - 3, gy + ysize - 3, b_border2);
  }
  else
  {
    pls(x, y, (unsigned char *) name, b_backcolor);
  }

  button_id[h]  = id;
  button_acc[h] = acc;
  button_x[h]   = gx;
  button_y[h]   = gy;

  if (font_width > 1)
  {
    button_xsize[h] = xsize;
    button_ysize[h] = ysize;
  }
  else
  {
    button_xsize[h] = (int) strlen(name);
    button_ysize[h] = 1;
  }

  return h;
}
/*--------------------------------------------------------------------------*/
/* Disable a button                                                         */
/*--------------------------------------------------------------------------*/
void disable_button(int h)
{
  if (h < 0 || h >= MAX_BUTTONS) return;
  if (button_handle[h] == -1) return;

  button_handle[h] = -1;
  num_buttons--;

  if (num_buttons == 0) alt_control(INIT, 0);
  if (ms == PRESSEDRELEASED) ms = RELEASED;
}
/*--------------------------------------------------------------------------*/
/* Disable a button and paint the background color                          */
/*--------------------------------------------------------------------------*/
void disable_button2(int h, int color)
{
  if (h < 0 || h >= MAX_BUTTONS) return;
  if (button_handle[h] == -1) return;

  gfillrect(button_x[h], button_y[h],
            button_x[h] + button_xsize[h] - 1, button_y[h] + button_ysize[h] - 1, color);
  disable_button(h);
}
/*--------------------------------------------------------------------------*/
static int in_button(int i, int x, int y)
{
  return button_x[i] <= x && x < button_x[i] + button_xsize[i] &&
         button_y[i] <= y && y < button_y[i] + button_ysize[i];
}

static void where_is_mouse(int *x, int *y)
{
  if (font_width == 1)
  {
    get_mouse_location();
    *x = mouse_x * font_width;
    *y = mouse_y * font_height;
  }
  else
  {
    get_ml(x, y);
  }
}

static void blink_button(int i, int color)
{
  if (font_width > 1)
  {
    xor_frect(button_x[i] + 3, button_y[i] + 3,
              button_x[i] + button_xsize[i] - 4, button_y[i] + button_ysize[i] - 4, b_blinkcolor);
  }
  else
  {
    chcolor(button_x[i], button_y[i], button_xsize[i], button_ysize[i], color);
  }
}
/*--------------------------------------------------------------------------*/
/* Check button activity, If nothing return ch else it returns the button id */
/*--------------------------------------------------------------------------*/
int check_button(int ch)
{
  int lp;
  int mx = 0, my = 0;
  int i, ret;

  if (ch == -2)
  {
    alt_control(ACTION, -2);
    return -1;
  }

  ch = alt_control(ACTION, ch);

  lp = mouse_left_pressed();
  if (lp == 0 && ch == -1) return -1;

  if (lp == 1)
  {
    where_is_mouse(&mx, &my);
    alt_control(ACTION, -2);
  }

  for (i = 0; i < MAX_BUTTONS; i++)
  {
    if (button_handle[i] == -1 || button_sw[i] == 0) continue;

    if (lp == 1)
    {
      if (in_button(i, mx, my))
      {
        /* pressed in while the mouse button is down; it counts if the    */
        /* mouse is still on the button when it is let go                 */
        blink_button(i, b_blinkcolor);
        ret = button_id[i];

        while (mouse_left_pressed() == 1)
        {
          gcurs(MOVE);
          where_is_mouse(&mx, &my);
          if (!in_button(i, mx, my))
          {
            ret = -1;
            break;
          }
        }

        blink_button(i, b_textcolor);
        return ret;
      }
    }
    else if (button_acc[i] == ch)
    {
      return button_id[i];
    }
  }

  return ch;
}
/*--------------------------------------------------------------------------*/
void set_bshadow(int shadow) { b_shadow = shadow; }
int  get_bshadow(void)       { return b_shadow; }
/*--------------------------------------------------------------------------*/
/* deactivate or reactivate displayed buttons                               */
/*--------------------------------------------------------------------------*/
void deactivate_activebuttons(void)
{
  int i;

  for (i = 0; i < MAX_BUTTONS; i++)
  {
    if (button_sw[i] == 1 && button_handle[i] >= 0)
    {
      button_sw[i] = 0;
      num_buttons--;
    }
  }
}

void reactivate_activebuttons(void)
{
  int i;

  for (i = 0; i < MAX_BUTTONS; i++)
  {
    if (button_sw[i] == 0 && button_handle[i] >= 0)
    {
      button_sw[i] = 1;
      num_buttons++;
    }
  }
}
/*--------------------------------------------------------------------------*/
/* which button is the n-th active one                                      */
/*--------------------------------------------------------------------------*/
static int nth_button(int n)
{
  int i, count = -1;

  for (i = 0; i < MAX_BUTTONS; i++)
  {
    if (button_handle[i] != -1 && button_sw[i] == 1) count++;
    if (count == n) break;
  }
  return i;
}
/*--------------------------------------------------------------------------*/
/* TAB among buttons (and fields).                                          */
/*   INIT            forget everything                                      */
/*   NEW_FIELD       one more field shares TAB with the buttons             */
/*   SET_NUM_FIELDS  ch fields do                                           */
/*   ACTION          ch is a key: TAB and SHIFTTAB step through the active  */
/*                   buttons (returns -1 while one is chosen, 0 when the    */
/*                   last one is left), ENTER returns the chosen button's   */
/*                   id, CONTROL gives TAB to the buttons, -2 lets go       */
/*   returns ch if it was not used                                          */
/*--------------------------------------------------------------------------*/
int alt_control(int action, int ch)
{
  int i;

  switch (action)
  {
    case INIT:
     num_fields  = 0;
     tab_button  = -1;
     tab_on      = 0;
     tab_control = 0;
     break;

    case NEW_FIELD:
     num_fields++;
     break;

    case SET_NUM_FIELDS:
     num_fields = ch;
     break;

    case ACTION:
     switch (ch)
     {
       case CONTROL:
        if (tab_control == 0) tab_control = 1;
        break;

       case -2:
        if (tab_on == 1)
        {
          select_button(0, tab_button);
          tab_button  = -1;
          tab_on      = 0;
          tab_control = 0;
          ch = 0;
        }
        break;

       case ENTER:
        if (tab_on == 1)
        {
          select_button(0, tab_button);
          i = nth_button(tab_button);
          tab_button  = -1;
          tab_on      = 0;
          tab_control = 0;
          if (i >= MAX_BUTTONS) return ch;
          return button_id[i];
        }
        break;

       case TAB:
        if (num_fields > 1 && tab_control == 0) break;
        ch = -1;
        if (tab_on == 0 && num_buttons > 0)
        {
          tab_on     = 1;
          tab_button = 0;
          select_button(1, 0);
        }
        else if (tab_on == 1 && num_buttons > 0)
        {
          select_button(0, tab_button);
          tab_button++;
          if (tab_button >= num_buttons)
          {
            tab_button = -1;
            tab_on     = 0;
            if (tab_control == 1) tab_control = 0;
            ch = 0;
          }
          else
          {
            select_button(1, tab_button);
          }
        }
        break;

       case SHIFTTAB:
        if (num_fields > 1 && tab_control == 0) break;
        ch = -1;
        if (tab_on == 0 && num_buttons > 0)
        {
          tab_on     = 1;
          tab_button = num_buttons - 1;
          select_button(1, tab_button);
        }
        else if (tab_on == 1 && num_buttons > 0)
        {
          select_button(0, tab_button);
          tab_button--;
          if (tab_button < 0)
          {
            tab_button = -1;
            tab_on     = 0;
            if (tab_control == 1) tab_control = 0;
            ch = 0;
          }
          else
          {
            select_button(1, tab_button);
          }
        }
        break;
     }
     break;
  }

  return ch;
}
/*--------------------------------------------------------------------------*/
/* put (1) or take away (0) the outline that shows the n-th active button   */
/* is chosen; the outline is XORed, so drawing it again removes it          */
/*--------------------------------------------------------------------------*/
int select_button(int action, int n)
{
  int i = nth_button(n);

  if (i >= MAX_BUTTONS) return -1;

  switch (action)
  {
    case 1:
     ox1 = button_x[i] + 4;
     oy1 = button_y[i] + 4;
     ox2 = button_x[i] + button_xsize[i] - 5;
     oy2 = button_y[i] + button_ysize[i] - 5;
     outlined = i;
     break;

    case 0:
     outlined = -1;
     break;

    case -1:
     if (outlined == -1) return -1;
     on_off_sw = 0;
     xor_rect(1, ox1, oy1, ox2, oy2, 2);
     outlined = -1;
     break;
  }

  xor_rect(1, button_x[i] + 4, button_y[i] + 4,
           button_x[i] + button_xsize[i] - 5, button_y[i] + button_ysize[i] - 5, 2);
  return 0;
}
/*--------------------------------------------------------------------------*/
