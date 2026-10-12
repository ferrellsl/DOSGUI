/*--------------------------------------------------------------------------*/
/* Graphics User Interface Library by Plain Design Inc.  ver 1.01           */
/*                                                                          */
/* Rebuilt as C source in 2026 from the library's Borland C binary, its     */
/* help file and its example program.  The interface is the original one;   */
/* what is new:                                                             */
/*   - any screen size: 640x480 as before, and the VESA modes listed        */
/*     below (set_video_mode() takes their numbers);                        */
/*   - 16-bit (large model) and 32-bit (DOS extender) programs.             */
/*--------------------------------------------------------------------------*/
#ifndef GUI_H
#define GUI_H

/*--------------------------------------------------------------------------*/
/* 16-bit or 32-bit program ?                                               */
/*--------------------------------------------------------------------------*/
#if defined(__386__) || defined(__DJGPP__) || defined(__FLAT__) || defined(_M_I386)
#define GUI32
#define GUI_FAR
#else
#define GUI_FAR far
#endif

/*--------------------------------------------------------------------------*/
/*                              VIDEO MODES                                 */
/* for set_video_mode(); initdisplay() then takes the screen as it finds it */
/*--------------------------------------------------------------------------*/
#define MODE_TEXT               0x03    /* 80 x 25 text                      */
#define MODE_640x350x16         0x10
#define MODE_640x480x16         0x12    /* the mode the library was made for */
#define MODE_800x600x16         0x102   /* VESA, 16 colors                   */
#define MODE_1024x768x16        0x104
#define MODE_1280x1024x16       0x106
#define MODE_640x480x256        0x101   /* VESA, 256 colors (the first 16    */
#define MODE_800x600x256        0x103   /* are the usual ones)               */
#define MODE_1024x768x256       0x105
#define MODE_1280x1024x256      0x107

/*--------------------------------------------------------------------------*/
/*                                 PROTO TYPES                              */
/*--------------------------------------------------------------------------*/
int           alt_control(int,int);
void          pls(int,int,unsigned char[],int);
void          plsh(int,int,unsigned char[],int,int);
void          gpls(int,int,unsigned char[],int);
void          gpls2(int,int,unsigned char *,int);
void          rgpls(int,int,unsigned char *,int);
void          cgpls(int,int,unsigned char[],int);
void          help(char *,char *,char *);
void          frect(int,int,int,int,int,int);
void          dl_rect(int,int,int,int,int);
void          rect(int,int,int,int,int);
int           menus(char [20][40],int,int,int,int,int,int,int);
void          writechar(int,int,unsigned char,int);
void          gwritechar2(int,int,int ,int);
void          rgwritechar(int,int,int ,int);
void          gwritechar(int,int,unsigned char,int);
void          delay_func(int);
void          writecharh(int,int,unsigned char,int,int);
void          initdisplay(void);
int           getkey(void);
int           messagebox(int,int,int,int,int,int,int);
void          destroy_messagebox(int);
void          chcolor(int,int,int,int,int);
void          mchcolor(int,int,int,int,int);
void          mchcolorh(int,int,int,int,int,int);
void          show_mouse_cursor(void);
void          hide_mouse_cursor(void);
void          get_mouse_location(void);
void          get_ml(int *,int *);
void          set_ml(int ,int );
int           mouse_left_pressed(void);
int           mouse_state(void);
void          limit_mouse(int,int,int,int);
int           bar_menu(int);
int           alt_pressed(void);
void          txt_curs(int);
void          cls(void);
void          bg_menu_color(int color);
void          fg_menu_color(int color);
void          bg_menu_hotkey_color(int);
void          fg_menu_hotkey_color(int);
void          bg_menu_sel_color(int);
void          fg_menu_sel_color(int);
void          bg_menu_shadow(int);
void          fg_menu_shadow(int);
void          ch_menu_x(int);
void          ch_menu_y(int);
void          ch_menu_xsize(int);
void          ch_menu_filename(char *);
void          set_menu_descriptor(int, char **,int);
void          frect_char(unsigned char) ;
unsigned char readchar(int,int);
int           readcolor(int,int);
int           text_field(struct xfield *,int);
int           binary_field(struct bfield *,int,int);
void          readstring(int,int,int,char *);
int           set_video_mode(int);
int           get_video_mode(void);
void          gcurs(int);

void          DISPLAYCHAR(int,int,int,int,int);

