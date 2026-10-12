/*--------------------------------------------------------------------------*/
/* GPORT : interrupts, low memory, clock, keyboard and memory for           */
/*         Watcom (16 and 32 bit), DJGPP and the old 16-bit compilers       */
/*--------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gui.h"
#include "gport.h"

/*==========================================================================*/
#if defined(__DJGPP__)
/*==========================================================================*/
#include <dpmi.h>
#include <go32.h>
#include <sys/farptr.h>
#include <sys/movedata.h>
#include <bios.h>
#include <conio.h>
#include <unistd.h>

static int near_ok;

static void need_near(void)
{
  if (!near_ok)
  {
    if (!__djgpp_nearptr_enable())
    {
      printf("\n cannot reach the video memory (no near pointers)");
      exit(-1);
    }
    near_ok = 1;
  }
}

void gp_int(int no, GREGS *r)
{
  __dpmi_regs d;

  need_near();
  memset(&d, 0, sizeof(d));
  d.x.ax = r->ax; d.x.bx = r->bx; d.x.cx = r->cx; d.x.dx = r->dx;
  d.x.si = r->si; d.x.di = r->di; d.x.bp = r->bp; d.x.es = r->es;
  __dpmi_int(no, &d);
  r->ax = d.x.ax; r->bx = d.x.bx; r->cx = d.x.cx; r->dx = d.x.dx;
  r->si = d.x.si; r->di = d.x.di; r->bp = d.x.bp; r->es = d.x.es;
}

void gp_peek(unsigned long linear, unsigned char *to, unsigned n)
{
  dosmemget(linear, n, to);
}

int gp_vbe_modeinfo(unsigned mode, unsigned char *info256)
{
  __dpmi_regs d;

  memset(&d, 0, sizeof(d));
  d.x.ax = 0x4F01;
  d.x.cx = mode;
  d.x.es = __tb >> 4;
  d.x.di = __tb & 15;
  __dpmi_int(0x10, &d);
  if (d.x.ax != 0x004F) return 0;
  dosmemget(__tb, 256, info256);
  return 1;
}

int      gp_kbhit(void)      { return kbhit(); }
unsigned gp_readkey(void)    { return (unsigned) bioskey(0); }
unsigned gp_shiftstate(void) { return (unsigned) bioskey(2); }
void     gp_delay(int milli) { if (milli > 0) usleep((unsigned long) milli * 1000UL); }

/*==========================================================================*/
#elif defined(GUI32)                         /* Watcom, 32 bits (DOS/4GW)   */
/*==========================================================================*/
#include <i86.h>
#include <dos.h>
#include <conio.h>
#include <bios.h>

/* a real mode interrupt through DPMI (int 31h, function 0300h): the BIOS   */
/* gets real segments, which a plain int386() cannot give it                */
#pragma pack(push, 1)
typedef struct
{
  unsigned long  edi, esi, ebp, reserved, ebx, edx, ecx, eax;
  unsigned short flags, es, ds, fs, gs, ip, cs, sp, ss;
} RMI;
#pragma pack(pop)

static unsigned short dos_seg;       /* 512 bytes of DOS memory for the BIOS */
static unsigned short dos_sel;

static void need_dos_block(void)
{
  union REGS r;

  if (dos_seg) return;
  memset(&r, 0, sizeof(r));
  r.x.eax = 0x0100;
  r.x.ebx = 32;                      /* paragraphs */
  int386(0x31, &r, &r);
  if (r.x.cflag)
  {
    printf("\n no DOS memory for the video BIOS");
    exit(-1);
  }
  dos_seg = (unsigned short) r.x.eax;
  dos_sel = (unsigned short) r.x.edx;
}

static void real_int(int no, RMI *rmi)
{
  union REGS   r;
  struct SREGS s;

  segread(&s);
  memset(&r, 0, sizeof(r));
  r.x.eax = 0x0300;
  r.x.ebx = (unsigned) no;
  r.x.ecx = 0;
  s.es    = s.ds;
  r.x.edi = (unsigned) rmi;
  int386x(0x31, &r, &r, &s);
}

void gp_int(int no, GREGS *r)
{
  RMI rmi;

  memset(&rmi, 0, sizeof(rmi));
  rmi.eax = r->ax; rmi.ebx = r->bx; rmi.ecx = r->cx; rmi.edx = r->dx;
  rmi.esi = r->si; rmi.edi = r->di; rmi.ebp = r->bp; rmi.es  = r->es;
  real_int(no, &rmi);
  r->ax = (unsigned short) rmi.eax; r->bx = (unsigned short) rmi.ebx;
  r->cx = (unsigned short) rmi.ecx; r->dx = (unsigned short) rmi.edx;
  r->si = (unsigned short) rmi.esi; r->di = (unsigned short) rmi.edi;
  r->bp = (unsigned short) rmi.ebp; r->es = rmi.es;
}

