/*--------------------------------------------------------------------------*/
/* HELP : the hyper text help window                                        */
/*                                                                          */
/* Rebuilt from the Plain Design GUI library ver 1.01 (BC_GUI.LIB, module   */
/* HELP).                                                                   */
/*                                                                          */
/* The help file: its first line is the number of lines in it; a topic      */
/* starts with a line "$Topic,name"; a name between two ^ in the text is a  */
/* link to that topic (\^ is a ^ that is not one).                          */
/*                                                                          */
/* The window was made for a 640 x 480 screen; on a larger one it is the    */
/* same size, in the middle.                                                */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gui.h"
#include "gport.h"

#define MAX_LINKS   30            /* links on one screen of text            */
#define MAX_HISTORY 40            /* topics to go Back through              */
#define TEXT_ROWS   19
#define TEXT_COLS   67

#define ID_BACK     98
#define ID_CONTENTS 99
#define ID_DONE     100

static int tx0, ty0;              /* where the window is, in text cells     */
static int gx0, gy0;              /* and in pixels                          */

#define GX(x)  ((x) + gx0)
#define GY(y)  ((y) + gy0)
#define TX(x)  ((x) + tx0)
#define TY(y)  ((y) + ty0)
#define ROW(y) (TY(y) * font_height)      /* the top of text row y          */

void process_text(char *helpfile, char *topic);
void show_text(FILE *f, int line, int *nlinks, int *lx, int *ly, int *llen);
int  process_line(char *buf, int *nlinks, int *lx, int *ly, int *llen, int x, int y);
int  get_line_for_topic(char *topic, FILE *f, int *line);
void ms_frect(int x1, int y1, int x2, int y2);
void flush_kbbuf(void);

/*--------------------------------------------------------------------------*/
/* title          : is the displayed title on the help window.              */
/* helpfile       : Hyper text help file.                                   */
/* starting_topic : The topic to be shown on the startup screen.            */
/*--------------------------------------------------------------------------*/
void help(char *title, char *helpfile, char *starting_topic)
{
  int w1, w2;
  int b1, b2, b3;
  int i;

  tx0 = (txt_scr_xsize - 80) / 2;
  ty0 = (txt_scr_ysize - 30) / 2;
  if (tx0 < 0) tx0 = 0;
  if (ty0 < 0) ty0 = 0;
  gx0 = tx0 * font_width;
  gy0 = ty0 * font_height;

  deactivate_activebuttons();

  /* two windows side by side keep what the help window covers */
  w1 = messagebox(TX(4),  TY(5), 35, 23, 0xff, 0, 1);
  w2 = messagebox(TX(39), TY(5), 36, 23, 0xff, 0, 1);

  /* the frame */
  grect(GX(0x20), GY(0x50), GX(0x258), GY(0x1c0), 0);
  for (i = 0; i <= 2; i++)
  {
    grect(GX(0x21 + i), GY(0x51 + i), GX(0x257 - i), GY(0x1bf - i), 8);
  }
  grect(GX(0x24), GY(0x54), GX(0x254), GY(0x1bc), 0);

  /* the title bar, with the box that closes the window */
  gfillrect(GX(0x25), GY(0x55), GX(0x253), GY(0x66), 4);
  gr_line(GX(0x25), GY(0x67), GX(0x253), GY(0x67), 0);
  gpls2(GX(0x140) - (int) strlen(title) * 4, GY(0x57), (unsigned char *) title, 15);
  ms_frect(GX(0x24), GY(0x54), GX(0x37), GY(0x67));
  ms_frect(GX(0x29), GY(0x59), GX(0x31), GY(0x61));

  /* the row of buttons */
  gfillrect(GX(0x25), GY(0x68), GX(0x253), GY(0x7d), 7);
  gr_line(GX(0x25), GY(0x7d), GX(0x253), GY(0x7d), 0);

  b1 = draw_button("^Contents", 1000 + GX(37),  1000 + GY(104), 78, 21, ID_CONTENTS, ALTC);
  b2 = draw_button("^Back",     1000 + GX(115), 1000 + GY(104), 78, 21, ID_BACK,     ALTB);
  b3 = draw_button("^Done",     1000 + GX(193), 1000 + GY(104), 78, 21, ID_DONE,     ALTD);

  /* TAB goes through the links first, then the buttons */
  alt_control(INIT, 0);
  alt_control(NEW_FIELD, 0);
  alt_control(NEW_FIELD, 0);

  process_text(helpfile, starting_topic);

  disable_button(b1);
  disable_button(b2);
  disable_button(b3);

  destroy_messagebox(w1);
  destroy_messagebox(w2);

  reactivate_activebuttons();
}
/*--------------------------------------------------------------------------*/
/* read the text of a link from the screen and go to its topic              */
/*--------------------------------------------------------------------------*/
static char history[MAX_HISTORY][40];
static int  history_num;

