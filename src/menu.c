/*--------------------------------------------------------------------------*/
/* MENU : the pull down menu (bar_menu) and the scroll down menu (menus)    */
/*                                                                          */
/* Rebuilt from the Plain Design GUI library ver 1.01 (BC_GUI.LIB, module   */
/* MENU).  The menu is described by a text file ("MENUFILE") or by the      */
/* same lines compiled into the program (set_menu_descriptor):              */
/*                                                                          */
/*   "%Title" id acc          a title on the bar                            */
/*   "Entry" id acc           an entry under the title before it            */
/*   LINE                     a line between entries                        */
/*   SUBMENU ... END          a menu under the entry before it              */
/*                                                                          */
/* a ^ marks the hot key: in a title and in an entry it comes before the    */
/* letter (in a title the letter after the ^ is the one shown), in a        */
/* submenu's entry too.                                                     */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "gui.h"
#include "gport.h"

int   menu_descriptor = TEXT_FILE;
char  menu_file[40]   = "MENUFILE";     /* current menu file name           */
int   menu_color        = 0x70;         /* color of the menu                */
int   menu_hotkey_color = 0x74;         /* color of the menu hot keys       */
int   menu_sel_color    = 0x40;         /* color of the selected entry      */
int   menu_shadow       = 0x08;
int   menu_x            = 0;            /* menu x,y coordinates in text     */
int   menu_y            = 0;
int   menu_x_size       = 80;           /* menu xsize in text sizes         */
int   in_menu;
int   menu_move_lock;

int   menu_line_ptr;
int   menu_lines;
char **menu;
int   key_return;
int   cur_ident;
int   menu_ident[MAX_ACC];
int   menu_acc  [MAX_ACC];
int   menu_acc_num;
FILE *fp;
int   title_ptr;
int   title_num;
int   title_x    [MAX_TITLE];
int   title_acc  [MAX_TITLE];
int   title_ident[MAX_TITLE];
long  title_loc  [MAX_TITLE];
char  title_hotkey_loc[MAX_TITLE];
char  title_hotkey    [MAX_TITLE];
char  menu_title[MAX_TITLE][20];        /* menu titles array                */

#define ENTRY_LEN    50                 /* a pull down's entry              */
#define SUB_ENTRIES  15                 /* entries of a submenu             */
#define SUB_LEN      40

int  process_windows(void);
void init_menu(void);
void display_menu(void);
char seperate(char *line, char *text, int *ident, int *acc, int sw);
int  expand(int acc_sw);
void place_title_hot(int t);
int  submenu(long pos, int x, int y);
int  remove_spaces(char *s);
int  next_line(char *buffer, int size);
void set_pos(long *pos);
long get_pos(void);

/*--------------------------------------------------------------------------*/
static int is_keyword(const char *line, const char *word)
{
  return strnicmp(line, word, strlen(word)) == 0;
}

/* a line of the descriptor that means something: a keyword or "text" */
static int is_menu_line(const char *line)
{
  return is_keyword(line, "LINE") || is_keyword(line, "SUBMENU") ||
         is_keyword(line, "END")  || line[0] == '"';
}

/* is the text column x on title t of the bar ? */
static int on_title(int t, int x)
{
  return title_x[t] <= x && x <= title_x[t] + (int) strlen(menu_title[t]) - 1;
}

static void show_title(int t, int color)
{
  pls(title_x[t], menu_y, (unsigned char *) (menu_title[t] + 1), color);
}

/* after "SUBMENU": read on to the END that closes it */
static void skip_submenu(char *line)
{
  int depth = 1;

  while (next_line(line, 100) != -1)
  {
    if (!is_menu_line(line)) continue;
    if (is_keyword(line, "SUBMENU")) depth++;
    if (is_keyword(line, "END"))     depth--;
    if (depth == 0) break;
  }
  if (depth != 0)
  {
    gui_fatal("\n ERROR   SUBMENU without an END ");
  }
}

