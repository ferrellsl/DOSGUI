/*--------------------------------------------------------------------------*/
/* FILEDLG : a File dialog box                                              */
/*                                                                          */
/* New in the rebuilt library (the original had none).  It is made of the   */
/* library's own parts: a window, buttons, the scroll bar, the text screen. */
/*                                                                          */
/*   int file_open_dialog(char *title, char *mask, char *filename);         */
/*       to choose a file that is there: a name that is not is refused      */
/*   int file_save_dialog(char *title, char *mask, char *filename);         */
/*       to name a file to write: if there is such a file already, the      */
/*       user is asked whether to replace it; a name typed without an       */
/*       extension gets the one of the mask ("*.TXT" gives .TXT)            */
/*   int file_dialog(char *title, char *mask, char *filename);              */
/*       the plain one: any name is taken, nothing is asked                 */
/*                                                                          */
/*   title    : the title of the window (NULL: "Open File", "Save File")    */
/*   mask     : the files to list, "*.TXT" for one (NULL or "" : "*.*")     */
/*   filename : where the chosen name is put, with its drive and path;      */
/*              room for 128 characters                                     */
/*   returns YES if a file was chosen, NO if the dialog was canceled        */
/*                                                                          */
/* In the dialog:                                                           */
/*   arrows, PgUp, PgDn, Home, End   move through the list                  */
/*   letters                         type a name (or a mask like *.C)       */
/*   Enter, a double click, OK       go into the directory or drive, apply  */
/*                                   the mask, or take the file             */
/*   TAB                             to the buttons                         */
/*   ESC, Cancel                     give up                                */
/* A name that is not in the list can be typed (but not to open a file).    */
/* The current drive and directory are the same after the dialog as before. */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "gui.h"
#include "gport.h"

#define DLG_W       46            /* the window, in text cells              */
#define DLG_H       19
#define LIST_X      2             /* the list, inside the window            */
#define LIST_Y      6
#define LIST_W      24
#define LIST_ROWS   11
#define NAME_X      13            /* the name field                         */
#define NAME_Y      3
#define NAME_W      30
#define MAX_FILES   400
#define FNAME_MAX    64

#define FD_ANY      0             /* the kinds of dialog                    */
#define FD_OPEN     1
#define FD_SAVE     2

#define ID_OK       2001
#define ID_CANCEL   2002

#define C_WINDOW    0xF0          /* black on white                         */
#define C_LABEL     0xF4
#define C_PATH      0xF8
#define C_FIELD     0x70
#define C_LIST      0xF0
#define C_SELECT    0x1F
#define C_DIR       0xF1

#define E_FILE      0
#define E_DIR       1
#define E_DRIVE     2

struct entry
{
  char name[14];
  char kind;
};

static struct entry *files;       /* what is in the directory               */
static int   nfiles;
static int   top;                 /* the first one shown                    */
static int   sel;                 /* the selected one                       */
static int   wx, wy;              /* the window's corner                    */
static char  name[FNAME_MAX + 1];  /* the name field                         */
static char  mask[FNAME_MAX + 1];
static int   sb_x1, sb_y1, sb_x2, sb_y2;
static int   dlg_type;            /* FD_ANY, FD_OPEN or FD_SAVE             */

int process_windows(void);

/*--------------------------------------------------------------------------*/
static int compare(const void *a, const void *b)
{
  const struct entry *x = (const struct entry *) a;
  const struct entry *y = (const struct entry *) b;

  if (x->kind != y->kind)
  {
    /* directories, then files, then drives */
    static const int order[3] = { 1, 0, 2 };
    return order[(int) x->kind] - order[(int) y->kind];
  }
  return strcmp(x->name, y->name);
}

static void add(const char *n, int kind)
{
  if (nfiles >= MAX_FILES) return;
  strncpy(files[nfiles].name, n, 13);
  files[nfiles].name[13] = 0;
  files[nfiles].kind = (char) kind;
  nfiles++;
}