static void follow_link(FILE *f, int x, int y, int len, int *line)
{
  char name[100];
  int  i;

  memset(name, 0, sizeof(name));
  for (i = 0; i < len && i < 99; i++)
  {
    name[i] = (char) readchar(x + i, y);
  }

  if (get_line_for_topic(name, f, line) == -1)
  {
    printf("\a");
    return;
  }
  if (strlen(name) < 40 && history_num < MAX_HISTORY)
  {
    strcpy(history[history_num], name);
    history_num++;
  }
}
/*--------------------------------------------------------------------------*/
void process_text(char *helpfile, char *topic)
{
  FILE *f;
  char  buf[100];
  int   lx[MAX_LINKS], ly[MAX_LINKS], llen[MAX_LINKS];
  int   nlinks  = 0;
  int   done    = 0;
  int   line    = 0;              /* the first line that is shown           */
  int   oldline;
  int   total;
  int   sel_on  = 0;              /* TAB has chosen a link                  */
  int   sel     = 0;
  int   key, i, mx, my;

  /* the scroll bar */
  int sx1 = GX(0x241), sy1 = GY(0x7d), sx2 = GX(0x254), sy2 = GY(0x1bb);

  history_num = 0;
  memset(history, 0, sizeof(history));

  f = fopen(helpfile, "r");
  if (f == NULL)
  {
    sprintf(buf, "Cannot Open %s file", helpfile);
    gpls(GX(0x138) - (int) strlen(buf) * 4, GY(0x112), (unsigned char *) buf, 0xf4);
    delay_func(3000);
    return;
  }

  buf[0] = 0;
  fgets(buf, 100, f);
  total = (int) atol(buf);
  if (total < 1) total = 1;

  scr_bar(INIT, 0, total, &line, sx1, sy1, sx2, sy2, 0, 7, 0);

  get_line_for_topic(topic ? topic : "Contents", f, &line);

  show_text(f, -1, &nlinks, lx, ly, llen);
  show_text(f, line, &nlinks, lx, ly, llen);
  oldline = line;

  strncpy(history[history_num], topic ? topic : "Contents", 39);
  history_num++;

  if (ms == PRESSEDRELEASED) ms = RELEASED;

  check_button(TAB);

  while (!done)
  {
    key = getkey();
    flush_kbbuf();
    key = check_button(key);
    key = scr_bar(ACTION, key, total, &line, sx1, sy1, sx2, sy2, 0, 7, 0);

    if (line != oldline)
    {
      if (sel_on == 1)
      {
        mchcolor(lx[sel], ly[sel], llen[sel], 1, 0xf2);
        sel_on = 0;
      }
      show_text(f, line, &nlinks, lx, ly, llen);
      oldline = line;
      sel_on  = 0;
    }

    mouse_state();

    /* a double click on the box of the title bar closes the window */
    if (dc == DOUBLECLICKED)
    {
      mx = my = -1;
      get_ml(&mx, &my);
      if (mx >= GX(0x24) && mx <= GX(0x37) && my >= GY(0x54) && my <= GY(0x67))
      {
        key = ESC;
      }
    }

    /* a click on a link */
    if (ms == PRESSEDRELEASED)
    {
      ms = RELEASED;
      get_mouse_location();
      if (mouse_x >= TX(5) && mouse_x < TX(0x48) && mouse_y >= TY(8) && mouse_y <= TY(0x1a))
      {
        for (i = 0; i < nlinks; i++)
        {
          if (ly[i] == mouse_y && lx[i] <= mouse_x && mouse_x < lx[i] + llen[i])
          {
            follow_link(f, lx[i], ly[i], llen[i], &line);
            break;
          }
        }
      }
    }

    switch (key)
    {
      case ESC:
      case ID_DONE:
       done = 1;
       break;

      case ENTER:
       if (sel_on == 1 && sel >= 0 && sel < nlinks)
       {
         follow_link(f, lx[sel], ly[sel], llen[sel], &line);
       }
       break;

      case ID_BACK:
       if (history_num >= 2)
       {
         get_line_for_topic(history[history_num - 2], f, &line);
         history_num--;
       }
       break;

      case ID_CONTENTS:
       get_line_for_topic("Contents", f, &line);
       break;

      case TAB:
       if (nlinks == 0)
       {
         check_button(CONTROL);
         check_button(TAB);
       }
       else if (sel_on == 0)
       {
         sel_on = 1;
         sel    = 0;
         mchcolor(lx[0], ly[0], llen[0], 1, 0x4f);
       }
       else
       {
         mchcolor(lx[sel], ly[sel], llen[sel], 1, 0xf2);
         sel++;
         if (sel == nlinks)
         {
           sel_on = 0;
           check_button(CONTROL);
           check_button(TAB);
         }
         else
         {
           mchcolor(lx[sel], ly[sel], llen[sel], 1, 0x4f);
         }
       }
       break;
    }
  }

  fclose(f);
}
/*--------------------------------------------------------------------------*/
/* show 19 lines of the file, from line 'line' on (-1: start afresh)        */
/* the links that are shown are listed in lx, ly (where) and llen (length)  */
/*--------------------------------------------------------------------------*/
void show_text(FILE *f, int line, int *nlinks, int *lx, int *ly, int *llen)
{
  static int title_row = -1;      /* a topic's title is on this row         */
  static int title_on;
  char buf[112];
  int  eof = 0;
  int  row, x, y, i, len, color, start;

  if (line == -1)
  {
    title_row = -1;
    title_on  = 0;
    return;
  }

  rewind(f);
  fgets(buf + 1, 100, f);
  for (i = 0; i < line; i++)
  {
    fgets(buf + 1, 100, f);
  }

  if (title_on == 1)
  {
    /* take away the box around the title */
    gfillrect(GX(0x2b), title_row * font_height - 1, GX(0x23a), title_row * font_height + font_height, 15);
    title_on = 0;
  }

  *nlinks = 0;

  /* (the text is kept at buf + 1, so that the character before the first  */
  /* can be looked at)                                                     */
  buf[0] = 0;

  for (row = 0; row < TEXT_ROWS; row++)
  {
    char *text = buf + 1;

    if (eof == 0)
    {
      if (fgets(text, 100, f) == NULL)
      {
        text[0] = 0;
        eof = 1;
      }
    }

    x = TX(5);
    y = TY(row + 8);

    len = process_line(text, nlinks, lx, ly, llen, x, y);

    if (text[0] == 3)
    {
      /* the title of a topic: in a box, with a clear line above and below */
      for (i = TX(5); i < TX(0x48); i++) writechar(i, y, ' ', 0xff);
      if (y - 1 >= TY(8))
      {
        for (i = TX(5); i < TX(0x48); i++) writechar(i, y - 1, ' ', 0xff);
      }
      if (y + 1 <= TY(0x1a))
      {
        for (i = TX(5); i < TX(0x48); i++) writechar(i, y + 1, ' ', 0xff);
      }

      color = 0xe4;
      gfillrect(x * font_width + 3, y * font_height - 1,
                (x + 0x43) * font_width - 6, y * font_height + font_height, 14);
      grect(x * font_width + 3, y * font_height - 1,
            (x + 0x43) * font_width - 6, y * font_height + font_height, 0);

      start     = 2;
      x        += 2;
      title_on  = 1;
      title_row = y;
    }
    else
    {
      color = 0xf0;
      start = 0;
    }

    gcurs(0);
    for (i = start; i < TEXT_COLS; i++, x++)
    {
      if (i < len)
      {
        if (text[i] == '^' && text[i - 1] != '\\')
        {
          /* a link begins or ends: it is shown in green */
          text[i] = ' ';
          color = (color == 0xf0) ? 0xf2 : 0xf0;
        }
        if (text[i] == '\\' && text[i + 1] == '^')
        {
          writechar(x, y, ' ', color);
        }
        else
        {
          writechar(x, y, (unsigned char) text[i], color);
        }
      }
      else if (readchar(x, y) != ' ')
      {
        writechar(x, y, ' ', 0xf0);
      }
    }
    gcurs(1);
  }
}
/*--------------------------------------------------------------------------*/
/* make a line of the file ready to be shown at x,y: a topic's title is     */
/* centered (and marked with a 3 as its first character); the links in it   */
/* are added to the list.  Returns the length of the line.                  */
/*--------------------------------------------------------------------------*/
int process_line(char *buf, int *nlinks, int *lx, int *ly, int *llen, int x, int y)
{
  char tmp[100];
  int  i, in_link;

  i = (int) strlen(buf) - 1;
  while (i >= 0 && (buf[i] == '\r' || buf[i] == '\n'))
  {
    buf[i] = 0;
    i--;
  }

  if (strnicmp(buf, "$topic", 6) == 0)
  {
    memmove(buf, buf + 7, strlen(buf + 7) + 1);
    if (strlen(buf) > 60) buf[60] = 0;

    memset(tmp, 0, sizeof(tmp));
    memset(tmp, ' ', TEXT_COLS);
    strcpy(tmp + 33 - strlen(buf) / 2, buf);
    strcpy(buf, tmp);
    buf[0] = 3;
  }

  in_link = 0;
  for (i = 0; (unsigned) i < strlen(buf); i++)
  {
    if (buf[i] == '^' && (i == 0 || buf[i - 1] != '\\'))
    {
      if (*nlinks >= MAX_LINKS) break;
      if (in_link == 0)
      {
        lx[*nlinks] = i + x + 1;
        ly[*nlinks] = y;
        in_link = 1;
      }
      else
      {
        llen[*nlinks] = i + x - lx[*nlinks];
        (*nlinks)++;
        in_link = 0;
      }
    }
  }

  if (in_link == 1)
  {
    gui_fatal("\n error in Topic accelerator,  Topic must preceed ^ ");
  }

  return (int) strlen(buf);
}
/*--------------------------------------------------------------------------*/
/* where in the file is the topic: *line is the number of lines before it   */
/* (not counting the first).  Returns -1 if there is no such topic.         */
/*--------------------------------------------------------------------------*/
int get_line_for_topic(char *topic, FILE *f, int *line)
{
  char buf[100];
  int  n = -1;

  rewind(f);

  while (fgets(buf, 100, f) != NULL)
  {
    n++;
    if (buf[0] != '$') continue;
    if (strnicmp(buf, "$topic", 6) != 0) continue;
    if (strnicmp(buf + 7, topic, strlen(topic)) != 0) continue;

    *line = n - 1;
    return 0;
  }
  return -1;
}
/*--------------------------------------------------------------------------*/
/* a raised gray rectangle                                                  */
/*--------------------------------------------------------------------------*/
void ms_frect(int x1, int y1, int x2, int y2)
{
  gfillrect(x1, y1, x2, y2, 7);
  grect(x1, y1, x2, y2, 0);
  grect(x1 + 1, y1 + 1, x2 - 1, y2 - 1, 15);
  hide_mouse_cursor();
  gr_line(x1 + 1, y2 - 1, x2 - 1, y2 - 1, 8);
  gr_line(x2 - 1, y1 + 1, x2 - 1, y2 - 1, 8);
  show_mouse_cursor();
}
/*--------------------------------------------------------------------------*/
/* forget the keys that are waiting                                         */
/*--------------------------------------------------------------------------*/
void flush_kbbuf(void)
{
  while (gp_kbhit()) gp_readkey();
}
/*--------------------------------------------------------------------------*/
/* delay in milli second                                                    */
/*--------------------------------------------------------------------------*/
void delay_func(int milli)
{
  long start = gp_ms();
  long now;

  for (;;)
  {
    now = gp_ms();
    if (now < start) break;               /* (midnight) */
    if (now - start >= milli) break;
  }
}
/*--------------------------------------------------------------------------*/