void          SETPIX(int ,int ,char );
void          RSETPIX(int ,int ,char );
int           GETPIX(int ,int );
void          GLINE(int ,int ,int ,int ,int,char );
void          gr_line(int ,int ,int ,int ,int );
void          cgr_line(int ,int ,int ,int ,int );
void          grect(int ,int ,int ,int ,int );
void          cgrect(int ,int ,int ,int ,int );
void          gfillrect(int ,int ,int ,int ,int );
void          cgfillrect(int ,int ,int ,int ,int );
void          ELLIPSE(int,int,int,int,int);
void          RELLIPSE(int,int,int,int,int);
int           RCELLIP(int,int,int,int,int,int,int,int,int);
int           PCELLIP(int,int,int,int,int,int,int,int,int);
void          RESTORE_IMAGE(char GUI_FAR *,int,int);
int           GET_IMAGE(int,int,int,int,char GUI_FAR *);
long          get_block_size(int ,int ,int ,int );
void          draw_cursor(int,int);
void          xor_line(int,int,int,int,int);
void          xor_rect(int ,int ,int ,int ,int ,int );
void          xor_frect(int ,int ,int ,int ,int );
int           show_percent_bar( int,int, int, int, int, int, int, int ,int);
int           vtext( int,int,int,int,char *,int,int,char);
int           scr_bar(int,int,int,int *,int,int,int,int,int,int,int);
int           draw_button(char *,int ,int ,int ,int ,int ,int );
void          disable_button(int );
void          disable_button2(int ,int);
int           check_button(int);
int           file_dialog(char *title,char *mask,char *filename);
void          deactivate_activebuttons(void);
void          reactivate_activebuttons(void); // reactivate all previously active buttons

/*--------------------------------------------------------------------------*/
/*                               STRUCTURES                                 */
/*--------------------------------------------------------------------------*/
struct xfield
{
  int  type;               /* type of the field                 */
  int  x;                  /* field's X coordinate              */
  int  y;                  /* field's Y coordinate              */
  int  length;             /* field length                      */
  char title[20];          /* title of the field                */
  char f[80];              /* field content                     */
  int  fcolor;             /* field color                       */
  int  tcolor;             /* title color                       */
  int  xstart;             /* default x location for the cursor */

};
/*--------------------------------------------------------------------------*/
struct bfield
{
  int  x;                  /* x coordinate of the field         */
  int  y;                  /* y coordinate of the field         */
  char title[54];          /* title of the field                */
  int  fcolor;             /* color of the field it selft       */
  int  tcolor;             /* color of the title of the field   */
  int  content;            /* set to yes or no                  */

};
/*--------------------------------------------------------------------------*/
/*                               DEFINITIONS                                */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* Changing these values is only effective after a recompile of the library */
/*--------------------------------------------------------------------------*/
#define MAX_TITLE   10
#define MAX_ENTRIES 25
#define MAX_ACC     100
#define MAX_WINDOWS 20
#define MAX_BUTTONS 40
/*--------------------------------------------------------------------------*/

#define F_TEXT                  0
#define F_NUMBER                1
#define F_DATE                  2
#define F_TIME                  3
#define F_TEL                   4
#define TEXT_FILE               0
#define VARIABLE                1
#define OFF                     0
#define ON                      1
#define YES                     1
#define NO                      0
#define BS                      8
#define FORMFEED                12
#define CR                      13
#define ENTER                   13
#define ESC                     27
#define HOMEKEY                 327
#define ENDKEY                  335
#define UPKEY                   328
#define DOWNKEY                 336
#define PGUPKEY                 329
#define PGDNKEY                 337
#define LEFTKEY                 331
#define INSKEY                  338
#define RIGHTKEY                333
#define DELKEY                  339
#define CTRLLEFTKEY             371
#define CTRLRIGHTKEY            372
#define TAB                     9
#define SHIFTTAB                271
#define SPACE                   32
#define BLOCK1                  176
#define BLOCK2                  177
#define BLOCK3                  178
#define PRESSED                 0
#define RELEASED                1
#define PRESSEDRELEASED         2
#define DOUBLECLICKED           3
#define DOUBLECLICKDELAY        300   // delay between clickes to qualify as
                                      // double click
#define CONTROL                 -5
#define REPORT                  30
#define SET_NUM_FIELDS          22
#define ACTION                  21
#define NEW_FIELD               20
#define MOVE                    9
#define INIT                    10
#define KEY_RETURN              1
#define DISPLAY                 2
#define EDIT                    3

#define F1                      315
#define F2                      316
#define F3                      317
#define F4                      318
#define F5                      319
#define F6                      320
#define F7                      321
#define F8                      322
#define F9                      323
#define F10                     324

/*----------*/
/* Alt Keys */
/*----------*/

#define ALTA                    286
#define ALTB                    304
#define ALTC                    302
#define ALTD                    288
#define ALTE                    274
#define ALTF                    289
#define ALTG                    290
#define ALTH                    291
#define ALTI                    279
#define ALTJ                    292
#define ALTK                    293
#define ALTL                    294
#define ALTM                    306
#define ALTN                    305
#define ALTO                    280
#define ALTP                    281
#define ALTQ                    272
#define ALTR                    275
#define ALTS                    287
#define ALTT                    276
#define ALTU                    278
#define ALTV                    303
#define ALTW                    273
#define ALTX                    301
#define ALTY                    277
#define ALTZ                    300

