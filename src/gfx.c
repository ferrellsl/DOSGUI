/*--------------------------------------------------------------------------*/
/* GFX : the drawing layer                                                  */
/*                                                                          */
/* In the original library these were eleven assembler modules (SETPIX,     */
/* RSETPIX, GETPIX, LINE, ELLIPSE, RELLIPSE, PCELLIP, RCELLIP, DISPCHAR,    */
/* GET_BB, SET_BB) written for one mode: 640 pixels across, 16 colors, all  */
/* of a bit plane within one 64K segment.  Here they are C, and take the    */
/* size of the screen from the mode:                                        */
/*                                                                          */
/*   16 colors (4 bit planes)  640x350, 640x480, 800x600, 1024x768,         */
/*                             1280x1024                                    */
/*   256 colors (byte a pixel) 640x480, 800x600, 1024x768, 1280x1024        */
/*                                                                          */
/* Above 64K a plane or the screen is reached through the VESA BIOS's       */
/* window (bank switching), which works the same in 16 and 32 bits.         */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gui.h"
#include "gport.h"

int gfx_kind = GFX_TEXT;
int gfx_mode = 3;

static int   g_w, g_h;            /* pixels                                 */
static long  g_bpl;               /* bytes from one line to the next        */
static int   g_banked;            /* more than one 64K window of it         */
static int   g_bankmul = 1;       /* window positions per 64K               */
static int   g_bank    = -1;      /* the window position that is set        */

#define FONT_ROWS 16
static unsigned char g_font[256 * FONT_ROWS];
static int           g_fh = 16;

#define GC  0x3CE                 /* VGA graphics controller                */
#define SEQ 0x3C4                 /* VGA sequencer                          */

/*--------------------------------------------------------------------------*/
/* the address of a byte of video memory                                    */
/*--------------------------------------------------------------------------*/
static void set_bank(int bank)
{
  GREGS r;

  memset(&r, 0, sizeof(r));
  r.ax = 0x4F05;
  r.bx = 0;                       /* window A */
  r.dx = (unsigned short) (bank * g_bankmul);
  gp_int(0x10, &r);
  g_bank = bank;
}

static volatile unsigned char GP_VFAR *vp(long off)
{
  if (g_banked)
  {
    int bank = (int) (off >> 16);

    if (bank != g_bank) set_bank(bank);
    off &= 0xFFFFL;
  }
  return GP_VBASE + (unsigned) off;
}
/*--------------------------------------------------------------------------*/
/* learn the mode that has been set: what kind, how large, and its font     */
/*--------------------------------------------------------------------------*/
static void load_font(void)
{
  unsigned      off, seg;
  int           h;
  unsigned long at;
  GREGS         r;

  off = gp_peekw(0x43 * 4UL);             /* the BIOS's font for graphics   */
  seg = gp_peekw(0x43 * 4UL + 2);
  h   = gp_peekb(0x485UL);                /* and its height                 */

  if ((off == 0 && seg == 0) || (h != 8 && h != 14 && h != 16))
  {
    memset(&r, 0, sizeof(r));             /* ask for the 8x16 font in ROM   */
    r.ax = 0x1130;
    r.bx = 0x0600;
    gp_int(0x10, &r);
    seg = r.es;
    off = r.bp;
    h   = 16;
  }

  g_fh = h;
  at   = ((unsigned long) seg << 4) + off;
  gp_peek(at, g_font, (unsigned) (256 * h));
}

