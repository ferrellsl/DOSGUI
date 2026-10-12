/*--------------------------------------------------------------------------*/
/* FIELDS : entry fields - a line of text, or a field that is set or not    */
/*                                                                          */
/* Rebuilt from the Plain Design GUI library ver 1.01 (BC_GUI.LIB, module   */
/* FIELDS).                                                                 */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gui.h"
#include "gport.h"

int  soft_return;                /* when on field will do soft returns     */
char soft_word[80];              /* store the word for soft return         */

int  process_windows(void);
void set_binary_field(struct bfield *f, int h, int sw);
int  is_date_valid(char *s, int *pos);
int  is_time_valid(char *s, int *pos);
void format_tel(char *s);

/*--------------------------------------------------------------------------*/
/* the field's content is what is on the screen                             */
/*--------------------------------------------------------------------------*/
static void read_field(struct xfield *f)
{
  int i;

  for (i = 0; i < f->length; i++)
  {
    f->f[i] = (char) readchar(f->x + i, f->y);
  }
  if (f->length < (int) sizeof(f->f)) f->f[f->length] = 0;
}

static void show_field(struct xfield *f)
{
  pls(f->x, f->y, (unsigned char *) f->f, f->fcolor);
}

/* check a date, a time or a telephone number and show it as it should be; */
/* returns -1 if it is wrong, with *pos the place of the mistake            */
static int check_field(struct xfield *f, int *pos)
{
  if (f->type == F_DATE)
  {
    if (is_date_valid(f->f, pos) == -1) return -1;
    txt_curs(0);
    show_field(f);
  }
  if (f->type == F_TIME)
  {
    if (is_time_valid(f->f, pos) == -1) return -1;
    txt_curs(0);
    show_field(f);
  }
  if (f->type == F_TEL)
  {
    format_tel(f->f);
    txt_curs(0);
    show_field(f);
  }
  return 0;
}
/*--------------------------------------------------------------------------*/
/*                              TEXT  FIELD                                 */
/* action : INIT  show the field and its title                              */
/*          -2    edit: returns the key that ended it (ENTER, ESC, TAB, an  */
/*                arrow at the end of the field ...), or EOF if the mouse   */
/*                was clicked somewhere else                                */
/*          other show the field, then edit                                 */
/*--------------------------------------------------------------------------*/
int text_field(struct xfield *f, int action)
{
  char save[80];
  int  key;
  int  cx;                        /* the cursor's column                    */
  int  last;                      /* the field's last column                */
  int  pos = 0;
  int  i, c;

  memset(save, 0, sizeof(save));
  if (f->type == F_DATE)
  {
    strncpy(save, f->f, f->length);
  }

  if (action != -2)
  {
    hide_mouse_cursor();
    for (i = 0; i < f->length; i++)
    {
      writechar(f->x + i, f->y, ' ', f->fcolor);
    }
    show_field(f);
    pls(f->x, f->y - 1, (unsigned char *) f->title, f->tcolor);

    if (action == INIT)
    {
      show_mouse_cursor();
      alt_control(NEW_FIELD, 0);
      return -1;
    }
    show_mouse_cursor();
  }

  last = f->x + f->length - 1;
  cx   = f->x + f->xstart;

  txt_curs(1);
  txt_curs_x = cx;
  txt_curs_y = f->y;
  txt_curs(5);

  for (;;)
  {
    key = getkey();
    txt_curs(8);
    gcurs(MOVE);

    if (mouse_state() == PRESSED)
    {
      get_mouse_location();
      if (f->y == mouse_y && f->x <= mouse_x && mouse_x < f->x + f->length)
      {
        /* a click in the field puts the cursor there */
        cx = mouse_x;
        txt_curs_x = cx;
        txt_curs_y = f->y;
        txt_curs(5);
      }
      else
      {
        /* a click somewhere else ends the editing */
        f->xstart = cx - f->x;
        txt_curs(0);
        read_field(f);

        if (check_field(f, &pos) == -1)
        {
          printf("\a");
          cx = f->x + pos;
          txt_curs(1);
          continue;
        }
        return -1;
      }
    }

    if ((key >= 0x2c && key <= 0x7e) || key == ' ')
    {
      /* a character */
      if (soft_return == 1 && cx == last && readchar(last, f->y) != ' ')
      {
        /* the field is full: the last word is taken out, for the next line */
        memset(soft_word, 0, sizeof(soft_word));
        i = last;
        while (f->x < i)
        {
          c = readchar(i, f->y);
          if (c == ' ' || c == 0) break;
          i--;
        }
        i++;

        txt_curs(0);
        for (c = i; c <= last; c++)
        {
          soft_word[c - i] = (char) readchar(c, f->y);
          writechar(c, f->y, ' ', f->fcolor);
        }
        soft_word[strlen(soft_word)] = (char) key;
        key = -100;
      }
      else
      {
        txt_curs(0);
        c = readchar(last, f->y);
        gcurs(0);

        if (c == ' ' && f->type != F_TIME)
        {
          /* room at the end: what follows the cursor moves right */
          for (i = last; i != cx; i--)
          {
            writechar(i, f->y, readchar(i - 1, f->y), f->fcolor);
          }
        }
        writechar(cx, f->y, (unsigned char) key, f->fcolor);

        txt_curs(1);
        gcurs(1);

        if (cx != last)
        {
          cx++;
          txt_curs_x = cx;
          txt_curs_y = f->y;
          txt_curs(5);
        }
        continue;
      }
    }
    else
    {
      switch (key)
      {
        case RIGHTKEY:
         if (cx != last)
         {
           cx++;
           continue;
         }
         f->xstart = cx - f->x;
         read_field(f);
         txt_curs(0);
         return key;

        case LEFTKEY:
         if (cx != f->x)
         {
           cx--;
           continue;
         }
         f->xstart = cx - f->x;
         read_field(f);
         txt_curs(0);
         return key;

        case HOMEKEY:
         cx = f->x;
         continue;

        case ENDKEY:
         i = f->length - 1;
         while (i >= 0 && readchar(f->x + i, f->y) == ' ') i--;
         if (last > f->x + i + 1) cx = f->x + i + 1;
         else                     cx = f->x + i;
         if (cx < f->x) cx = f->x;
         continue;

        case DELKEY:
         if (cx == last)
         {
           writechar(cx, f->y, ' ', f->fcolor);
           break;
         }
         cx++;
         key = BS;
         /* and take out the character before the cursor */

        case BS:
         if (cx == f->x) continue;
         txt_curs(0);
         gcurs(0);
         for (i = cx - 1; i < last; i++)
         {
           writechar(i, f->y, readchar(i + 1, f->y), f->fcolor);
         }
         writechar(last, f->y, ' ', f->fcolor);
         cx--;
         txt_curs(1);
         gcurs(1);
         continue;
      }
    }

    if (key == -1)
    {
      if (txt_curs_x != cx || txt_curs_y != f->y)
      {
        txt_curs_x = cx;
        txt_curs_y = f->y;
        txt_curs(5);
      }
      continue;
    }

    /* any other key ends the editing */
    f->xstart = cx - f->x;
    read_field(f);

    if ((f->type == F_DATE || f->type == F_TIME) && key == ESC)
    {
      strncpy(f->f, save, f->length);
      show_field(f);
      txt_curs(0);
      return key;
    }

    if (check_field(f, &pos) == -1)
    {
      printf("\a");
      cx = f->x + pos;
      continue;
    }

    txt_curs(0);
    return key;
  }
}
/*--------------------------------------------------------------------------*/
/*                             BINARY  FIELD                                */
/* action : INIT  show the field and its title                              */
/*          -2    one turn of editing: returns the key, or EOF; SPACE (or a */
/*                click on the field) sets it or clears it                  */
/* h      : window handle or -1 for no window                               */
/*--------------------------------------------------------------------------*/
static int on_bfield(struct bfield *f, int h)
{
  int wx = 0, wy = 0;

  if (h != -1)
  {
    wx = xwin[h];
    wy = ywin[h];
  }
  return wy + f->y == mouse_y && f->x + wx <= mouse_x && mouse_x <= f->x + wx + 2;
}

