Graphics User Interface Library by Plain Design Inc. ver 1.01 - rebuilt source
==============================================================================

The library came as binaries only (BC_GUI.LIB, MS_GUI.LIB, PC_GUI.LIB).  This
folder is its source rebuilt in C from the Borland library, checked against
the library's help file and against the original DEMO.EXE.  Nothing in the
folder above has been changed.

What is new
  - Any screen size.  The original was made for 640 x 480 in 16 colors; this
    one takes the size from the mode that is set:

        mode   set_video_mode()        screen        text screen
        12h    MODE_640x480x16         640 x 480     80 x 30    (as before)
        102h   MODE_800x600x16         800 x 600     100 x 37
        104h   MODE_1024x768x16        1024 x 768    128 x 48
        106h   MODE_1280x1024x16       1280 x 1024   160 x 64
        101h   MODE_640x480x256        640 x 480     80 x 30
        103h   MODE_800x600x256        800 x 600     100 x 37
        105h   MODE_1024x768x256       1024 x 768    128 x 48
        107h   MODE_1280x1024x256      1280 x 1024   160 x 64

    The modes from 101h up are VESA modes: the video card needs a VESA BIOS.
    In the 256-color modes the first 16 colors are the usual ones, so colors
    mean what they always did.  set_video_mode() returns -1 if the card will
    not set a VESA mode.  The font is 8 x 16 in all of them.
    txt_scr_xsize, txt_scr_ysize, gr_scr_xsize and gr_scr_ysize say how large
    the screen is, as before.

  - 16-bit and 32-bit programs.  The same source builds a large model
    library for real mode and a 32-bit one for a DOS extender (DOS/4GW).

  - File Dialog Boxes, which the library never had:

        int file_open_dialog(char *title, char *mask, char *filename);
        int file_save_dialog(char *title, char *mask, char *filename);
        int file_dialog(char *title, char *mask, char *filename);

    The box lists the directories, the files of the mask and the drives,
    with a scroll bar; a name or a mask can be typed; Enter, a double click
    or the button opens a directory or takes the file.  It returns YES and
    the file's whole name (room for 128 characters), or NO.
      open : the file must be there; a name that is not is refused.
      save : if there is such a file, the user is asked whether to replace
             it; a name typed without an extension gets the mask's.
      the plain one : any name is taken and nothing is asked.
    With NULL for the title it is "Open File" or "Save File".  See the top
    of filedlg.c, or "File Dialog Box" in the help of the example.

  - GUI.H is the original interface.  DEMO.C from the folder above compiles
    against this library unchanged.

Building (Open Watcom 2, on Windows)
    build           16-bit:  build16\gui.lib and build16\demo.exe
    build 32        32-bit:  build32\gui.lib and build32\demo.exe
  The batch file looks for Open Watcom in C:\WATCOM (or where WATCOM points).
  The 32-bit demo needs DOS4GW.EXE beside it; the batch file copies it.

  A program of your own:
    16-bit   wcl    -ml -bt=dos -l=dos    prog.c build16\gui.lib
    32-bit   wcl386     -bt=dos -l=dos4g  prog.c build32\gui.lib
  Give a 16-bit program a good stack (the demo has 20000 bytes: -k20000).

The example
    demo            640 x 480, 16 colors, as the original
    demo 104        1024 x 768, 16 colors   (any mode number of the table, in hex)
  It needs MENUFILE and HELPFILE in the current folder.  demo.c here is
  DEMO.C with the mode taken from the command line, the background painted
  to the edges of the screen, and the File Dialog Boxes (in the Functions
  menu, or F2 to open and F3 to save).  MENUFILE and HELPFILE in this
  folder are the originals with those menu entries and a help topic
  "File Dialog Box" added; the
  build copies them beside the example.