void gp_peek(unsigned long linear, unsigned char *to, unsigned n)
{
  memcpy(to, (void *) linear, n);    /* the first megabyte is where it says */
}

int gp_vbe_modeinfo(unsigned mode, unsigned char *info256)
{
  RMI rmi;

  need_dos_block();
  memset((void *) ((unsigned long) dos_seg << 4), 0, 256);
  memset(&rmi, 0, sizeof(rmi));
  rmi.eax = 0x4F01;
  rmi.ecx = mode;
  rmi.es  = dos_seg;
  rmi.edi = 0;
  real_int(0x10, &rmi);
  if ((rmi.eax & 0xFFFF) != 0x004F) return 0;
  memcpy(info256, (void *) ((unsigned long) dos_seg << 4), 256);
  return 1;
}

int      gp_kbhit(void)      { return kbhit(); }
unsigned gp_readkey(void)    { return _bios_keybrd(_KEYBRD_READ); }
unsigned gp_shiftstate(void) { return _bios_keybrd(_KEYBRD_SHIFTSTATUS); }
void     gp_delay(int milli) { if (milli > 0) delay(milli); }

/*==========================================================================*/
#else                                        /* 16 bits                     */
/*==========================================================================*/
#include <malloc.h>
#include <dos.h>
#include <conio.h>
#include <bios.h>

void gp_int(int no, GREGS *r)
{
#if defined(__WATCOMC__)
  union REGPACK  p;
  struct SREGS   s;

  segread(&s);
  memset(&p, 0, sizeof(p));
  p.w.ax = r->ax; p.w.bx = r->bx; p.w.cx = r->cx; p.w.dx = r->dx;
  p.w.si = r->si; p.w.di = r->di; p.w.bp = r->bp;
  p.w.es = r->es; p.w.ds = s.ds;
  intr(no, &p);
  r->ax = p.w.ax; r->bx = p.w.bx; r->cx = p.w.cx; r->dx = p.w.dx;
  r->si = p.w.si; r->di = p.w.di; r->bp = p.w.bp; r->es = p.w.es;
#else
  union REGS   x;
  struct SREGS s;

  segread(&s);
  x.x.ax = r->ax; x.x.bx = r->bx; x.x.cx = r->cx; x.x.dx = r->dx;
  x.x.si = r->si; x.x.di = r->di; s.es = r->es;
  int86x(no, &x, &x, &s);
  r->ax = x.x.ax; r->bx = x.x.bx; r->cx = x.x.cx; r->dx = x.x.dx;
  r->si = x.x.si; r->di = x.x.di; r->es = s.es;     /* (bp is not returned) */
#endif
}

void gp_peek(unsigned long linear, unsigned char *to, unsigned n)
{
  unsigned char far *p = (unsigned char far *) MK_FP((unsigned) (linear >> 4), (unsigned) (linear & 15));

  while (n--) *to++ = *p++;
}

int gp_vbe_modeinfo(unsigned mode, unsigned char *info256)
{
  static unsigned char info[256];
  unsigned char far   *p = (unsigned char far *) info;
  GREGS                r;

  memset(info, 0, sizeof(info));
  memset(&r, 0, sizeof(r));
  r.ax = 0x4F01;
  r.cx = mode;
  r.es = FP_SEG(p);
  r.di = FP_OFF(p);
  gp_int(0x10, &r);
  if (r.ax != 0x004F) return 0;
  memcpy(info256, info, 256);
  return 1;
}

int gp_kbhit(void) { return kbhit(); }

#if defined(__TURBOC__)
unsigned gp_readkey(void)    { return (unsigned) bioskey(0); }
unsigned gp_shiftstate(void) { return (unsigned) bioskey(2); }
#else
unsigned gp_readkey(void)    { return _bios_keybrd(_KEYBRD_READ); }
unsigned gp_shiftstate(void) { return _bios_keybrd(_KEYBRD_SHIFTSTATUS); }
#endif

void gp_delay(int milli)
{
#if defined(__WATCOMC__) || defined(__TURBOC__)
  if (milli > 0) delay(milli);
#else
  long until = gp_ms() + milli;

  while (gp_ms() < until) ;
#endif
}

/*==========================================================================*/
#endif
/*==========================================================================*/

/*--------------------------------------------------------------------------*/
/* files and directories                                                    */
/*--------------------------------------------------------------------------*/
#if defined(__TURBOC__)