/* read the current directory: its directories, the files of the mask, */
/* and the drives                                                       */
static void read_directory(void)
{
  char n[16];
  char d[8];
  int  is_dir;
  int  here, drive;

  char cwd[140];
  int  at_root;

  nfiles = 0;

  cwd[0] = 0;
  gp_getcwd(cwd, sizeof(cwd) - 1);
  at_root = (strlen(cwd) <= 3);                 /* "C:\" : nothing above it */

  if (gp_findfirst("*.*", n, &is_dir) == 0)
  {
    do
    {
      if (!is_dir || strcmp(n, ".") == 0) continue;
      if (at_root && strcmp(n, "..") == 0) continue;
      add(n, E_DIR);
    }
    while (gp_findnext(n, &is_dir) == 0);
  }

  if (gp_findfirst(mask, n, &is_dir) == 0)
  {
    do
    {
      if (!is_dir) add(n, E_FILE);
    }
    while (gp_findnext(n, &is_dir) == 0);
  }

  here = gp_getdrive();
  for (drive = 1; drive <= 26; drive++)
  {
    if (drive == here || gp_setdrive(drive) == 0)
    {
      sprintf(d, "[-%c-]", 'A' + drive - 1);
      add(d, E_DRIVE);
    }
  }
  gp_setdrive(here);

  qsort(files, nfiles, sizeof(struct entry), compare);

  top = 0;
  sel = 0;
}
/*--------------------------------------------------------------------------*/
static void show_name(void)
{
  char line[NAME_W + 1];
  int  len = (int) strlen(name);
  int  from = (len > NAME_W - 1) ? len - (NAME_W - 1) : 0;
  int  i;

  memset(line, ' ', NAME_W);
  line[NAME_W] = 0;
  for (i = 0; name[from + i] != 0; i++) line[i] = name[from + i];

  txt_curs(0);
  pls(wx + NAME_X, wy + NAME_Y, (unsigned char *) line, C_FIELD);

  txt_curs(1);
  txt_curs_x = wx + NAME_X + len - from;
  txt_curs_y = wy + NAME_Y;
  txt_curs(5);
}

static void show_path(void)
{
  char path[140];
  char line[DLG_W];
  int  w = DLG_W - 4;
  int  len;

  path[0] = 0;
  gp_getcwd(path, sizeof(path) - 1);
  len = (int) strlen(path);

  memset(line, ' ', w);
  line[w] = 0;
  if (len > w)
  {
    memcpy(line, "...", 3);
    memcpy(line + 3, path + len - (w - 3), w - 3);
  }
  else
  {
    memcpy(line, path, len);
  }
  pls(wx + 2, wy + NAME_Y + 1, (unsigned char *) line, C_PATH);
}

static void show_row(int row)
{
  char line[LIST_W + 1];
  int  i = top + row;
  int  color = C_LIST;

  memset(line, ' ', LIST_W);
  line[LIST_W] = 0;

  if (i < nfiles)
  {
    memcpy(line + 1, files[i].name, strlen(files[i].name));
    if (files[i].kind == E_DIR)
    {
      memcpy(line + LIST_W - 6, "<DIR>", 5);
      color = C_DIR;
    }
    if (files[i].kind == E_DRIVE) color = C_DIR;
    if (i == sel) color = C_SELECT;
  }
  pls(wx + LIST_X, wy + LIST_Y + row, (unsigned char *) line, color);
}

static void show_list(void)
{
  int row;

  txt_curs(7);
  for (row = 0; row < LIST_ROWS; row++) show_row(row);
}

/* the scroll bar goes from 0 to this */
static int scroll_max(void)
{
  return (nfiles > LIST_ROWS) ? nfiles - LIST_ROWS : 1;
}