int gfx_setup(int mode)
{
  unsigned char info[256];
  int           planes, bits, model;

  gfx_mode  = mode;
  gfx_kind  = GFX_TEXT;
  g_banked  = 0;
  g_bankmul = 1;
  g_bank    = -1;

  if (mode == 0x12 || mode == 0x11)
  {
    gfx_kind = GFX_PLANAR; g_w = 640; g_h = 480; g_bpl = 80;
  }
  else if (mode == 0x10 || mode == 0x0F)
  {
    gfx_kind = GFX_PLANAR; g_w = 640; g_h = 350; g_bpl = 80;
  }
  else if (mode == 0x0E)
  {
    gfx_kind = GFX_PLANAR; g_w = 640; g_h = 200; g_bpl = 80;
  }
  else if (mode == 0x6A)
  {
    gfx_kind = GFX_PLANAR; g_w = 800; g_h = 600; g_bpl = 100;
  }
  else if (mode >= 0x100)
  {
    memset(info, 0, sizeof(info));
    if (gp_vbe_modeinfo((unsigned) mode, info) && (info[0x12] | info[0x13]) != 0)
    {
      g_bpl  = info[0x10] | ((unsigned) info[0x11] << 8);
      g_w    = info[0x12] | ((unsigned) info[0x13] << 8);
      g_h    = info[0x14] | ((unsigned) info[0x15] << 8);
      planes = info[0x18];
      bits   = info[0x19];
      model  = info[0x1B];
      if (info[4] | info[5])
      {
        int gran = info[4] | ((unsigned) info[5] << 8);      /* in KB */

        if (gran > 0 && gran <= 64) g_bankmul = 64 / gran;
      }
      if (bits == 8 && (model == 4 || planes == 1)) gfx_kind = GFX_PACKED;
      else if (bits == 4 || planes == 4)            gfx_kind = GFX_PLANAR;
    }
    else
    {
      /* a BIOS that sets the mode but will not describe it */
      switch (mode)
      {
        case 0x102: gfx_kind = GFX_PLANAR; g_w = 800;  g_h = 600;  break;
        case 0x104: gfx_kind = GFX_PLANAR; g_w = 1024; g_h = 768;  break;
        case 0x106: gfx_kind = GFX_PLANAR; g_w = 1280; g_h = 1024; break;
        case 0x101: gfx_kind = GFX_PACKED; g_w = 640;  g_h = 480;  break;
        case 0x103: gfx_kind = GFX_PACKED; g_w = 800;  g_h = 600;  break;
        case 0x105: gfx_kind = GFX_PACKED; g_w = 1024; g_h = 768;  break;
        case 0x107: gfx_kind = GFX_PACKED; g_w = 1280; g_h = 1024; break;
      }
      g_bpl = (gfx_kind == GFX_PLANAR) ? g_w / 8 : g_w;
    }
    if (gfx_kind == GFX_PLANAR && g_bpl > g_w / 8 + 64) g_bpl = g_w / 8;
    g_banked = (g_bpl * g_h > 65536L);
  }

  if (gfx_kind == GFX_TEXT) return 0;

  load_font();

  font_width   = 8;
  font_height  = g_fh;
  gr_scr_xsize = g_w;
  gr_scr_ysize = g_h;
  return 1;
}
/*--------------------------------------------------------------------------*/
/* a horizontal run of pixels: everything that is filled is made of these   */
/*--------------------------------------------------------------------------*/
void gfx_hline(int x1, int x2, int y, int color, int rop)
{
  long off;
  int  t, n;

  if (gfx_kind == GFX_TEXT) return;
  if (x1 > x2) { t = x1; x1 = x2; x2 = t; }
  if (y < 0 || y >= g_h || x2 < 0 || x1 >= g_w) return;
  if (x1 < 0)    x1 = 0;
  if (x2 >= g_w) x2 = g_w - 1;

  if (gfx_kind == GFX_PLANAR)
  {
    volatile unsigned char GP_VFAR *p;
    unsigned lmask = 0xFF >> (x1 & 7);
    unsigned rmask = (0xFF << (7 - (x2 & 7))) & 0xFF;

    off = (long) y * g_bpl + (x1 >> 3);
    n   = (x2 >> 3) - (x1 >> 3);                  /* bytes after the first   */

    gp_outw(GC, ((color & 15) << 8) | 0x00);      /* set/reset = the color   */
    gp_outw(GC, 0x0F01);                          /* for all four planes     */
    gp_outw(GC, ((rop & 0x18) << 8) | 0x03);      /* replace or XOR          */

    if (n == 0)
    {
      gp_outw(GC, ((lmask & rmask) << 8) | 0x08);
      p = vp(off); t = *p; *p = 0xFF;
    }
    else
    {
      gp_outw(GC, (lmask << 8) | 0x08);
      p = vp(off); t = *p; *p = 0xFF;
      off++;
      if (n > 1)
      {
        gp_outw(GC, 0xFF08);
        for (n--; n > 0; n--)
        {
          p = vp(off); t = *p; *p = 0xFF;
          off++;
        }
      }
      gp_outw(GC, (rmask << 8) | 0x08);
      p = vp(off); t = *p; *p = 0xFF;
    }

    gp_outw(GC, 0xFF08);
    gp_outw(GC, 0x0003);
    gp_outw(GC, 0x0001);
    gp_outw(GC, 0x0000);
  }
  else
  {
    volatile unsigned char GP_VFAR *p;
    unsigned char c = (unsigned char) (color & 15);

    off = (long) y * g_bpl + x1;
    n   = x2 - x1 + 1;
    while (n > 0)
    {
      /* as many as there are before the end of the 64K window */
      long room = 0x10000L - (off & 0xFFFFL);
      int  run  = (room < n) ? (int) room : n;

      p    = vp(off);
      off += run;
      n   -= run;
      if (rop & 0x18) { for (; run > 0; run--) *p++ ^= c; }
      else            { for (; run > 0; run--) *p++  = c; }
    }
  }
}
/*--------------------------------------------------------------------------*/
/* single pixels; a run of them (a line, an ellipse) sets the card up once  */
/*--------------------------------------------------------------------------*/
static void dots_begin(int color, int rop)
{
  if (gfx_kind == GFX_PLANAR)
  {
    gp_outw(GC, ((color & 15) << 8) | 0x00);
    gp_outw(GC, 0x0F01);
    gp_outw(GC, ((rop & 0x18) << 8) | 0x03);
  }
}

