/*--------------------------------------------------------------------------*/
/* PERC_BAR : the percent bar, and text that is drawn without a background  */
/*                                                                          */
/* Rebuilt from the Plain Design GUI library ver 1.01 (BC_GUI.LIB, module   */
/* PERC_BAR).  The original carried its own copy of the 8x16 font for the   */
/* text functions here; they now use the font of the mode, like the rest.   */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gui.h"
#include "gport.h"

/*--------------------------------------------------------------------------*/
/* Displays a bar showing the percentage of  a / b.                         */
/* The first call must have a = 0: the bar is drawn, with a Cancel button.  */
/* The last call must have a = -1: what the bar covered is put back.        */
/* Returns -1 when Cancel or ESC was pressed.                               */
/*--------------------------------------------------------------------------*/
int show_percent_bar(int a, int b, int x, int y, int xw, int yw,
                     int border_color, int bar_color, int rev_num_color)
{
  static char GUI_FAR *under;     /* what the bar covers                    */
  static char          shown[8];  /* the percentage that is displayed       */
  static int           cancel;    /* the Cancel button                      */
  char now[8];
  int  ty = y + (yw - 16) / 2;
  int  tx;
  int  w;

  if (a == 0)
  {
    under = (char GUI_FAR *) gui_farmalloc(get_block_size(x, y, x + xw - 1, y + yw - 1));
    if (under == NULL)
    {
      gui_fatal("\n \a not enough memory to allocate for percent rectangle");
    }

    hide_mouse_cursor();
    GET_IMAGE(x, y, x + xw - 1, y + yw - 1, under);
    show_mouse_cursor();

    grect(x, y, x + xw - 1, y + yw - 1, border_color);

    sprintf(shown, "%d%%", a);
    tx = x + ((xw - (int) strlen(shown) * 8 - 2) >> 1);
    rgpls(tx + 1, ty + 1, (unsigned char *) shown, rev_num_color);

    deactivate_activebuttons();
    cancel = draw_button("Cancel", x + (xw - 90) / 2 + 1000, ty + yw + 1004, 90, 23, ESC, ESC);
    return 0;
  }

  if (a == -1)
  {
    hide_mouse_cursor();
    RESTORE_IMAGE(under, x, y);
    show_mouse_cursor();
    gui_farfree(under);
    under = NULL;

    disable_button2(cancel, 15);
    reactivate_activebuttons();
    return -1;
  }

  if (check_button(getkey()) == ESC) return -1;

  sprintf(now, "%d%%", (int) ((long) a * 100L / (b ? b : 1)));
  if (strcmp(now, shown) == 0) return 0;

  /* the number is XORed: drawing it again takes it away */
  tx = x + ((xw - (int) strlen(shown) * 8 - 2) >> 1);
  rgpls(tx + 1, ty + 1, (unsigned char *) shown, rev_num_color);

  w = (int) ((long) a * (xw - 2) / (b ? b : 1)) + 1;
  gfillrect(x + 1, y + 1, x + w - 1, y + yw - 2, bar_color);

  strcpy(shown, now);
  tx = x + ((xw - (int) strlen(shown) * 8 - 2) >> 1);
  rgpls(tx + 1, ty + 1, (unsigned char *) shown, rev_num_color);

  return 0;
}
/*--------------------------------------------------------------------------*/
/* text that XORs the color value (gx,gy are graphics coordinates)          */
/*--------------------------------------------------------------------------*/
void rgwritechar(int gx, int gy, int c, int color)
{
  gfx_char(c, gx, gy, color & 15, -1, ROP_XOR);
}

void rgpls(int gx, int gy, unsigned char *string, int color)
{
  int i;

  hide_mouse_cursor();
  for (i = 0; (unsigned) i < strlen((char *) string); i++)
  {
    rgwritechar(gx + i * 8, gy, string[i], color);
  }
  show_mouse_cursor();
}
/*--------------------------------------------------------------------------*/
/* text with no background color; color is between 0 and 15                 */
/*--------------------------------------------------------------------------*/
void gwritechar2(int gx, int gy, int c, int color)
{
  gfx_char(c, gx, gy, color & 15, -1, ROP_SET);
}

/* a ^ in the string underlines the letter after it */
void gpls2(int gx, int gy, unsigned char *string, int color)
{
  unsigned char text[80];
  int  hot = -1;
  int  j   = 0;
  int  md;
  int  i;

  memset(text, 0, sizeof(text));

  if (initvar == 0)
  {
    gui_fatal("\a \n Display borad not initialized");
  }

  md = mouse_display;
  if (mouse_display == 1) hide_mouse_cursor();

  for (i = 0; (unsigned) i < strlen((char *) string) && j < 79; i++)
  {
    if (string[i] == '^') hot = i;
    else                  text[j++] = string[i];
  }

  for (i = 0; (unsigned) i < strlen((char *) text); i++)
  {
    gwritechar2(gx + i * font_width, gy, text[i], color);
  }

  if (hot != -1)
  {
    gr_line(gx + hot * 8, gy + 13, gx + hot * 8 + 6, gy + 13, color & 0xff);
  }

  if (md == 1) show_mouse_cursor();
}
/*--------------------------------------------------------------------------*/