/* the line that separates entries in text mode: left tee, dashes, right tee */
static void make_bar(char *bar, int len)
{
  int i;

  bar[0] = (char) 0xC3;
  for (i = 0; i < len; i++) bar[1 + i] = (char) 0xC4;
  bar[1 + len] = (char) 0xB4;
  bar[2 + len] = 0;
}
/*--------------------------------------------------------------------------*/
/* The pull down menu.                                                      */
/*   INIT        read the descriptor and show the bar                       */
/*   DISPLAY     show the bar again                                         */
/*   KEY_RETURN  returns a key, the id of a menu entry that was chosen (or  */
/*               whose accelerator was pressed), or EOF if nothing happened */
/*--------------------------------------------------------------------------*/
int bar_menu(int action)
{
  int key      = 0;
  int mouse_on = 0;                 /* a title was clicked                  */
  int acc_hit  = 0;                 /* a title's accelerator was pressed    */
  int i = 0;
  int r;

  if (action == INIT)
  {
    init_menu();
    return -1;
  }
  if (action == DISPLAY)
  {
    display_menu();
    return -1;
  }

  gcurs(MOVE);

  if (mouse_left_pressed() == 1)
  {
    get_mouse_location();
    if (mouse_y == menu_y)
    {
      for (i = 0; i < title_num; i++)
      {
        if (on_title(i, mouse_x))
        {
          mouse_on  = 1;
          title_ptr = i;
          mouse_state();
          break;
        }
      }
    }
  }

  if (mouse_on == 0)
  {
    if (!alt_pressed())
    {
      key = getkey();
      if (key != -1)
      {
        for (i = 0; i < menu_acc_num; i++)
        {
          if (menu_acc[i] == key) return menu_ident[i];
        }
        for (i = 0; i < title_num; i++)
        {
          if (title_acc[i] == key)
          {
            title_ptr = i;
            acc_hit   = 1;
            break;
          }
        }
      }
      if (acc_hit == 0) return key;
    }
    else
    {
      /* while Alt is held: Alt and a key is an accelerator; Alt alone,   */
      /* let go, opens the first menu                                     */
      while (alt_pressed() == 1)
      {
        key = getkey();
        if (key != -1)
        {
          key_return = 1;
          for (i = 0; i < menu_acc_num; i++)
          {
            if (menu_acc[i] == key) return menu_ident[i];
          }
          for (i = 0; i < title_num; i++)
          {
            if (title_acc[i] == key)
            {
              title_ptr = i;
              acc_hit   = 1;
              break;
            }
          }
          if (acc_hit == 0) return key;
        }
        if (acc_hit == 1) break;
      }

      if (key_return == 1 && acc_hit == 0)
      {
        key_return = 0;
        return -1;
      }
      if (acc_hit == 0) title_ptr = 0;
      else              title_ptr = i;
    }
  }

  if (menu_descriptor == TEXT_FILE)
  {
    fp = fopen(menu_file, "rb");
    if (fp == NULL)
    {
      cls();
      printf("\n cannot open file %s ", menu_file);
      exit(-1);
    }
  }

  in_menu = 1;

  for (;;)
  {
    r = expand(acc_hit);
    if (acc_hit == 1) acc_hit = 0;

    switch (r)
    {
      case RIGHTKEY:
       title_ptr++;
       if (title_ptr == title_num) title_ptr = 0;
       break;

      case LEFTKEY:
       title_ptr--;
       if (title_ptr < 0) title_ptr = title_num - 1;
       break;

      case ESC:
       if (menu_descriptor == TEXT_FILE) fclose(fp);
       menu_move_lock = 0;
       in_menu        = 0;
       return -1;

      case ENTER:
       if (menu_descriptor == TEXT_FILE) fclose(fp);
       menu_move_lock = 0;
       in_menu        = 0;
       return cur_ident;
    }
  }
}
/*--------------------------------------------------------------------------*/
/* read the descriptor: the titles go on the bar, the accelerators of all   */
/* the entries are remembered                                               */
/*--------------------------------------------------------------------------*/
void init_menu(void)
{
  char line[100];
  char text[100];
  int  i;

  if (menu_descriptor == TEXT_FILE)
  {
    fp = fopen(menu_file, "rb");
    if (fp == NULL)
    {
      cls();
      printf("\n cannot open %s file \n press a key to exit to dos ", menu_file);
      getkey_wait();
      if (font_width > 1) set_video_mode(3);
      exit(-1);
    }
  }
  else
  {
    /* (the lines compiled into the program are read from the start again) */
    menu          -= menu_line_ptr;
    menu_line_ptr  = 0;
  }

  frect(0, menu_y, txt_scr_xsize - 1, menu_y, menu_color, 0);

  title_num    = 0;
  title_ptr    = 0;
  menu_acc_num = 0;

  while (next_line(line, 100) != -1)
  {
    if (!is_menu_line(line)) continue;

    if (line[1] != '%')
    {
      /* an entry: only its accelerator matters here */
      if (menu_acc_num == MAX_ACC - 1) continue;
      if (is_keyword(line, "LINE") || is_keyword(line, "SUBMENU") || is_keyword(line, "END")) continue;

      menu_acc[menu_acc_num] = 0;
      seperate(line, text, &menu_ident[menu_acc_num], &menu_acc[menu_acc_num], 1);
      if (menu_acc[menu_acc_num] != 0) menu_acc_num++;
      assert(menu_acc_num < MAX_ACC);
      continue;
    }

    /* a title */
    memset(menu_title[title_num], 0, sizeof(menu_title[title_num]));
    title_ident[title_num] = 0;
    title_acc[title_num]   = 0;
    text[0] = 0;
    title_hotkey[title_num] = seperate(line, text, &title_ident[title_num], &title_acc[title_num], 1);
    strncpy(menu_title[title_num], text, sizeof(menu_title[title_num]) - 1);

    if (title_hotkey[title_num] != 0)
    {
      for (i = 0; (unsigned) i < strlen(menu_title[title_num]); i++)
      {
        if (menu_title[title_num][i] == title_hotkey[title_num])
        {
          title_hotkey_loc[title_num] = (char) i;
          break;
        }
      }
    }

    title_loc[title_num] = get_pos();
    title_x[title_num]   = title_ptr + menu_x;

    show_title(title_num, menu_color);
    place_title_hot(title_num);

    title_ptr += (int) strlen(menu_title[title_num]) + 1;
    title_num++;

    if (title_num == MAX_TITLE)
    {
      gui_fatal("\n too many menu titles ");
    }
  }

  title_ptr = 0;

  if (menu_descriptor == TEXT_FILE) fclose(fp);
}
/*--------------------------------------------------------------------------*/
void display_menu(void)
{
  int i;

  frect(0, menu_y, txt_scr_xsize - 1, menu_y, menu_color, 0);

  for (i = 0; i < title_num; i++)
  {
    show_title(i, menu_color);
    place_title_hot(i);
  }
}
/*--------------------------------------------------------------------------*/
/* take a line of the descriptor apart:  "text" id acc                      */
/* returns the hot key (the letter after ^); sw 1 leaves the ^ out of text  */
/*--------------------------------------------------------------------------*/
char seperate(char *line, char *text, int *ident, int *acc, int sw)
{
  int  j      = 0;
  int  quotes = 0;
  char hot    = 0;
  int  i;

  for (i = 0; (unsigned) i < strlen(line); i++)
  {
    if (line[i] == '"')
    {
      quotes++;
      continue;
    }
    if (quotes == 1)
    {
      if (line[i] == '^')
      {
        if (sw == 1)
        {
          i++;
          hot = line[i];
        }
        else
        {
          hot = line[i + 1];
        }
      }
      text[j++] = line[i];
    }
    if (quotes == 2) break;
  }
  text[j] = 0;

  if (quotes != 2)
  {
    cls();
    printf("\n the following line in the menu descriptor file is illegal");
    printf("\n var s=%d and ^%s^", quotes, line);
    getkey_wait();
    if (font_width > 1) set_video_mode(3);
    exit(-1);
  }

  if ((unsigned) i < strlen(line))
  {
    sscanf(line + i, "%d %d", ident, acc);
  }
  return hot;
}
/*--------------------------------------------------------------------------*/
/* the pull down under title title_ptr.                                     */
/* returns ENTER (cur_ident is what was chosen), ESC, LEFTKEY or RIGHTKEY   */
/* (go to the next title), or anything else: show title_ptr's menu instead  */
/*--------------------------------------------------------------------------*/
static char e_text  [MAX_ENTRIES][ENTRY_LEN];  /* (static: there is only    */
static char e_hot   [MAX_ENTRIES];             /*  ever one pull down, and  */
static int  e_hotloc[MAX_ENTRIES];             /*  a 16-bit stack is small) */
static int  e_ident [MAX_ENTRIES];
static int  e_acc   [MAX_ENTRIES];
static long e_sub   [MAX_ENTRIES];
static int  e_len;                             /* the longest entry         */
static int  e_win;                             /* the pull down's window    */