The files
    gui.h        the interface (the original, plus the mode numbers)
    gport.h, .c  what differs between compilers and between 16 and 32 bits:
                 interrupts, low memory, clock, keyboard, memory
    gfx.c        the drawing layer.  It replaces the eleven assembler modules
                 of the original (SETPIX, RSETPIX, GETPIX, LINE, ELLIPSE,
                 RELLIPSE, PCELLIP, RCELLIP, DISPCHAR, GET_BB, SET_BB): pixels,
                 lines, ellipses, characters, saving and restoring blocks, for
                 16-color (planar) and 256-color modes, with VESA bank
                 switching above 64K
    dispfunc.c   the text screen kept on top of the graphics, the mouse pointer
    base.c       text rectangles, keyboard, mouse, text cursor, video modes,
                 graphics lines and rectangles (the general part of the
                 original's MENU module, and its PIX module)
    window.c     messagebox(): windows that save what they cover; dragging
    button.c     buttons, TAB among buttons (alt_control)
    menu.c       bar_menu() and menus()
    fields.c     text_field() and binary_field()
    scr_bar.c    scr_bar()
    perc_bar.c   show_percent_bar(), gpls2(), gwritechar2(), rgpls(),
                 rgwritechar()
    help.c       help(), delay_func()
    filedlg.c    file_open_dialog(), file_save_dialog(), file_dialog() - new
    MENUFILE, HELPFILE   the example's menu and help, with the File Dialog Box
    demo.c       the example
    build.bat    builds it all

How it was checked
  The original DEMO.EXE and the demo built from this source were run side by
  side in DOSBox-X with the same keys, at 640 x 480, and their screens
  compared pixel by pixel: the About box, the pull down menu and its
  submenus, the scroll down menu, the drawing functions that were tried
  (line, ellipse, rectangles, text), the text fields, the binary fields, the buttons, the scroll bar and the help
  window are identical.  (The blinking text cursor and the mouse pointer are
  where the two happen to be at the moment of the picture.)
  The percent bar looks the same and runs faster.
  Larger screens: the demo was run at 1024 x 768 in 16 colors (16-bit
  build), and at 800 x 600 and 1024 x 768 in 256 colors (32-bit build); a
  drawing test was run at 800 x 600, 1024 x 768 and 1280 x 1024.  640 x 480
  in 256 colors and 1280 x 1024 in 256 colors were not tried.
  The File Dialog Boxes were tried with the keyboard at 640 x 480 (16-bit
  build): listing, going through the list, taking a file, typing a name,
  the refusal of a file that is not there (open), the question before a
  file is replaced and the added extension (save), the help topic.
  Its mouse side (clicks, double clicks, dragging the scroll bar), other
  drives and deep directories were not tried.
  Dragging a window, and everything else done with the mouse, was rebuilt
  from the original's code like the rest but could not be tried from my
  side: please try it.

Where it is not the original, on purpose
  - Sizes that were written into the code as 80, 30, 639, 479 come from the
    mode.  The help window, made for 640 x 480, is shown in the middle of a
    larger screen.
  - GET_IMAGE keeps a block in a form of its own; get_block_size() says how
    large it is.  Use gui_farmalloc() (or farmalloc, which gui.h maps to it)
    for the memory: a block can be larger than 64K.
  - Some slips of the original are put right where they could only do harm:
    the text cursor is now always taken away by txt_curs(0); fg_menu_shadow()
    changes the foreground, not the background; an accelerator key pressed
    while a menu is open chooses its own entry; the time field takes 0
    minutes and the date field the year 00; a window handle is used where
    the original counted on windows being opened in order; mouse_state()
    measures the time of a double click properly.
  - The percent bar and gpls2()/gwritechar2()/rgpls() draw with the font of
    the mode; the original carried a copy of the 8 x 16 font for them.
  - dl_rect() is in GUI.H but was never in the library: it is here (a
    rectangle of double lines).  vtext() and draw_cursor(), also declared
    there, were not in the library either and are still not.
  - The text-mode side of the library (video mode 3) is rebuilt too but was
    not tried.

Not done
  - DJGPP: gport.h and gport.c have its branch written, never compiled.
  - Borland and Microsoft C: the 16-bit branch should be close; never
    compiled either.
  - A menu compiled into the program (CT_VAR.EXE and set_menu_descriptor())
    is rebuilt but was not tried.
