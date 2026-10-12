/*--------------------------------------------------------------------------*/
/* GPORT : what differs between compilers and between 16 and 32 bits.       */
/* For the library's own files only; programs include gui.h.                */
/*--------------------------------------------------------------------------*/
#ifndef GPORT_H
#define GPORT_H

#include "gui.h"

/* registers for a software interrupt (BIOS video 10h, mouse 33h) */
typedef struct
{
  unsigned short ax, bx, cx, dx, si, di, bp, es;
} GREGS;

void          gp_int(int no, GREGS *r);          /* registers in and out      */
unsigned char gp_peekb(unsigned long linear);    /* a byte of the first MB    */
unsigned      gp_peekw(unsigned long linear);
void          gp_peek(unsigned long linear, unsigned char *to, unsigned n);
int           gp_vbe_modeinfo(unsigned mode, unsigned char *info256);
long          gp_ms(void);                       /* a clock in milliseconds   */
int           gp_kbhit(void);
unsigned      gp_readkey(void);                  /* scan code << 8 | ASCII    */
unsigned      gp_shiftstate(void);               /* BIOS shift flags          */
void          gp_delay(int milli);

/* files and directories, for the file dialog */
int           gp_findfirst(const char *mask, char *name, int *is_dir);  /* 0: found */
int           gp_findnext(char *name, int *is_dir);
int           gp_getdrive(void);                 /* 1 = A:                    */
int           gp_setdrive(int drive);            /* 0: there is such a drive  */
int           gp_getcwd(char *buf, int size);    /* 0: ok                     */
int           gp_chdir(const char *dir);         /* 0: ok                     */

void          gui_fatal(const char *message);    /* message, key, exit(-1)    */
int           getkey_wait(void);                 /* wait for a key            */

/*--------------------------------------------------------------------------*/
/* the video card: memory at A000:0 and its ports                           */
/*--------------------------------------------------------------------------*/
#if defined(__DJGPP__)

#include <pc.h>
#include <sys/nearptr.h>
#define GP_VBASE        ((volatile unsigned char *)(0xA0000UL + __djgpp_conventional_base))
#define GP_VFAR
#define gp_outb(p, v)   outportb((p), (v))
#define gp_outw(p, v)   outportw((p), (v))
#define gp_inb(p)       inportb(p)

#elif defined(GUI32)

#include <conio.h>
#define GP_VBASE        ((volatile unsigned char *) 0xA0000UL)
#define GP_VFAR
#define gp_outb(p, v)   outp((p), (v))
#define gp_outw(p, v)   outpw((p), (v))
#define gp_inb(p)       inp(p)

#else

#include <conio.h>
#include <dos.h>
#ifndef MK_FP
#define MK_FP(seg,ofs) ((void far *) (((unsigned long)(seg) << 16) | (unsigned)(ofs)))
#endif
#define GP_VBASE        ((volatile unsigned char far *) MK_FP(0xA000, 0))
#define GP_VFAR         far
#if defined(__TURBOC__)
#define gp_outb(p, v)   outportb((p), (v))
#define gp_outw(p, v)   outport((p), (v))
#define gp_inb(p)       inportb(p)
#else
#define gp_outb(p, v)   outp((p), (v))
#define gp_outw(p, v)   outpw((p), (v))
#define gp_inb(p)       inp(p)
#endif

#endif

/*--------------------------------------------------------------------------*/
/* blocks of pixels saved by GET_IMAGE can be larger than 64K: in a 16-bit  */
/* program they are walked with a huge pointer                              */
/*--------------------------------------------------------------------------*/
#ifdef GUI32
#define GP_HUGE
#else
#if defined(__WATCOMC__)
#define GP_HUGE __huge
#else
#define GP_HUGE huge
#endif
#endif

/*--------------------------------------------------------------------------*/
/* the drawing layer (gfx.c)                                                */
/*--------------------------------------------------------------------------*/
#define GFX_TEXT        0       /* not a graphics mode                       */
#define GFX_PLANAR      1       /* 16 colors in 4 bit planes                 */
#define GFX_PACKED      2       /* 256 colors, a byte per pixel              */

extern int  gfx_kind;
extern int  gfx_mode;           /* the mode number that was set              */

int  gfx_setup(int mode);                /* learn the mode that is set: 1=ok  */
void gfx_char(int ch, int x, int y, int fg, int bg, int rop);  /* bg -1: none */
void gfx_hline(int x1, int x2, int y, int color, int rop);

/* how GLINE's last argument asks for XOR (the VGA's function select value) */
#define ROP_SET         0x00
#define ROP_XOR         0x18

#endif