int binary_field(struct bfield *f, int action, int h)
{
  int key;
  int wx = 0, wy = 0;

  if (h != -1)
  {
    wx = xwin[h];
    wy = ywin[h];
  }

  if (action != -2)
  {
    set_binary_field(f, h, f->content);
    plsh(f->x + 4, f->y, (unsigned char *) f->title, f->tcolor, h);
  }

  if (action == INIT)
  {
    alt_control(NEW_FIELD, 0);
    return -1;
  }

  txt_curs(1);
  txt_curs_x = wx + f->x + 1;
  txt_curs_y = wy + f->y;
  txt_curs(5);

  key = getkey();
  set_binary_field(f, h, 0x80);                   /* it blinks: it is the field in use */
  gcurs(MOVE);

  if (process_windows() == -2)
  {
    if (h != -1)
    {
      wx = xwin[h];
      wy = ywin[h];
    }
    txt_curs_x = wx + f->x + 1;
    txt_curs_y = wy + f->y;
    txt_curs(5);
  }

  mouse_state();
  if (ms == PRESSED)
  {
    get_mouse_location();
    if (!on_bfield(f, h))
    {
      txt_curs(0);
      set_binary_field(f, h, 3);
      return -1;
    }
  }
  if (ms == PRESSEDRELEASED)
  {
    ms = RELEASED;
    get_mouse_location();
    if (on_bfield(f, h)) key = ' ';
  }

  switch (key)
  {
    case ' ':
     f->content = (f->content == NO) ? YES : NO;
     set_binary_field(f, h, f->content);
     return -1;

    case TAB:
    case ENTER:
    case ESC:
    case SHIFTTAB:
    case UPKEY:
    case LEFTKEY:
    case RIGHTKEY:
    case DOWNKEY:
     txt_curs(0);
     set_binary_field(f, h, 3);
     break;
  }

  return key;
}
/*--------------------------------------------------------------------------*/
/* draw a binary field: a ring, filled if the field is set.                 */
/*   sw YES, NO  as it is set                                               */
/*      0x80     let the ring blink (called over and over)                  */
/*      3        the ring as it should be, the blinking is over             */
/*--------------------------------------------------------------------------*/
void set_binary_field(struct bfield *f, int h, int sw)
{
  static int  blink;              /* the ring is in its background color    */
  static long blink_time;
  int  wx = 0, wy = 0;
  int  cx, cy;
  int  color;
  int  mx = -1000, my = -1000;
  int  over = 1;                  /* the mouse pointer is on the ring       */
  int  r;
  long elapsed;

  if (h != -1)
  {
    wx = xwin[h];
    wy = ywin[h];
  }
  cx = (f->x + wx) * font_width + 10;
  cy = (f->y + wy) * font_height + 7;

  if (sw == 0x80 || sw == 3)
  {
    if (font_width == 1) return;
    if (sw == 3 && blink == 0) return;

    if (sw == 0x80)
    {
      elapsed = gp_ms() - blink_time;
      if (elapsed < 0) elapsed = 1000;
      if (blink == 0 && elapsed < 250) return;
      if (blink == 1 && elapsed < 120) return;
    }

    if (blink == 1)
    {
      color = f->fcolor & 0xf;
      blink = 0;
    }
    else
    {
      color = f->fcolor >> 4;
      blink = 1;
    }

    get_ml(&mx, &my);
    if (!(cx - 6 <= mx + 10 && cx + 6 >= mx && cy - 6 <= my + 16 && cy + 6 >= my))
    {
      over = 0;
    }

    if (over == 1) gcurs(0);
    ELLIPSE(cx, cy, 6, 6, color);
    if (over == 1) gcurs(1);

    blink_time = gp_ms();
    return;
  }

  blink      = 0;
  blink_time = gp_ms();

  if (font_width > 1)
  {
    gcurs(0);
    ELLIPSE(cx, cy, 6, 6, f->fcolor & 0xf);

    if (sw == 1) color = f->fcolor & 0xf;
    else         color = f->fcolor >> 4;

    for (r = 0; r < 4; r++)
    {
      ELLIPSE(cx, cy, r, r, color);
    }
    SETPIX(cx - 1, cy - 1, (char) color);
    SETPIX(cx + 1, cy - 1, (char) color);
    SETPIX(cx - 1, cy + 1, (char) color);
    SETPIX(cx + 1, cy + 1, (char) color);
    gcurs(1);
  }
  else
  {
    plsh(f->x, f->y, (unsigned char *) "[ ]", f->fcolor, h);
    if (sw == 1)
    {
      plsh(f->x + 1, f->y, (unsigned char *) "X", f->fcolor, h);
    }
  }
}
/*--------------------------------------------------------------------------*/
/* mm-dd-yy : 0 if it is a date (or empty), -1 and the place of the mistake */
/*--------------------------------------------------------------------------*/
static int two_digits(const char *s)
{
  char tmp[3];

  tmp[0] = s[0];
  tmp[1] = s[1];
  tmp[2] = 0;
  return (int) atol(tmp);
}