static void entry_normal(int sel)
{
  mchcolorh(1, sel + 1, e_len, 1, menu_color, e_win);
  if (e_hot[sel] != 0)
  {
    writecharh(e_hotloc[sel] + 1, sel + 1, e_text[sel][e_hotloc[sel]], menu_hotkey_color, e_win);
  }
}

static void entry_selected(int sel)
{
  mchcolorh(1, sel + 1, e_len, 1, menu_sel_color, e_win);
}

/* which entry is the mouse on; -1 if on none */
static int entry_at_mouse(int n)
{
  if (xwin[e_win] + 1 <= mouse_x && mouse_x <= xwin[e_win] + e_len &&
      ywin[e_win] < mouse_y && mouse_y <= ywin[e_win] + n)
  {
    return mouse_y - ywin[e_win] - 1;
  }
  return -1;
}

/* the mouse is on another title: make it the current one */
static int switch_title_at_mouse(void)
{
  int i;

  for (i = 0; i < title_num; i++)
  {
    if (on_title(i, mouse_x) && title_ptr != i)
    {
      show_title(title_ptr, menu_color);
      place_title_hot(title_ptr);
      title_ptr = i;
      return 1;
    }
  }
  return 0;
}

int expand(int acc_sw)
{
  char line[100];
  char text[100];
  char bar[ENTRY_LEN + 4];
  int  n      = 0;                  /* entries                              */
  int  sel    = 0;                  /* the one that is selected             */
  int  redo   = 0;                  /* another title has been chosen        */
  int  extra  = 2;                  /* rows of the window besides entries   */
  int  drawn  = 0;                  /* the selection bar is shown           */
  int  key, i, x, y, idx;

  e_len = 0;
  memset(e_text, 0, sizeof(e_text));
  if (font_width > 1) extra = 1;

  show_title(title_ptr, menu_sel_color);
  set_pos(&title_loc[title_ptr]);

  while (next_line(line, 100) != -1)
  {
    if (!is_menu_line(line)) continue;
    if (line[1] == '%') break;                    /* the next title        */

    e_sub[n] = 0;

    if (is_keyword(line, "LINE"))
    {
      strcpy(e_text[n], "LINE");
      e_hot[n] = 0;
      n++;
      if (n == MAX_ENTRIES)
      {
        cls();
        printf("\n too many entries for the title=%s", menu_title[title_ptr]);
        getkey_wait();
        if (font_width > 1) set_video_mode(3);
        exit(-1);
      }
      continue;
    }

    if (is_keyword(line, "SUBMENU"))
    {
      if (n > 0) e_sub[n - 1] = get_pos();
      skip_submenu(line);
      continue;
    }

    if (is_keyword(line, "END")) continue;

    e_ident[n] = 0;
    e_acc[n]   = 0;
    e_hot[n]   = seperate(line, text, &e_ident[n], &e_acc[n], 1);
    strncpy(e_text[n], text, ENTRY_LEN - 1);

    e_hotloc[n] = 0;
    for (i = 0; (unsigned) i < strlen(e_text[n]); i++)
    {
      if (e_text[n][i] == e_hot[n])
      {
        e_hotloc[n] = i;
        break;
      }
    }
    if (e_hot[n] >= 'a' && e_hot[n] <= 'z') e_hot[n] -= 0x20;

    if ((int) strlen(e_text[n]) > e_len) e_len = (int) strlen(e_text[n]);
    n++;
    if (n == MAX_ENTRIES)
    {
      cls();
      printf("\n too many entries for the title=%s", menu_title[title_ptr]);
      getkey_wait();
      if (font_width > 1) set_video_mode(3);
      exit(-1);
    }
  }

  if (n > 0)
  {
    x = title_x[title_ptr];
    y = menu_y + 1;
  }
  else
  {
    if (acc_sw == 1)
    {
      /* a title without entries, reached by its accelerator: it is the   */
      /* choice                                                           */
      show_title(title_ptr, menu_sel_color);
      cur_ident = title_ident[title_ptr];
      delay_func(100);
      show_title(title_ptr, menu_color);
      place_title_hot(title_ptr);
      return ENTER;
    }
    x = -1;
    y = -1;
  }

  make_bar(bar, e_len);

  if (n > 0)
  {
    if (x + e_len + 3 > txt_scr_xsize - 1)
    {
      x = txt_scr_xsize - 1 - e_len - 3;
      if (x < 0) x = 0;
    }

    e_win = messagebox(x, y, e_len + 2, n + extra, menu_color, menu_shadow, 1);

    for (i = 0; i < n; i++)
    {
      if (stricmp(e_text[i], "LINE") == 0)
      {
        if (font_width == 1)
        {
          pls(x, y + i + 1, (unsigned char *) bar, menu_color);
        }
        else
        {
          gr_line(x * font_width, (y + i + 1) * font_height + 7,
                  (x + e_len + 2) * font_width - 2, (y + i + 1) * font_height + 7,
                  menu_color & 0xff);
        }
      }
      else
      {
        pls(x + 1, y + i + 1, (unsigned char *) e_text[i], menu_color);
        if (e_hot[i] != 0)
        {
          writechar(x + e_hotloc[i] + 1, y + i + 1, e_text[i][e_hotloc[i]], menu_hotkey_color);
        }
        if (e_sub[i] != 0)
        {
          writechar(x + e_len, y + i + 1, 0x10, menu_color);    /* an arrow */
        }
      }
    }

    mchcolor(x + 1, y + sel + 1, e_len, 1, menu_sel_color);
    drawn = 1;
  }

  menu_move_lock = 1;

  for (;;)
  {
    key = getkey();
    gcurs(MOVE);
    mouse_state();
    process_windows();

    if (key != -1)
    {
      /* the accelerator of another title, or of any entry */
      for (i = 0; i < title_num; i++)
      {
        if (title_acc[i] == key && title_ptr != i)
        {
          show_title(title_ptr, menu_color);
          place_title_hot(title_ptr);
          title_ptr = i;
          redo      = 1;
          break;
        }
      }
      for (i = 0; i < menu_acc_num; i++)
      {
        if (menu_acc[i] == key)
        {
          cur_ident = menu_ident[i];
          key       = ENTER;
          redo      = 1;
          break;
        }
      }
    }

    if (key >= 'a' && key <= 'z') key -= 0x20;

    /* an entry's hot key */
    for (i = 0; i < n; i++)
    {
      if (e_hot[i] != 0 && (unsigned char) e_hot[i] == key)
      {
        cur_ident = e_ident[i];
        entry_normal(sel);
        sel = i;
        entry_selected(sel);
        key = ENTER;
        break;
      }
    }

    if (ms == PRESSED)
    {
      /* the selection follows the mouse while its button is down */
      get_mouse_location();
      idx = (n > 0) ? entry_at_mouse(n) : -1;

      if (idx >= 0)
      {
        menu_move_lock = 1;
        if (strnicmp(e_text[idx], "LINE", 4) == 0)
        {
          if (drawn == 1) entry_normal(sel);
          drawn = 0;
          continue;
        }
        if (!(idx == sel && drawn != 0))
        {
          entry_normal(sel);
          sel = idx;
          entry_selected(sel);
          drawn = 1;
          continue;
        }
      }
      else if (mouse_y == menu_y)
      {
        if (switch_title_at_mouse()) redo = 1;
      }
      else if (mouse_y == menu_y + 1)
      {
        continue;
      }
      else if (n != 0 && drawn == 1)
      {
        entry_normal(sel);
        drawn = 0;
      }
    }
    else if (ms == PRESSEDRELEASED)
    {
      /* the mouse button has been let go */
      if (on_title(title_ptr, mouse_x)) menu_move_lock = 0;
      ms = RELEASED;

      if (n > 0 && mouse_y == menu_y && on_title(title_ptr, mouse_x))
      {
        /* on the menu's own title: the menu stays */
        entry_selected(sel);
        drawn = 1;
        continue;
      }

      idx = (n > 0) ? entry_at_mouse(n) : -1;
      if (idx >= 0)
      {
        if (strnicmp(e_text[idx], "LINE", 4) == 0)
        {
          key = ESC;
          break;
        }
        cur_ident = e_ident[sel];
        key       = ENTER;
      }
      else if (n == 0 && mouse_y == menu_y && on_title(title_ptr, mouse_x))
      {
        key       = ENTER;
        cur_ident = title_ident[title_ptr];
      }
      else
      {
        key = ESC;
      }
    }

    if (key == ENTER && redo == 0 && n > 0 && e_sub[sel] != 0)
    {
      key = submenu(e_sub[sel], xwin[e_win] + e_len / 2 + 1, ywin[e_win] + sel + 2);

      if (key == -2)
      {
        /* the mouse was clicked outside the submenu */
        get_mouse_location();
        idx = -1;
        if (xwin[e_win] <= mouse_x && mouse_x <= xwin[e_win] + e_len + 2 &&
            ywin[e_win] + 1 <= mouse_y && mouse_y <= ywin[e_win] + n)
        {
          idx = mouse_y - ywin[e_win] - 1;
        }

        if (idx >= 0)
        {
          if (strnicmp(e_text[idx], "LINE", 4) == 0)
          {
            key = ESC;
            break;
          }
          entry_normal(sel);
          sel = idx;
          entry_selected(sel);
          continue;
        }
        if (mouse_y == menu_y)
        {
          if (switch_title_at_mouse()) redo = 1;
          else                         key  = ESC;
        }
        else
        {
          key = ESC;
        }
      }
      if (key != -1) break;
    }

    if (key == RIGHTKEY || key == LEFTKEY || key == ESC || key == ENTER)
    {
      if (title_num == 1 && (key == RIGHTKEY || key == LEFTKEY)) continue;

      if (key == ENTER && redo == 0)
      {
        if (n > 0) cur_ident = e_ident[sel];
        else       cur_ident = title_ident[title_ptr];
      }
      break;
    }

    if (key == UPKEY || key == DOWNKEY)
    {
      if (n == 0) continue;

      entry_normal(sel);
      if (key == DOWNKEY)
      {
        sel++;
        if (sel == n) sel = 0;
        if (stricmp(e_text[sel], "LINE") == 0)
        {
          sel++;
          if (sel == n) sel = 0;
        }
      }
      else
      {
        sel--;
        if (sel < 0) sel = n - 1;
        if (stricmp(e_text[sel], "LINE") == 0)
        {
          sel--;
          if (sel < 0) sel = n - 1;
        }
      }
      entry_selected(sel);
      drawn = 1;
    }

    if (redo == 1) break;
  }

  show_title(title_ptr, menu_color);
  place_title_hot(title_ptr);

  if (n > 0) destroy_messagebox(e_win);

  return key;
}
/*--------------------------------------------------------------------------*/
/* a title's hot key in its own color                                       */
/*--------------------------------------------------------------------------*/
void place_title_hot(int t)
{
  if (title_hotkey[t] != 0)
  {
    writechar(title_x[t] + title_hotkey_loc[t] - 1, menu_y, title_hotkey[t], menu_hotkey_color);
  }
}
/*--------------------------------------------------------------------------*/
/* a submenu: the lines that follow place pos of the descriptor, up to END. */
/* returns ENTER (cur_ident is what was chosen), -1 (ESC: back to the menu  */
/* above) or -2 (the mouse was clicked somewhere else)                      */
/*--------------------------------------------------------------------------*/
int submenu(long pos, int x, int y)
{
  char line[100];
  char one[100];
  char text[SUB_ENTRIES][SUB_LEN];
  long sub  [SUB_ENTRIES];
  int  ident[SUB_ENTRIES];
  int  n      = 0;
  int  len    = 0;                  /* the longest entry                    */
  int  cursel = 0;
  int  extra  = 2;
  int  r, i, wh, dummy, hot, w;

  if (font_width > 1) extra = 1;
  memset(text, 0, sizeof(text));

  set_pos(&pos);

  while (next_line(line, 100) != -1)
  {
    if (!is_menu_line(line)) continue;
    if (is_keyword(line, "END")) break;
    if (n == SUB_ENTRIES) continue;

    sub[n] = 0;

    if (is_keyword(line, "SUBMENU"))
    {
      if (n > 0) sub[n - 1] = get_pos();
      skip_submenu(line);
      continue;
    }

    if (is_keyword(line, "LINE"))
    {
      strcpy(text[n], "LINE");
      ident[n] = 0;
      n++;
      continue;
    }

    ident[n] = 0;
    hot = seperate(line, one, &ident[n], &dummy, 0);
    strncpy(text[n], one, SUB_LEN - 2);

    w = (int) strlen(text[n]);
    if (hot != 0) w--;                            /* (the ^ is not shown)  */
    if (w > len) len = w;
    n++;
  }

  if (n == 0) return -1;

  for (i = 0; i < n; i++)
  {
    while ((int) strlen(text[i]) < len) strcat(text[i], " ");

    if (sub[i] != 0)                              /* an arrow at the end   */
    {
      if (strchr(text[i], '^') == NULL) text[i][len - 1] = 0x10;
      else                              text[i][len]     = 0x10;
    }
  }

  if (x + len + 3 > txt_scr_xsize - 1) x -= len + 3;
  if (x < 0) x = 0;
  if (y + n + 1 > txt_scr_ysize - 1)   y = txt_scr_ysize - 3 - n;

  wh = messagebox(x, y, len + 2, n + extra, menu_color, menu_shadow, 1);

  for (;;)
  {
    r = menus(text, n, 1, 1, menu_color, menu_sel_color, cursel, wh);
    if (r >= 0 && r < n) cursel = r;

    if (r < 0 || sub[r] == 0) break;

    r = submenu(sub[r], xwin[wh] + len / 2 + 1, ywin[wh] + r + 2);

    if (r == ENTER)
    {
      r = ESC;                      /* (chosen further down: cur_ident is set) */
      break;
    }
    if (r == -1) continue;
    if (r == -2)
    {
      /* a click outside the submenu below: on one of these entries ? */
      w = 0;
      for (i = 0; i < n; i++)
      {
        if ((int) strlen(text[i]) > w) w = (int) strlen(text[i]);
      }
      get_mouse_location();
      if (xwin[wh] <= mouse_x && mouse_x <= xwin[wh] + w + 1 &&
          ywin[wh] + 1 <= mouse_y && mouse_y <= ywin[wh] + n)
      {
        cursel = mouse_y - ywin[wh] - 1;
        continue;
      }
    }
    break;
  }

  destroy_messagebox(wh);

  if (r == -1 || r == -2) return r;
  if (r != ESC) cur_ident = ident[r];
  return ENTER;
}
/*--------------------------------------------------------------------------*/
/* how many spaces a line begins with                                       */
/*--------------------------------------------------------------------------*/
int remove_spaces(char *s)
{
  int i = 0;

  while (s[i] == ' ') i++;
  return i;
}
/*--------------------------------------------------------------------------*/
/* The scroll down menu: n entries of 40 characters, shown at x,y (of       */
/* window h, if h is not -1), ptr is the entry selected at first.           */
/* An entry "LINE" is a line; a ^ comes before an entry's hot key.          */
/* Returns the entry that was chosen, -1 for ESC, -2 for a mouse click      */
/* that was not on the menu.                                                */
/*--------------------------------------------------------------------------*/
static char  (*m_menu)[40];
static char  m_hot[25];
static int   m_hotloc[25];
static int   m_x, m_y, m_len, m_color;