/*--------------------------------------------*/
/*                  COLORS                    */
/*--------------------------------------------*/
#define BK_BLACK                0x00
#define BK_BLUE                 0x10
#define BK_GREEN                0x20
#define BK_CYAN                 0x30
#define BK_RED                  0x40
#define BK_MAGENTA              0x50
#define BK_BROWN                0x60
#define BK_WHITE                0x70


#define FG_BLACK                0x00
#define FG_BLUE                 0x01
#define FG_GREEN                0x02
#define FG_CYAN                 0x03
#define FG_RED                  0x04
#define FG_MAGENTA              0x05
#define FG_BROWN                0x06
#define FG_WHITE                0x07
#define FG_GRAY                 0x08
#define FG_LIGHT_BLUE           0x09
#define FG_LIGHT_GREEN          0x0A
#define FG_LIGHT_CYAN           0x0B
#define FG_LIGHT_RED            0x0C
#define FG_LIGHT_MAGENTA        0x0D
#define FG_LIGHT_YELLOW         0x0E
#define FG_BRIGHT_WHITE         0x0F
/*--------------------------------------------------------------------------*/
/*                          GLOBAL     VARIABLES                            */
/*--------------------------------------------------------------------------*/
extern  int            mousefound;    /* YES or NO  exist ?      */
extern  int            mouse_x;       /* mouse x,y in text mode coordinates */
extern  int            mouse_y;

extern  int            txt_curs_x;    /* text cursor coordinates */
extern  int            txt_curs_y;
extern  int            ms;            /* mouse state varaible    */
extern  int            dc;            /* mouse double click state */
extern  unsigned char  back_block;   /* character for fillrect in text mode */
extern char            menu_file[40];     /* current menu file name     */
extern int             menu_color;        /* color of the menu          */
extern int             menu_hotkey_color; /* color of the menu hot keys */
extern int             menu_sel_color;    /* color of the selected entry */
extern int             menu_x;            /* menu x,y coordinates in text */
extern int             menu_y;
extern int             menu_x_size;       /* menu xsize in text sizes  */
extern int             font_width;       /* font width & height in graphics */
extern int             font_height;
extern int             gr_scr_xsize;     /* current video mode screen size */
extern int             gr_scr_ysize;
extern int             txt_scr_xsize;    /* current mode text screen size  */
extern int             txt_scr_ysize;
extern int             xwin      [MAX_WINDOWS];  /* open window variables */
extern int             ywin      [MAX_WINDOWS];
extern int             xwinsize  [MAX_WINDOWS];
extern int             ywinsize  [MAX_WINDOWS];
extern int             wincolor  [MAX_WINDOWS];
extern int             winshadow [MAX_WINDOWS];
extern int             winborder [MAX_WINDOWS];
extern char            menu_title[MAX_TITLE][20]; /* menu titles array */
extern char            title_hotkey[MAX_TITLE];   /* menu bar variables */
extern char            title_hotkey_loc[MAX_TITLE];
extern long            title_loc [MAX_TITLE];
extern int             title_ident[MAX_TITLE];
extern int             title_acc[MAX_TITLE];
extern int             title_x[MAX_TITLE];
extern int             title_num;
extern int             title_ptr;
extern unsigned char   *sc;
extern unsigned char   *save_box [MAX_WINDOWS + 5];
extern int             num_of_lines;
extern int             text_show;
extern int             open_win  [MAX_WINDOWS];
extern int             show_win_num;                /* total displayed windows        */
extern int             initvar;
extern char GUI_FAR    *gsave_box[MAX_WINDOWS];
extern int             mouse_display;
extern int             in_menu;
extern int             menu_move_lock;
extern char            xor_var;
extern int             soft_return    ;    // when on field will do soft returns
extern char            soft_word[80]  ;    // store the word for soft return;

/*-------------------------------*/
/* FOR POWERC      COMPILER      */
/*-------------------------------*/
#ifdef PC
#define delay(i) sleep(i/1000)
#define _enable() enable()
#define _disable() disable()
#endif
/*-------------------------------*/

/*-------------------------------*/
/* FOR Microsoft   COMPILER      */
/*-------------------------------*/
#ifdef MSC
#define MK_FP(seg,ofs) ((void far *) (((unsigned long)(seg) << 16) | (unsigned)(ofs)))
#define farmalloc(i) _fmalloc(i)
#define farfree _ffree
#endif
/*-------------------------------*/

/*-------------------------------*/
/* FOR Watcom and DJGPP          */
/* memory for GET_IMAGE          */
/*-------------------------------*/
void GUI_FAR *gui_farmalloc(long size);
void          gui_farfree(void GUI_FAR *block);
#if defined(__WATCOMC__) || defined(__DJGPP__)
#define farmalloc(i) gui_farmalloc(i)
#define farfree(p)   gui_farfree(p)
#endif
/*-------------------------------*/

#endif