#include <dir.h>

static struct ffblk find_block;

int gp_findfirst(const char *mask, char *name, int *is_dir)
{
  if (findfirst(mask, &find_block, FA_DIREC | FA_RDONLY | FA_ARCH) != 0) return -1;
  strcpy(name, find_block.ff_name);
  *is_dir = (find_block.ff_attrib & FA_DIREC) != 0;
  return 0;
}

int gp_findnext(char *name, int *is_dir)
{
  if (findnext(&find_block) != 0) return -1;
  strcpy(name, find_block.ff_name);
  *is_dir = (find_block.ff_attrib & FA_DIREC) != 0;
  return 0;
}

int gp_getdrive(void) { return getdisk() + 1; }

int gp_setdrive(int drive)
{
  setdisk(drive - 1);
  return (getdisk() + 1 == drive) ? 0 : -1;
}

int gp_getcwd(char *buf, int size) { return getcwd(buf, size) ? 0 : -1; }
int gp_chdir(const char *dir)      { return chdir(dir); }

#else

#include <dos.h>
#if defined(__DJGPP__)
#include <unistd.h>
#else
#include <direct.h>
#endif

static struct find_t find_block;

int gp_findfirst(const char *mask, char *name, int *is_dir)
{
  if (_dos_findfirst(mask, _A_SUBDIR | _A_RDONLY | _A_ARCH, &find_block) != 0) return -1;
  strcpy(name, find_block.name);
  *is_dir = (find_block.attrib & _A_SUBDIR) != 0;
  return 0;
}

int gp_findnext(char *name, int *is_dir)
{
  if (_dos_findnext(&find_block) != 0) return -1;
  strcpy(name, find_block.name);
  *is_dir = (find_block.attrib & _A_SUBDIR) != 0;
  return 0;
}

int gp_getdrive(void)
{
  unsigned drive = 0;

  _dos_getdrive(&drive);
  return (int) drive;
}

int gp_setdrive(int drive)
{
  unsigned total = 0;
  unsigned now   = 0;

  _dos_setdrive((unsigned) drive, &total);
  _dos_getdrive(&now);
  return ((int) now == drive) ? 0 : -1;
}

int gp_getcwd(char *buf, int size) { return getcwd(buf, size) ? 0 : -1; }
int gp_chdir(const char *dir)      { return chdir(dir); }

#endif
/*--------------------------------------------------------------------------*/
unsigned char gp_peekb(unsigned long linear)
{
  unsigned char b;

  gp_peek(linear, &b, 1);
  return b;
}

unsigned gp_peekw(unsigned long linear)
{
  unsigned char b[2];

  gp_peek(linear, b, 2);
  return b[0] | ((unsigned) b[1] << 8);
}
/*--------------------------------------------------------------------------*/
/* milliseconds, from the BIOS's count of timer ticks (18.2 a second): good */
/* for the blink of a cursor and the delay of a double click                */
/*--------------------------------------------------------------------------*/
long gp_ms(void)
{
  unsigned char b[4];
  unsigned long ticks;

  gp_peek(0x46CUL, b, 4);
  ticks = b[0] | ((unsigned long) b[1] << 8) | ((unsigned long) b[2] << 16) | ((unsigned long) b[3] << 24);
  return (long) (ticks * 55UL);
}
/*--------------------------------------------------------------------------*/
int getkey_wait(void)
{
  unsigned k = gp_readkey();

  return (k & 0xff) ? (int) (k & 0xff) : (int) (k >> 8) + 256;
}
/*--------------------------------------------------------------------------*/
/* the library's way with errors it cannot go on from                       */
/*--------------------------------------------------------------------------*/
void gui_fatal(const char *message)
{
  if (initvar) cls();
  printf("%s", message);
  getkey_wait();
  if (font_width > 1) set_video_mode(3);
  exit(-1);
}
/*--------------------------------------------------------------------------*/
/* memory for GET_IMAGE: a block may be larger than 64K                     */
/*--------------------------------------------------------------------------*/
void GUI_FAR *gui_farmalloc(long size)
{
#ifdef GUI32
  return malloc((size_t) size);
#elif defined(__WATCOMC__)
  return halloc(size, 1);
#elif defined(__TURBOC__)
  return farmalloc(size);
#else
  return halloc(size, 1);
#endif
}

void gui_farfree(void GUI_FAR *block)
{
  if (block == NULL) return;
#ifdef GUI32
  free(block);
#elif defined(__WATCOMC__)
  hfree(block);
#elif defined(__TURBOC__)
  farfree(block);
#else
  hfree(block);
#endif
}
/*--------------------------------------------------------------------------*/