static void m_normal(int sel)
{
  mchcolor(m_x, m_y + sel, m_len, 1, m_color);
  if (m_hot[sel] != 0)
  {
    writechar(m_x + m_hotloc[sel], m_y + sel, m_menu[sel][m_hotloc[sel] + 1], menu_hotkey_color);
  }
}

int menus(char menu_entries[20][40], int n, int x, int y, int color, int selcolor, int ptr, int h)
{
  char  bar[44];
  char  tmp[44];
  char *p;
  int   key, sel, i, len, idx;
  int   drawn;
  int   inside;

  if (n > 25) n = 25;

  m_menu  = menu_entries;
  m_color = color;
  m_len   = 0;
  m_x     = x;
  m_y     = y;
  if (h >= 0)
  {
    m_x = x + xwin[h];
    m_y = y + ywin[h];
  }

  if (ptr < 0 || ptr >= n) ptr = 0;
  sel = ptr;
  ms  = RELEASED;

  for (i = 0; i < n; i++)
  {
    p = strchr(m_menu[i], '^');
    if (p != NULL)
    {
      m_hot[i] = p[1];
      if (m_hot[i] >= 'a' && m_hot[i] <= 'z') m_hot[i] -= 0x20;
      m_hotloc[i] = (int) (p - m_menu[i]);
    }
    else
    {
      m_hot[i]    = 0;
      m_hotloc[i] = -1;
    }

    len = (int) strlen(m_menu[i]);
    if (m_hot[i] != 0) len--;
    if (len > m_len) m_len = len;
  }
  if (m_len > 39) m_len = 39;

  make_bar(bar, m_len);

  for (i = 0; i < n; i++)
  {
    while ((int) strlen(m_menu[i]) < m_len) strcat(m_menu[i], " ");

    if (strnicmp(m_menu[i], "LINE", 4) == 0)
    {
      if (font_width == 1)
      {
        pls(m_x - 1, m_y + i, (unsigned char *) bar, menu_color);
      }
      else
      {
        gr_line((m_x - 1) * font_width, (m_y + i) * font_height + 7,
                (m_x + m_len + 1) * font_width - 2, (m_y + i) * font_height + 7,
                menu_color & 0xff);
      }
    }
    else
    {
      strcpy(tmp, m_menu[i]);
      if (m_hot[i] != 0)
      {
        p = strchr(tmp, '^');
        memmove(p, p + 1, strlen(p + 1) + 1);
      }
      if (font_width > 1)
      {
        gfillrect((m_x - 1) * font_width + 1, (m_y + i) * font_height,
                  (m_x + m_len + 1) * font_width - 2, (m_y + i + 1) * font_height - 1, color >> 4);
      }
      pls(m_x, m_y + i, (unsigned char *) tmp, color);
      if (m_hot[i] != 0)
      {
        writechar(m_x + m_hotloc[i], m_y + i, m_menu[i][m_hotloc[i] + 1], menu_hotkey_color);
      }
    }
  }

  drawn = 1;
  mchcolor(m_x, m_y + sel, m_len, 1, selcolor);
  menu_move_lock = 0;

  for (;;)
  {
    key = getkey();
    gcurs(MOVE);

    if (process_windows() == -2 && h >= 0)
    {
      /* the window has been moved, and the menu with it */
      m_x = x + xwin[h];
      m_y = y + ywin[h];
      mchcolor(m_x, m_y + sel, m_len, 1, selcolor);
      drawn          = 1;
      menu_move_lock = 1;
    }

    if (key >= 'a' && key <= 'z') key -= 0x20;

    if (key != -1)
    {
      for (i = 0; i < n; i++)
      {
        if (m_hot[i] != 0 && (unsigned char) m_hot[i] == key)
        {
          key = ENTER;
          m_normal(sel);
          sel = i;
          mchcolor(m_x, m_y + sel, m_len, 1, selcolor);
          break;
        }
      }
    }

    if (key == -1 && mousefound == 1)
    {
      mouse_state();
      if (ms == PRESSED)
      {
        get_mouse_location();
        inside = (mouse_x >= m_x && m_x + (int) strlen(m_menu[0]) > mouse_x);

        /* a press on the row above the menu is left to the window */
        if (!(inside && m_y - 1 == mouse_y)) menu_move_lock = 1;

        if (inside && mouse_y >= m_y && m_y + n > mouse_y)
        {
          idx = mouse_y - m_y;
          if (strnicmp(m_menu[idx], "LINE", 4) == 0)
          {
            if (drawn == 1)
            {
              m_normal(sel);
              drawn = 0;
            }
          }
          else if (drawn == 0)
          {
            sel = idx;
            mchcolor(m_x, mouse_y, m_len, 1, selcolor);
            drawn = 1;
          }
          else if (idx != sel)
          {
            m_normal(sel);
            sel = idx;
            mchcolor(m_x, m_y + sel, m_len, 1, selcolor);
          }
        }
        else if (drawn == 1)
        {
          m_normal(sel);
          drawn = 0;
        }
      }
      else if (mouse_state() == PRESSEDRELEASED)
      {
        get_mouse_location();
        if (mouse_x >= m_x && m_x + (int) strlen(m_menu[0]) > mouse_x &&
            mouse_y >= m_y && m_y + n > mouse_y)
        {
          key = ENTER;
        }
        else
        {
          key = -2;
        }
        ms = RELEASED;
      }
    }

    if (key == UPKEY || key == DOWNKEY)
    {
      if (mousefound == 1) hide_mouse_cursor();
      m_normal(sel);

      if (key == UPKEY)
      {
        sel--;
        if (sel >= 0 && strnicmp(m_menu[sel], "LINE", 4) == 0) sel--;
        if (sel < 0) sel = n - 1;
      }
      else
      {
        sel++;
        if (sel < n && strnicmp(m_menu[sel], "LINE", 4) == 0) sel++;
        if (sel >= n) sel = 0;
      }

      mchcolor(m_x, m_y + sel, m_len, 1, selcolor);
      drawn = 1;
      if (mousefound == 1) show_mouse_cursor();
    }

    if (key == ENTER) return sel;
    if (key == ESC)   return -1;
    if (key == -2)    return -2;
  }
}
/*--------------------------------------------------------------------------*/
/*                    the colors and the place of the menu                  */
/*--------------------------------------------------------------------------*/
void bg_menu_color(int color)        { menu_color = (menu_color & 0x0f) | color; }
void fg_menu_color(int color)        { menu_color = (menu_color & 0xf0) | color; }
void bg_menu_hotkey_color(int color) { menu_hotkey_color = (menu_hotkey_color & 0x0f) | color; }
void fg_menu_hotkey_color(int color) { menu_hotkey_color = (menu_hotkey_color & 0xf0) | color; }
void bg_menu_sel_color(int color)    { menu_sel_color = (menu_sel_color & 0x0f) | color; }
void fg_menu_sel_color(int color)    { menu_sel_color = (menu_sel_color & 0xf0) | color; }
void bg_menu_shadow(int color)       { menu_shadow = (menu_shadow & 0x0f) | color; }
void fg_menu_shadow(int color)       { menu_shadow = (menu_shadow & 0xf0) | color; }