static void dot(int x, int y, int color, int rop)
{
  volatile unsigned char GP_VFAR *p;
  int t;

  if (x < 0 || y < 0 || x >= g_w || y >= g_h) return;

  if (gfx_kind == GFX_PLANAR)
  {
    gp_outw(GC, ((0x80 >> (x & 7)) << 8) | 0x08);
    p = vp((long) y * g_bpl + (x >> 3));
    t = *p; *p = 0xFF;
  }
  else if (gfx_kind == GFX_PACKED)
  {
    p = vp((long) y * g_bpl + x);
    if (rop & 0x18) *p ^= (unsigned char) (color & 15);
    else            *p  = (unsigned char) (color & 15);
  }
}

static void dots_end(void)
{
  if (gfx_kind == GFX_PLANAR)
  {
    gp_outw(GC, 0xFF08);
    gp_outw(GC, 0x0003);
    gp_outw(GC, 0x0001);
    gp_outw(GC, 0x0000);
  }
}
/*--------------------------------------------------------------------------*/
void SETPIX(int x, int y, char color)
{
  if (gfx_kind == GFX_TEXT) return;
  dots_begin(color, ROP_SET);
  dot(x, y, color, ROP_SET);
  dots_end();
}

void RSETPIX(int x, int y, char color)           /* XOR the color value */
{
  if (gfx_kind == GFX_TEXT) return;
  dots_begin(color, ROP_XOR);
  dot(x, y, color, ROP_XOR);
  dots_end();
}