/* a new directory or a new mask: read, and show it all */
static void refresh(void)
{
  read_directory();
  strcpy(name, mask);
  show_path();
  show_list();
  scr_bar(INIT, 0, scroll_max(), &top, sb_x1, sb_y1, sb_x2, sb_y2, 0, 7, -1);
  show_name();
}

/* the selection has moved: keep it in sight, and put its name in the field */
static void select_entry(int i)
{
  int old_top = top;
  int old_sel = sel;

  if (nfiles == 0) return;
  if (i < 0) i = 0;
  if (i >= nfiles) i = nfiles - 1;
  sel = i;

  if (sel < top) top = sel;
  if (sel >= top + LIST_ROWS) top = sel - LIST_ROWS + 1;

  if (top != old_top)
  {
    show_list();
  }
  else
  {
    txt_curs(7);
    if (old_sel >= top && old_sel < top + LIST_ROWS) show_row(old_sel - top);
    show_row(sel - top);
  }

  strcpy(name, files[sel].name);
  show_name();
}
/*--------------------------------------------------------------------------*/
/* a line to the user, where the directory is shown                         */
/*--------------------------------------------------------------------------*/
static void message(const char *text)
{
  char line[DLG_W];
  int  w = DLG_W - 4;

  memset(line, ' ', w);
  line[w] = 0;
  memcpy(line, text, strlen(text));
  txt_curs(7);
  pls(wx + 2, wy + NAME_Y + 1, (unsigned char *) line, 0xF4);
}

static int file_exists(const char *path)
{
  char n[16];
  int  is_dir;

  return gp_findfirst(path, n, &is_dir) == 0 && !is_dir;
}

/* the file is there already: write over it ?  Y or N */
static int ask_replace(void)
{
  int key;

  printf("\a");
  message("That file exists.  Replace it ?  (Y/N)");
  for (;;)
  {
    key = getkey();
    gcurs(MOVE);
    if (key == 'y' || key == 'Y') return 1;
    if (key == 'n' || key == 'N' || key == ESC || key == ENTER) break;
  }
  show_path();
  show_name();
  return 0;
}

/* to save: a name without an extension gets the one of the mask */
static void add_extension(void)
{
  char *dot = strrchr(mask, '.');
  char *last = strrchr(name, '\\');

  if (last == NULL) last = name;
  if (strchr(last, '.') != NULL) return;
  if (dot == NULL || dot[1] == 0) return;
  if (strchr(dot, '*') != NULL || strchr(dot, '?') != NULL) return;
  if (strlen(name) + strlen(dot) > FNAME_MAX) return;
  strcat(name, dot);
}
/*--------------------------------------------------------------------------*/
/* Enter: what the name field says is done.                                 */
/* returns 1 if it names a file (and filename is filled in)                 */
/*--------------------------------------------------------------------------*/
static int take_file(char *filename);

static int activate(char *filename)
{
  if (take_file(filename) == 0) return 0;

  if (dlg_type == FD_OPEN && !file_exists(filename))
  {
    printf("\a");
    message("There is no such file.");
    delay_func(1500);
    show_path();
    show_name();
    return 0;
  }
  if (dlg_type == FD_SAVE && file_exists(filename))
  {
    if (ask_replace() == 0) return 0;
  }
  return 1;
}