void ch_menu_x(int x)                { menu_x = x; }
void ch_menu_y(int y)                { menu_y = y; }
void ch_menu_xsize(int xsize)        { menu_x_size = xsize; }

void ch_menu_filename(char *newfilename)
{
  strncpy(menu_file, newfilename, sizeof(menu_file) - 1);
  menu_file[sizeof(menu_file) - 1] = 0;
}
/*--------------------------------------------------------------------------*/
/* the descriptor, a line at a time: from the file, or from the lines that  */
/* CT_VAR compiled into the program                                         */
/*--------------------------------------------------------------------------*/
int next_line(char *buffer, int size)
{
  char tmp[100];

  memset(buffer, 0, size);

  switch (menu_descriptor)
  {
    case TEXT_FILE:
     if (fgets(buffer, size, fp) != NULL)
     {
       strncpy(tmp, buffer + remove_spaces(buffer), sizeof(tmp) - 1);
       tmp[sizeof(tmp) - 1] = 0;
       strcpy(buffer, tmp);
       return 0;
     }
     return -1;

    case VARIABLE:
     if (menu_line_ptr >= menu_lines) return -1;
     strncpy(tmp, *menu, sizeof(tmp) - 1);
     tmp[sizeof(tmp) - 1] = 0;
     strncpy(buffer, tmp + remove_spaces(tmp), size - 1);
     menu++;
     menu_line_ptr++;
     assert(menu_line_ptr <= menu_lines);
     return 0;
  }
  return 0;
}
/*--------------------------------------------------------------------------*/
void set_pos(long *pos)
{
  switch (menu_descriptor)
  {
    case TEXT_FILE:
     fseek(fp, *pos, SEEK_SET);
     break;

    case VARIABLE:
     menu += (int) *pos - menu_line_ptr;
     menu_line_ptr = (int) *pos;
     assert(menu_line_ptr <= menu_lines);
     break;
  }
}
/*--------------------------------------------------------------------------*/
long get_pos(void)
{
  switch (menu_descriptor)
  {
    case TEXT_FILE: return ftell(fp);
    case VARIABLE:  return menu_line_ptr;
  }
  return -1;
}
/*--------------------------------------------------------------------------*/
void set_menu_descriptor(int type, char **menu_variable, int num_of_lines)
{
  if (type == TEXT_FILE)
  {
    menu_descriptor = TEXT_FILE;
  }
  else
  {
    menu_descriptor = VARIABLE;
    menu            = menu_variable;
    menu_lines      = num_of_lines;
    menu_line_ptr   = 0;
  }
}
/*--------------------------------------------------------------------------*/