int GETPIX(int x, int y)
{
  volatile unsigned char GP_VFAR *p;
  int plane, c = 0;

  if (gfx_kind == GFX_TEXT || x < 0 || y < 0 || x >= g_w || y >= g_h) return 0;

  if (gfx_kind == GFX_PACKED)
  {
    p = vp((long) y * g_bpl + x);
    return *p;
  }

  p = vp((long) y * g_bpl + (x >> 3));
  gp_outw(GC, 0x0005);                            /* read mode 0 */
  for (plane = 3; plane >= 0; plane--)
  {
    gp_outw(GC, (plane << 8) | 0x04);
    c <<= 1;
    if (*p & (0x80 >> (x & 7))) c |= 1;
  }
  gp_outw(GC, 0x0004);
  return c;
}
/*--------------------------------------------------------------------------*/
/* a line from x1,y1 to x2,y2; rop 0 draws, 0x18 XORs                       */
/*--------------------------------------------------------------------------*/
void GLINE(int x1, int y1, int x2, int y2, int color, char rop)
{
  int dx, dy, sy, d, incr1, incr2, t, steep;

  if (gfx_kind == GFX_TEXT) return;

  if (y1 == y2)
  {
    gfx_hline(x1, x2, y1, color, rop);
    return;
  }

  if (x1 > x2)                          /* always left to right */
  {
    t = x1; x1 = x2; x2 = t;
    t = y1; y1 = y2; y2 = t;
  }

  dx = x2 - x1;
  dy = y2 - y1;
  sy = 1;
  if (dy < 0) { dy = -dy; sy = -1; }

  dots_begin(color, rop);

  if (dx == 0)
  {
    for (t = dy + 1; t > 0; t--) { dot(x1, y1, color, rop); y1 += sy; }
  }
  else
  {
    steep = (dy > dx);
    if (steep) { t = dx; dx = dy; dy = t; }

    incr1 = 2 * dy;
    d     = incr1 - dx;
    incr2 = d - dx;

    for (t = dx + 1; t > 0; t--)
    {
      dot(x1, y1, color, rop);
      if (d < 0)
      {
        d += incr1;
        if (steep) y1 += sy; else x1++;
      }
      else
      {
        d += incr2;
        x1++;
        y1 += sy;
      }
    }
  }

  dots_end();
}
/*--------------------------------------------------------------------------*/
/* ellipses: center x,y and the two radii a (across) and b (down); the      */
/* "cut" ones draw only what lies inside x1cut,y1cut - x2cut,y2cut          */
/*--------------------------------------------------------------------------*/
static int e_x1, e_y1, e_x2, e_y2;

static void edot(int x, int y, int color, int rop)
{
  if (x < e_x1 || x > e_x2 || y < e_y1 || y > e_y2) return;
  dot(x, y, color, rop);
}

static void set4(int x, int y, int xc, int yc, int color, int rop)
{
  if (x == 0)
  {
    edot(xc, yc + y, color, rop);
    if (y != 0) edot(xc, yc - y, color, rop);
  }
  else if (y == 0)
  {
    edot(xc + x, yc, color, rop);
    edot(xc - x, yc, color, rop);
  }
  else
  {
    edot(xc + x, yc + y, color, rop);
    edot(xc - x, yc + y, color, rop);
    edot(xc + x, yc - y, color, rop);
    edot(xc - x, yc - y, color, rop);
  }
}

static void ellipse(int xc, int yc, int a0, int b0, int color, int rop)
{
  int  x = 0;
  int  y = b0;
  long a = a0, b = b0;
  long asq = a * a, two_asq = 2 * asq;
  long bsq = b * b, two_bsq = 2 * bsq;
  long d, dx, dy;

  if (gfx_kind == GFX_TEXT) return;
  dots_begin(color, rop);

  d  = bsq - asq * b + asq / 4L;
  dx = 0;
  dy = two_asq * b;

  while (dx < dy)
  {
    set4(x, y, xc, yc, color, rop);
    if (d > 0L)
    {
      y--;
      dy -= two_asq;
      d  -= dy;
    }
    x++;
    dx += two_bsq;
    d  += bsq + dx;
  }

  d += (3L * (asq - bsq) / 2L - (dx + dy)) / 2L;

  while (y >= 0)
  {
    set4(x, y, xc, yc, color, rop);
    if (d < 0L)
    {
      x++;
      dx += two_bsq;
      d  += dx;
    }
    y--;
    dy -= two_asq;
    d  += asq - dy;
  }

  dots_end();
}