static int is_empty(const char *s, int n)
{
  int i;

  for (i = 0; i < n; i++)
  {
    if (s[i] == 0) break;
    if (s[i] != ' ') return 0;
  }
  return 1;
}

int is_date_valid(char *s, int *pos)
{
  int mm, dd, yy;

  if (is_empty(s, 8)) return 0;

  mm = two_digits(s);
  dd = (strlen(s) > 3) ? two_digits(s + 3) : 0;
  yy = (strlen(s) > 6) ? two_digits(s + 6) : -1;

  if (mm < 1 || mm > 12) { *pos = 0; return -1; }
  if (dd < 1 || dd > 31) { *pos = 3; return -1; }
  if (yy < 0 || yy > 99) { *pos = 6; return -1; }

  sprintf(s, "%02d-%02d-%02d", mm, dd, yy);
  return 0;
}
/*--------------------------------------------------------------------------*/
/* hh:mm, with AM or PM after it or not                                     */
/*--------------------------------------------------------------------------*/
int is_time_valid(char *s, int *pos)
{
  char ap[3];
  int  hh, mi;

  if (is_empty(s, 7)) return 0;

  hh = two_digits(s);
  mi = (strlen(s) > 3) ? two_digits(s + 3) : -1;

  if (hh < 0 || hh > 23) { *pos = 0; return -1; }
  if (mi < 0 || mi > 59) { *pos = 3; return -1; }

  memset(ap, 0, sizeof(ap));
  if (strlen(s) > 5) strncpy(ap, s + 5, 2);

  if (ap[0] != 0 && stricmp(ap, "  ") != 0 && stricmp(ap, " ") != 0 &&
      stricmp(ap, "AM") != 0 && stricmp(ap, "PM") != 0)
  {
    *pos = 5;
    return -1;
  }

  sprintf(s, "%02d:%02d%s", hh, mi, ap);
  return 0;
}
/*--------------------------------------------------------------------------*/
/* 7 digits become 555-1234, 10 digits (201) 555-1234                       */
/*--------------------------------------------------------------------------*/
void format_tel(char *s)
{
  char a[4], b[4], c[5];
  int  n;

  memset(a, 0, sizeof(a));
  memset(b, 0, sizeof(b));
  memset(c, 0, sizeof(c));

  for (n = 0; n < 10; n++)
  {
    if (s[n] < '0' || s[n] > '9') break;
  }

  if (n == 7)
  {
    strncpy(b, s, 3);
    strncpy(c, s + 3, 4);
    sprintf(s, "%3s-%4s", b, c);
  }
  else if (n == 10)
  {
    strncpy(a, s, 3);
    strncpy(b, s + 3, 3);
    strncpy(c, s + 6, 4);
    sprintf(s, "(%3s) %3s-%4s", a, b, c);
  }
}
/*--------------------------------------------------------------------------*/