static int take_file(char *filename)
{
  char path[140];
  int  len;

  if (name[0] == 0) return 0;

  /* a drive, as the list shows it or as "C:" */
  if (name[0] == '[' && name[1] == '-' && name[3] == '-')
  {
    if (gp_setdrive(toupper((unsigned char) name[2]) - 'A' + 1) != 0) printf("\a");
    refresh();
    return 0;
  }
  if (name[1] == ':' && name[2] == 0)
  {
    if (gp_setdrive(toupper((unsigned char) name[0]) - 'A' + 1) != 0) printf("\a");
    refresh();
    return 0;
  }

  /* a mask */
  if (strchr(name, '*') != NULL || strchr(name, '?') != NULL)
  {
    strcpy(mask, name);
    refresh();
    return 0;
  }

  /* a directory */
  if (gp_chdir(name) == 0)
  {
    refresh();
    return 0;
  }

  /* a file: its whole name */
  if (dlg_type == FD_SAVE)
  {
    add_extension();
    show_name();
  }
  if (strchr(name, ':') != NULL || name[0] == '\\')
  {
    strcpy(filename, name);
    return 1;
  }

  path[0] = 0;
  gp_getcwd(path, sizeof(path) - 1);
  len = (int) strlen(path);
  if (len > 0 && path[len - 1] != '\\') strcat(path, "\\");
  if (strlen(path) + strlen(name) > 127)
  {
    printf("\a");
    return 0;
  }
  strcpy(filename, path);
  strcat(filename, name);
  return 1;
}
/*--------------------------------------------------------------------------*/
static int dialog(int type, char *title, char *file_mask, char *filename)
{
  char start_dir[140];
  int  start_drive;
  int  wh, b1, b2;
  int  key, i, row;
  int  old_top;
  int  result = NO;
  int  done   = 0;
  int  gx, gy, gx2, gy2;

  if (font_width == 1) return NO;             /* (graphics modes only)      */

  dlg_type = type;
  if (title == NULL) title = (type == FD_SAVE) ? "Save File" : "Open File";

  files = (struct entry *) malloc(MAX_FILES * sizeof(struct entry));
  if (files == NULL) return NO;

  start_drive  = gp_getdrive();
  start_dir[0] = 0;
  gp_getcwd(start_dir, sizeof(start_dir) - 1);

  strncpy(mask, (file_mask != NULL && file_mask[0] != 0) ? file_mask : "*.*", FNAME_MAX);
  mask[FNAME_MAX] = 0;

  wx = (txt_scr_xsize - DLG_W) / 2;
  wy = (txt_scr_ysize - DLG_H) / 2;
  if (wx < 0) wx = 0;
  if (wy < 1) wy = 1;

  deactivate_activebuttons();

  /* the window, framed as the example program frames its own */
  wh  = messagebox(wx, wy, DLG_W, DLG_H, C_WINDOW, 0x08, YES);
  gx  = wx * font_width;
  gy  = wy * font_height;
  gx2 = (wx + DLG_W) * font_width;
  gy2 = (wy + DLG_H) * font_height;

  grect(gx + 1, gy + 1, gx2 - 2, gy2 - 1, 0);
  for (i = 1; i <= 5; i++)
  {
    grect(gx + 1 + i, gy + 1 + i, gx2 - 2 - i, gy2 - 1 - i, 1);
  }
  gfillrect(gx + 8, gy + 8, gx2 - 9, gy + 27, 1);
  gpls((wx + DLG_W / 2 - (int) strlen(title) / 2) * font_width, gy + font_height - 5,
       (unsigned char *) title, 0x1F);

  pls(wx + 2, wy + NAME_Y, (unsigned char *) "File name:", C_LABEL);

  /* a line around the list, and the scroll bar beside it */
  grect((wx + LIST_X) * font_width - 2, (wy + LIST_Y) * font_height - 2,
        (wx + LIST_X + LIST_W) * font_width + 1, (wy + LIST_Y + LIST_ROWS) * font_height + 1, 0);

  sb_x1 = (wx + LIST_X + LIST_W) * font_width + 4;
  sb_y1 = (wy + LIST_Y) * font_height - 2;
  sb_x2 = sb_x1 + 19;
  sb_y2 = (wy + LIST_Y + LIST_ROWS) * font_height + 1;

  if (type == FD_OPEN)      b1 = draw_button(" ^Open ", wx + 33, wy + 7, 78, 23, ID_OK, ALTO);
  else if (type == FD_SAVE) b1 = draw_button(" ^Save ", wx + 33, wy + 7, 78, 23, ID_OK, ALTS);
  else                      b1 = draw_button("  ^OK  ", wx + 33, wy + 7, 78, 23, ID_OK, ALTO);
  b2 = draw_button("Cancel",  wx + 33, wy + 10, 78, 23, ID_CANCEL, ESC);

  alt_control(INIT, 0);
  txt_curs(INIT);

  refresh();
  old_top = top;

  if (ms == PRESSEDRELEASED) ms = RELEASED;

  while (!done)
  {
    key = getkey();
    txt_curs(8);
    gcurs(MOVE);

    key = check_button(key);
    key = scr_bar(ACTION, key, scroll_max(), &top, sb_x1, sb_y1, sb_x2, sb_y2, 0, 7, -1);

    if (top != old_top)
    {
      /* the scroll bar was dragged: the selection stays in sight */
      if (top > nfiles - LIST_ROWS) top = (nfiles > LIST_ROWS) ? nfiles - LIST_ROWS : 0;
      if (sel < top) sel = top;
      if (sel >= top + LIST_ROWS) sel = top + LIST_ROWS - 1;
      show_list();
      if (nfiles > 0)
      {
        strcpy(name, files[sel].name);
        show_name();
      }
    }

    /* the mouse on the list: a click selects, a double click takes */
    mouse_state();
    if (ms == PRESSED)
    {
      get_mouse_location();
      row = mouse_y - (wy + LIST_Y);
      if (mouse_x >= wx + LIST_X && mouse_x < wx + LIST_X + LIST_W &&
          row >= 0 && row < LIST_ROWS && top + row < nfiles)
      {
        if (top + row != sel) select_entry(top + row);
        if (dc == DOUBLECLICKED)
        {
          while (mouse_left_pressed() == 1) gcurs(MOVE);
          dc  = RELEASED;
          key = ENTER;
        }
      }
    }
    if (ms == PRESSEDRELEASED) ms = RELEASED;

    switch (key)
    {
      case -1:
       break;

      case ESC:
      case ID_CANCEL:
       done = 1;
       break;

      case ENTER:
      case ID_OK:
       if (activate(filename) == 1)
       {
         result = YES;
         done   = 1;
       }
       break;

      case UPKEY:    select_entry(sel - 1);             break;
      case DOWNKEY:  select_entry(sel + 1);             break;
      case PGUPKEY:  select_entry(sel - LIST_ROWS + 1); break;
      case PGDNKEY:  select_entry(sel + LIST_ROWS - 1); break;
      case HOMEKEY:  select_entry(0);                   break;
      case ENDKEY:   select_entry(nfiles - 1);          break;

      case BS:
       i = (int) strlen(name);
       if (i > 0)
       {
         name[i - 1] = 0;
         show_name();
       }
       break;

      default:
       if (key >= ' ' && key <= '~')
       {
         i = (int) strlen(name);
         if (i < FNAME_MAX)
         {
           name[i]     = (char) toupper(key);
           name[i + 1] = 0;
           show_name();
         }
       }
       break;
    }

    old_top = top;
  }

  txt_curs(0);

  disable_button(b1);
  disable_button(b2);
  destroy_messagebox(wh);
  reactivate_activebuttons();

  /* the drive and the directory as they were */
  gp_setdrive(start_drive);
  gp_chdir(start_dir);

  free(files);
  files = NULL;

  return result;
}
/*--------------------------------------------------------------------------*/
/* any name is taken */
int file_dialog(char *title, char *mask, char *filename)
{
  return dialog(FD_ANY, title, mask, filename);
}

/* a file that is there */
int file_open_dialog(char *title, char *mask, char *filename)
{
  return dialog(FD_OPEN, title, mask, filename);
}

/* a file to write: asks before one is replaced */
int file_save_dialog(char *title, char *mask, char *filename)
{
  return dialog(FD_SAVE, title, mask, filename);
}
/*--------------------------------------------------------------------------*/