void ELLIPSE(int x, int y, int a, int b, int color)
{
  e_x1 = 0; e_y1 = 0; e_x2 = g_w - 1; e_y2 = g_h - 1;
  ellipse(x, y, a, b, color, ROP_SET);
}

void RELLIPSE(int x, int y, int a, int b, int color)
{
  e_x1 = 0; e_y1 = 0; e_x2 = g_w - 1; e_y2 = g_h - 1;
  ellipse(x, y, a, b, color, ROP_XOR);
}

int PCELLIP(int x, int y, int a, int b, int x1cut, int y1cut, int x2cut, int y2cut, int color)
{
  e_x1 = x1cut; e_y1 = y1cut; e_x2 = x2cut; e_y2 = y2cut;
  ellipse(x, y, a, b, color, ROP_SET);
  return 0;
}

int RCELLIP(int x, int y, int a, int b, int x1cut, int y1cut, int x2cut, int y2cut, int color)
{
  e_x1 = x1cut; e_y1 = y1cut; e_x2 = x2cut; e_y2 = y2cut;
  ellipse(x, y, a, b, color, ROP_XOR);
  return 0;
}
/*--------------------------------------------------------------------------*/
/* a character of the BIOS's font with its top left at x,y.                 */
/* bg -1 leaves the background as it is; rop 0x18 XORs                      */
/*--------------------------------------------------------------------------*/
void gfx_char(int ch, int x, int y, int fg, int bg, int rop)
{
  const unsigned char *glyph;
  int row, i, t;

  if (gfx_kind == GFX_TEXT) return;
  if (x <= -8 || y <= -g_fh || x >= g_w || y >= g_h) return;

  glyph = g_font + (ch & 0xFF) * g_fh;

  if (gfx_kind == GFX_PLANAR && x >= 0 && x + 8 <= g_w)
  {
    volatile unsigned char GP_VFAR *p;
    int      sh = x & 7;
    unsigned bits, m;
    long     off = (long) y * g_bpl + (x >> 3);

    gp_outw(GC, 0x0205);                          /* write mode 2: the byte  */
    gp_outw(GC, ((rop & 0x18) << 8) | 0x03);      /* written is the color    */

    for (row = 0; row < g_fh; row++, off += g_bpl)
    {
      if (y + row < 0 || y + row >= g_h) continue;
      bits = glyph[row];

      m = bits >> sh;                             /* in the first byte       */
      if (m)
      {
        gp_outw(GC, (m << 8) | 0x08);
        p = vp(off); t = *p; *p = (unsigned char) fg;
      }
      if (bg >= 0)
      {
        m = (~bits & 0xFF) >> sh;
        if (m)
        {
          gp_outw(GC, (m << 8) | 0x08);
          p = vp(off); t = *p; *p = (unsigned char) bg;
        }
      }

      if (sh)                                     /* and the rest in the next */
      {
        m = (bits << (8 - sh)) & 0xFF;
        if (m)
        {
          gp_outw(GC, (m << 8) | 0x08);
          p = vp(off + 1); t = *p; *p = (unsigned char) fg;
        }
        if (bg >= 0)
        {
          m = ((~bits & 0xFF) << (8 - sh)) & 0xFF;
          if (m)
          {
            gp_outw(GC, (m << 8) | 0x08);
            p = vp(off + 1); t = *p; *p = (unsigned char) bg;
          }
        }
      }
    }

    gp_outw(GC, 0xFF08);
    gp_outw(GC, 0x0005);
    gp_outw(GC, 0x0003);
    return;
  }

  if (gfx_kind == GFX_PACKED && x >= 0 && x + 8 <= g_w)
  {
    volatile unsigned char GP_VFAR *p;
    long off   = (long) y * g_bpl + x;
    int  whole = 1;               /* no row lies across two 64K windows     */

    if (g_banked)
    {
      for (row = 0; row < g_fh; row++)
      {
        if (((off + row * g_bpl) & 0xFFFFL) > 0xFFF8L) whole = 0;
      }
    }

    for (row = 0; whole && row < g_fh; row++, off += g_bpl)
    {
      unsigned bits = glyph[row];

      if (y + row < 0 || y + row >= g_h) continue;

      p = vp(off);
      for (i = 0; i < 8; i++, p++, bits <<= 1)
      {
        if (bits & 0x80)
        {
          if (rop & 0x18) *p ^= (unsigned char) fg; else *p = (unsigned char) fg;
        }
        else if (bg >= 0)
        {
          if (rop & 0x18) *p ^= (unsigned char) bg; else *p = (unsigned char) bg;
        }
      }
    }
    if (whole) return;
  }

  /* a character that hangs over an edge (of the screen, or of a 64K window */
  /* in 256 colors): pixel by pixel                                          */
  dots_begin(fg, rop);
  for (row = 0; row < g_fh; row++)
  {
    for (i = 0; i < 8; i++)
    {
      if (glyph[row] & (0x80 >> i))
      {
        if (gfx_kind == GFX_PLANAR) gp_outw(GC, ((fg & 15) << 8) | 0x00);
        dot(x + i, y + row, fg, rop);
      }
      else if (bg >= 0)
      {
        if (gfx_kind == GFX_PLANAR) gp_outw(GC, ((bg & 15) << 8) | 0x00);
        dot(x + i, y + row, bg, rop);
      }
    }
  }
  dots_end();
}

void DISPLAYCHAR(int ch, int x, int y, int fg, int bg)
{
  gfx_char(ch, x, y, fg & 15, bg & 15, ROP_SET);
}
/*--------------------------------------------------------------------------*/
/* saving and restoring a block of pixels                                   */
/*                                                                          */
/* what is saved: width, height (two words each... as four bytes), then     */
/* line by line  - 16 colors : the four planes, each (width+7)/8 bytes,     */
/*                             with the block's left edge in the high bit   */
/*               - 256 colors: a byte per pixel                             */
/*--------------------------------------------------------------------------*/
#define BLOCK_HEADER 8

long get_block_size(int x1, int y1, int x2, int y2)
{
  long w, h;

  w = (long) x2 - x1; if (w < 0) w = -w;
  h = (long) y2 - y1; if (h < 0) h = -h;
  w += 1;
  h += 2;                                 /* (a line to spare)            */

  if (gfx_kind == GFX_PACKED) return BLOCK_HEADER + (w + 8) * h;
  return BLOCK_HEADER + ((w + 15) / 8) * 4L * h;
}

int GET_IMAGE(int x1, int y1, int x2, int y2, char GUI_FAR *m)
{
  unsigned char GP_HUGE          *b = (unsigned char GP_HUGE *) m;
  volatile unsigned char GP_VFAR *p;
  int  t, w, h, nb, row, plane, i, sh, lastbyte;
  long off;

  if (gfx_kind == GFX_TEXT || m == NULL) return 0;
  if (x1 > x2) { t = x1; x1 = x2; x2 = t; }
  if (y1 > y2) { t = y1; y1 = y2; y2 = t; }
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 >= g_w) x2 = g_w - 1;
  if (y2 >= g_h) y2 = g_h - 1;

  w = x2 - x1 + 1;
  h = y2 - y1 + 1;
  if (w <= 0 || h <= 0) { w = 0; h = 0; }

  b[0] = (unsigned char) w; b[1] = (unsigned char) (w >> 8);
  b[2] = (unsigned char) h; b[3] = (unsigned char) (h >> 8);
  b[4] = (unsigned char) gfx_kind; b[5] = b[6] = b[7] = 0;
  b += BLOCK_HEADER;

  if (gfx_kind == GFX_PACKED)
  {
    for (row = 0; row < h; row++)
    {
      off = (long) (y1 + row) * g_bpl + x1;
      for (i = 0; i < w; i++) { p = vp(off++); *b++ = *p; }
    }
    return (int) (BLOCK_HEADER + (long) w * h);
  }

  nb       = (w + 7) / 8;
  sh       = x1 & 7;
  lastbyte = x2 >> 3;

  gp_outw(GC, 0x0005);                            /* read mode 0 */
  for (row = 0; row < h; row++)
  {
    for (plane = 0; plane < 4; plane++)
    {
      int col = x1 >> 3;

      gp_outw(GC, (plane << 8) | 0x04);
      off = (long) (y1 + row) * g_bpl + col;
      for (i = 0; i < nb; i++, col++, off++)
      {
        unsigned v;

        p = vp(off);
        v = (unsigned) *p << 8;
        if (sh && col < lastbyte)
        {
          p = vp(off + 1);
          v |= *p;
        }
        *b++ = (unsigned char) ((v << sh) >> 8);
      }
    }
  }
  gp_outw(GC, 0x0004);

  return (int) (BLOCK_HEADER + (long) nb * 4 * h);
}

void RESTORE_IMAGE(char GUI_FAR *m, int x, int y)
{
  unsigned char GP_HUGE          *b = (unsigned char GP_HUGE *) m;
  volatile unsigned char GP_VFAR *p;
  int  w, h, vw, vh, nb, nd, row, plane, j, sh, t;
  long off;

  if (gfx_kind == GFX_TEXT || m == NULL) return;

  w = b[0] | ((int) b[1] << 8);
  h = b[2] | ((int) b[3] << 8);
  if (b[4] != (unsigned char) gfx_kind) return;   /* saved in another mode  */
  b += BLOCK_HEADER;

  if (x < 0 || y < 0 || x >= g_w || y >= g_h || w <= 0 || h <= 0) return;
  vw = (x + w > g_w) ? g_w - x : w;               /* what is on the screen  */
  vh = (y + h > g_h) ? g_h - y : h;

  if (gfx_kind == GFX_PACKED)
  {
    for (row = 0; row < vh; row++)
    {
      unsigned char GP_HUGE *s = b + (long) row * w;

      off = (long) (y + row) * g_bpl + x;
      for (j = 0; j < vw; j++) { p = vp(off++); *p = *s++; }
    }
    return;
  }

  nb = (w + 7) / 8;                               /* bytes saved per plane  */
  sh = x & 7;
  nd = (sh + vw + 7) / 8;                         /* bytes written to       */

  gp_outw(GC, 0x0005);                            /* write mode 0           */
  gp_outw(GC, 0x0003);                            /* replace                */
  gp_outw(GC, 0x0001);                            /* no set/reset           */

  for (row = 0; row < vh; row++)
  {
    for (plane = 0; plane < 4; plane++)
    {
      unsigned char GP_HUGE *s = b + ((long) row * 4 + plane) * nb;
      unsigned prev = 0, cur, data, mask;

      gp_outw(SEQ, ((1 << plane) << 8) | 0x02);
      off = (long) (y + row) * g_bpl + (x >> 3);

      for (j = 0; j < nd; j++, off++)
      {
        cur  = (j < nb) ? s[j] : 0;
        data = ((prev << (8 - sh)) | (cur >> sh)) & 0xFF;
        prev = cur;

        mask = 0xFF;
        if (j == 0)      mask &= 0xFF >> sh;
        if (j == nd - 1) mask &= (0xFF << (7 - ((sh + vw - 1) & 7))) & 0xFF;

        gp_outw(GC, (mask << 8) | 0x08);
        p = vp(off); t = *p; *p = (unsigned char) data;
      }
    }
  }

  gp_outw(SEQ, 0x0F02);
  gp_outw(GC, 0xFF08);
}
/*--------------------------------------------------------------------------*/
