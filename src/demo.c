/*--------------------------------------------------------------------------*/
/* Copyright Plain Design Inc 1992,1993                                     */
/*--------------------------------------------------------------------------*/
/* Plain Design's Example program for the menu system  ; every available    */
/* function has been demonstrated in this example.  Descriptions are        */
/* available under the help facilities                                      */
/* This is a shareware program; you may use this library to creat any       */
/* program's free of charge.                                                */
/*                                                                          */
/*                                                                          */
/*  Source Code price $29.00                                                */
/*                                                                          */
/*  Send inquiries to       :    Plain Design Inc.                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#include "gui.h"               /* required for use of the GR_MENU library   */
#include "string.h"
#include "stdio.h"
#include "time.h"
#include "dos.h"
#include "stdlib.h"
/*--------------------------------------------------------------------------*/
/*                               PROTO TYPES                                */
/*--------------------------------------------------------------------------*/
int  show_window(int ,int ,int ,int ,char *,int );
void one_button(char *,int ,int ,int ,int);
void about(void);
void ctxt(int ,char *,int );
void alternate_menu_colors(void);
void scroll_down_menu(void);
void one_text_field(void);
void five_text_fields(void);
void one_binary_field(void);
void five_binary_fields(void);
void percent_bar(void);
void scroll_bar_function(void);
/*--------------------------------------------------------------------------*/
/*                               Definitions                                */
/*--------------------------------------------------------------------------*/
#define TEST -10000

/*--------------------------------------------------------------------------*/

/*-------------------------------*/
/* FOR Borland   C COMPILER      */
/*-------------------------------*/
#ifdef BC
extern unsigned _stklen=15000u;
#endif
/*-------------------------------*/


/*--------------------------------------------------------------------------*/
/* The example program as it came with the library, with one addition:      */
/* the video mode can be given on the command line (in hex), for example    */
/*     demo 104      1024 x 768, 16 colors                                  */
/*     demo 105      1024 x 768, 256 colors                                 */
/* without it the mode is 12 (640 x 480, 16 colors) as before.              */
/* And the File Dialog Boxes, which the original library had not: in the    */
/* Functions menu, or F2 (open) and F3 (save).                              */
/*--------------------------------------------------------------------------*/
int main(int argc,char **argv)
{
  int  mode=18;
  int  message;
  int  break_loop=NO; // used to break out of a loop from a switch statement
  int  h=-1;  // button handle
  char button_string[20];
  char file_name[130];
  /*-----------------------------------------------------------*/
  /* This library is designed for VGA 640X480X16 color display */
  /* Mode must be changed to that first                        */
  /*-----------------------------------------------------------*/
  if(argc>1) mode=(int)strtol(argv[1],NULL,16);
  if(set_video_mode(mode)<0)
  {
    printf("the video card will not set mode %Xh",mode); puts("");
    return 1;
  }
  
  /*--------------------------------------------------*/
  /* to use this library you must initialize it first */
  /*--------------------------------------------------*/
  initdisplay();

  /*--------------------------------------------------*/
  /* turn the mouse cursor on ;  this is important    */
  /*--------------------------------------------------*/
  show_mouse_cursor();

  /*---------------------------*/
  /* paint the background gray */
  /*---------------------------*/
  gfillrect(0,0,gr_scr_xsize-1,gr_scr_ysize-1,8);

  /*--------------------------------------------------------------*/
  /* to place a pull down menu, a menu file must be created first */
  /* example 'MENUFILE' contains the format of this file          */
  /* after creating it you must initialize it                     */
  /*--------------------------------------------------------------*/
  ch_menu_x(1);      /* default starts the menu bar at location 0 */
  bar_menu(INIT);

  /*-----------------------------------------------------------*/
  /* show the about dialog box in the beginning of the program */
  /*-----------------------------------------------------------*/
  about();

  /*-------------------------------------------------------------------*/
  /* barmenu function will return the messages from the pull down menu */
  /* and the keyboard; when pull down menu is invoked, barmenu takes   */
  /* over the program.                                                 */
  /*-------------------------------------------------------------------*/
  while(1)      /* program's main message loop */
  {
    message=bar_menu(KEY_RETURN);
    if(h!=-1)
    {
      message=check_button(message);
    }

    switch(message)
    {
      case ESC:          /* exit the program */
       break_loop=YES;
       break;
/*--------------------------------------------------------------------------*/
      case 4005:   /* alternate menu colors    */
       alternate_menu_colors();
       break;
/*--------------------------------------------------------------------------*/
      case 4010:   /* Drawing a line           */
       gr_line(20,20,300,300,4);
       break;
/*--------------------------------------------------------------------------*/
      case 4020:   /* drawing an ellipse       */
       ELLIPSE(320,200,50,70,3);
       break;
/*--------------------------------------------------------------------------*/
      case 4030:   /* text mode rectangle      */
       rect(10,10,20,20,0x4F);
       break;
/*--------------------------------------------------------------------------*/
      case 4040:   /* graphics mode rectangle  */
       grect(300,300,350,350,12);
       break;
/*--------------------------------------------------------------------------*/
      case 4050:   /* text mode frect          */
       frect(20,20,25,25,0x24,0);
       break;
/*--------------------------------------------------------------------------*/
      case 4060:   /* graphics mode frect      */
       gfillrect(220,220,320,330,13);
       break;
/*--------------------------------------------------------------------------*/
      case 4070:   /* text write character     */
       writechar(30,20,'X',0x24);
       break;
/*--------------------------------------------------------------------------*/
      case 4080:   /* text write string        */
       pls(30,21,"Text Write String",0x42);
       break;
/*--------------------------------------------------------------------------*/
      case 4090:   /* graphics write character */
       gwritechar(240,352,'Y',0x52);
       break;
/*--------------------------------------------------------------------------*/
      case 4100:   /* graphics write string    */
       gpls(240,368,"graphics write string",0x72);
       break;
/*--------------------------------------------------------------------------*/
      case 4200:   /* scroll down menu         */
       scroll_down_menu();
       break;
/*--------------------------------------------------------------------------*/
      case 4210:   /* place 'test button       */
       if(h!=-1)break; /* button is already placed */
       strcpy(button_string,"^Test Button");
       h=draw_button(button_string,65,28,strlen(button_string)*8+12,23,TEST,ALTT); /* show button */
       break;
/*--------------------------------------------------------------------------*/
      case 4220:   /* remove 'test' button     */
       if(h==-1) break;   /* cannot remove a button that does not exist */
       disable_button2(h,8); /* disable_button2 will paint background to 8 */
       h=-1;
       break;
/*--------------------------------------------------------------------------*/
      case 4230:   /* place 1 text field       */
       one_text_field();
       break;
/*--------------------------------------------------------------------------*/
      case 4240:   /* place 3 text fields      */
       five_text_fields();
       break;
/*--------------------------------------------------------------------------*/
      case 4250:   /* place 1 binary field     */
       one_binary_field();
       break;
/*--------------------------------------------------------------------------*/
      case 4260:   /* place 3 binary fields    */
       five_binary_fields();
       break;
/*--------------------------------------------------------------------------*/
      case 4280:   /* percent display function */
       percent_bar();
       break;
/*--------------------------------------------------------------------------*/
      case 4290:   /* scroll bar function      */
       scroll_bar_function();
       break;
/*--------------------------------------------------------------------------*/
      case 4500:
       help("Plain Design GUI Example Help","HELPFILE","How to Order Source Code"); 
       break;
/*--------------------------------------------------------------------------*/
      case 5010:   /* help contents is invoked */
       help("Plain Design GUI Example Help","HELPFILE","contents"); 
       break;
/*--------------------------------------------------------------------------*/
      case 5020:   /* help on help  is invoked */
       help("Plain Design GUI Example Help","HELPFILE","Help on Help"); 
       break;
/*--------------------------------------------------------------------------*/
      case 5030:   /* about function */
       about();
       break;
/*--------------------------------------------------------------------------*/
      case 4300:   /* the File Open dialog box (new in the rebuilt library); F2 */
       if(file_open_dialog(NULL,"*.*",file_name)==YES)
       {
         frect(0,txt_scr_ysize-1,txt_scr_xsize-1,txt_scr_ysize-1,0x1F,0);
         pls(0,txt_scr_ysize-1,"Open : ",0x1E);
         pls(7,txt_scr_ysize-1,file_name,0x1F);
       }
       break;
/*--------------------------------------------------------------------------*/
      case 4310:   /* the File Save dialog box; F3 (nothing is written) */
       if(file_save_dialog(NULL,"*.TXT",file_name)==YES)
       {
         frect(0,txt_scr_ysize-1,txt_scr_xsize-1,txt_scr_ysize-1,0x1F,0);
         pls(0,txt_scr_ysize-1,"Save : ",0x1E);
         pls(7,txt_scr_ysize-1,file_name,0x1F);
       }
       break;
/*--------------------------------------------------------------------------*/
      case TEST:
       if(h==-1)break;
       printf("\a");
       pls(0,txt_scr_ysize-1,"Test Button was pressed",0x4F);
       break;
    }
    if(break_loop==YES)break;
  }


  set_video_mode(3);   /* switch the video mode when done */
  return 0;
}
/*--------------------------------------------------------------------------*/
void about(void)
{
  show_window(15,5,50,19,"About Demo",ON);

  ctxt(9,"GUI Library example program ver 1.01",0xF4);
  ctxt(12,"Copyright Plain Design Inc.",0xF0);
  ctxt(13,"1992 - 1994",0xF0);
  ctxt(15,"GUI for DOS ",0xF8);
  ctxt(16,"for C compilers  Borland,Microsoft,Powerc",0xF8);
  ctxt(17,"This program demonstrates the capabilities of",0xF8);
  ctxt(18,"Plain Design's GUI library tool kit",0xF8);


  one_button("  OK  " ,36,21,ENTER,ESC);
  show_window(15,10,50,15,"",OFF);
}
/*--------------------------------------------------------------------------*/
/*                               show_window                                */
/* x,y,xsize,ysize variables are in text mode ranges : 0 <=x<=79  0<=y<=29  */
/* xsize and ysize are also in the same ranges.                             */
/* each text block is 8*16 pixels.                                          */
/* sw should be set to ON to display window and  OFF to remove it           */
/*--------------------------------------------------------------------------*/
int show_window(int x,int y,int xsize,int ysize,char *title,int sw)
{ 
  static int wh;
  int        i;

  switch(sw)
  {    
    case ON:
     wh=messagebox(x,y,xsize,ysize,0xF0,0x08,YES);
     grect(x*8+1,y*16+1,(x+xsize)*8-2,(y+ysize)*16-1,0);

     for(i=1;i<=5;i++)
     {
       grect(x*8+1+i,y*16+1+i,(x+xsize)*8-2-i,(y+ysize)*16-1-i,1);
     }

     gfillrect(x*8+8,y*16+8,(x+xsize)*8-9,y*16+27,1);

     gpls( (x+(xsize/2)-strlen(title)/2)*8,(y+1)*16-5,title,0x1F);

     break;
    case OFF:
     destroy_messagebox(wh);
     if(wh>0)wh--;
     break;
  }

  return wh;
}
/*--------------------------------------------------------------------------*/
/* display button at x,y named 'string' activate button when key is pressed */
/*--------------------------------------------------------------------------*/
void one_button(char *string,int x,int y,int key,int acc)
{

  int h;
  int j,i;

  deactivate_activebuttons(); /* disable all other buttons */

  h=draw_button(string,x,y,strlen(string)*8+14,23,key,acc); /* show button */

  while(1)
  {
    j=getkey();
    i=check_button(j);
    gcurs(MOVE);

    if(i==key)break;
  }

  disable_button(h);    /* remove button */

  reactivate_activebuttons();  /* reactivate all other buttons */

}
/*--------------------------------------------------------------------------*/
/* write text centered on the screen                                        */
/*--------------------------------------------------------------------------*/
void ctxt(int y,char *string,int color)
{
  gpls(320-strlen(string)*4,y*16,string,color);
}
/*--------------------------------------------------------------------------*/
/* change the pull down menu colors                                         */
/*--------------------------------------------------------------------------*/
void alternate_menu_colors(void)
{
  bg_menu_color(BK_CYAN);              /* menu background to CYAN          */
  fg_menu_color(FG_BLACK);             /* menu  forground to Black         */
  bg_menu_hotkey_color(BK_CYAN);       /* Hotkey Back to CYAN              */
  fg_menu_hotkey_color(FG_WHITE);      /* Hotkey Forground to Whit         */
  bg_menu_sel_color(BK_BLUE);          /* Selected menu back to BLUE       */
  fg_menu_sel_color(FG_BRIGHT_WHITE);  /* Selected menu forground to white */
  bar_menu(DISPLAY);
}
/*--------------------------------------------------------------------------*/
/* demonstrate a scroll down menu                                           */
/*--------------------------------------------------------------------------*/
void scroll_down_menu(void)
{
  char menu[20][40] ;        /* menu array                */
  char buffer[30];
  int  menu_ptr=4;


  strcpy(menu[0]," Menu Entry Zero ");
  strcpy(menu[1]," Menu Entry One ");
  strcpy(menu[2]," Menu Entry Two   ");
  strcpy(menu[3]," Menu Entry Three");
  strcpy(menu[4]," Exit ");
  
  show_window(15,10,50,15,"Scroll Down Menu Demo",ON);

  while( (menu_ptr=menus(menu,5,20,13,0xF4,0x4F,menu_ptr,-1)) !=4)
  {
    if(menu_ptr < 0)continue;

    sprintf(buffer,"Menu item %d was selected   ",menu_ptr);
    pls(16,22,buffer,0x2F);
  }
  show_window(15,10,50,15,"",OFF);
}
/*--------------------------------------------------------------------------*/
void one_text_field(void)
{

  int i;
  char j;
  int h1,h2;
  char buffer[50];
  struct xfield pf;

  show_window(20,7,40,9,"One Text Field",ON);

  pf.type=F_TEXT;              /* field types can be F_TEXT,F_DATE,F_TEL */
  pf.xstart=0;                 /* first character for the cursor to show */
  pf.f[0]=0;                   /* initial field content                  */
  strcpy(pf.title,"Field");    /* field title                            */
  pf.fcolor=0x70;              /* field color                            */
  pf.tcolor=0xF4;              /* field title color                      */
  pf.x=28;                     /* x location of field on the text screen */
  pf.y=11;                     /* y location of field on the text screen */
  pf.length=24;                /* field length in characters             */

  /*----------------------------------------------------------------------*/
  /* alt control allows for alternate controls ; this way TAB will select */
  /* buttons or fields besides the mouse                                  */
  /*----------------------------------------------------------------------*/
  alt_control(INIT,0);  // alt control must be initialized, before any field work

  i=text_field(&pf,INIT);

  deactivate_activebuttons();  

  h1=draw_button("^Continue",25,13,78,23,ALTC,ALTC);
  h2=draw_button("Cancel",45,13,78,23,ESC,ESC  );

  check_button(TAB);   /* select the first button */

  while(1)
  {
    j=text_field(&pf,-2);
    i=check_button(j);

    if ( i==ESC || i==ALTC)break;
  }

  disable_button(h1);
  disable_button(h2);

  reactivate_activebuttons();

  show_window(20,7,40,9,"One Text Field",OFF);
  if(i==ALTC)
  {
    sprintf(buffer,"Field Content = %s",pf.f,0x4F);
    pls(0,29,buffer,0x24);
  }

}
/*--------------------------------------------------------------------------*/
void five_text_fields(void)
{
#ifdef MSC
 struct dosdate_t date;
#endif

  int    fret,fptr;

  struct  xfield pf[5];  
  int    i;

 struct tm *time_now;
 time_t secs_now;
 int    break_val=NO;
 int    h1,h2;

 show_window(20,6,40,13,"Five Text Field",ON);

 for(i=0;i<5;i++)
 { pf[i].xstart=0;
   strncpy(pf[i].f,"",sizeof(pf[i].f) );
   pf[i].title[0]=0;
   pf[i].fcolor=0x70;
 }

 tzset();
 time(&secs_now);
 time_now=localtime(&secs_now);

 pf[0].type=F_DATE;
 pf[0].x=27; pf[0].y=9; pf[0].length=8; pls(27,8,(unsigned char *)"Date1 :",0xF4);

#ifdef MSC
 _dos_getdate(&date);
 sprintf(pf[0].f,"%02d-%02d-%02d",date.month,date.day,date.year-1900);
#else
 strftime(pf[0].f,9,"%m-%d-%y",time_now);
#endif

 pf[1].type=F_DATE;
 pf[1].x=42; pf[1].y=9; pf[1].length=8; pls(42,8,(unsigned char *)"Date2 :",0xF4);

#ifdef MSC
 _dos_getdate(&date);
 sprintf(pf[1].f,"%02d-%02d-%02d",date.month,date.day,date.year-1900);
#else
 strftime(pf[1].f,9,"%m-%d-%y",time_now);
#endif

 pf[2].type=F_TEXT;
 pf[2].x=27; pf[2].y=11; pf[2].length=12; pls(27,10,(unsigned char *)"field 3 :",0xF4);

 pf[3].type=F_TEXT;
 pf[3].x=42; pf[3].y=11; pf[3].length=11; pls(42,10,(unsigned char *)"field 4 :",0xF4);

 pf[4].type=F_TEXT;
 pf[4].x=27; pf[4].y=13; pf[4].length=26; pls(27,12,(unsigned char *)"field 5 :",0xF4);

 alt_control(INIT,0);  // alt control must be initialized, before any field work

 for(i=0;i<5;i++)text_field(&pf[i],INIT);

 fptr=0;

 deactivate_activebuttons();


 h1=draw_button("^Continue",26,16,78,23,ALTC,ALTC);
 h2=draw_button("Cancel",44,16,78,23,ESC,ESC  );

 while(1)
 { 
   gcurs(MOVE);

   if(fptr!=-1)
   {
     i=text_field(&pf[fptr],-2);
   }
   else
   {
     i=getkey();
   }

   fret=check_button(i);

   if(fptr==-1 && fret !=EOF)
   {
     fptr=0;
   }


   if(mouse_left_pressed()==YES)
   {
     get_mouse_location();
     for(i=0;i<5;i++)
     {
       if(mouse_x>=pf[i].x && mouse_x<pf[i].x+pf[i].length && mouse_y==pf[i].y)
       { 
         fptr=i;
         break;
       }
     }

   }


   switch(fret)
   {
     case TAB   :
      fptr++;
      if(fptr>=5)
      {
        fptr=-1;
        check_button(CONTROL);
        check_button(TAB);
      }
      break;
     case ENTER :
     case RIGHTKEY :
     case DOWNKEY  :
      if(fptr==4 && fret==ENTER)
      { break_val=YES;
        break;
      }
      fptr++;
      if(fptr>=5)fptr=0;
      break;
     case SHIFTTAB:
     case UPKEY   :
      fptr--;
      if(fptr<0)fptr=4;
      break;
     case ESC :
      disable_button(h1);
      disable_button(h2);
      reactivate_activebuttons();
      show_window(20,6,40,13,"Three Text Field",OFF);
      return;
     case ALTC :
      break_val=YES;
      break;

   }
   if(break_val==YES)break;
 }

 disable_button(h1);
 disable_button(h2);

 reactivate_activebuttons();
 
 show_window(20,6,40,13,"Three Text Field",OFF);

 pls(0,20,"Field Contents :",0x4F);
 pls(0,22,pf[0].f,0x42);
 pls(0,23,pf[1].f,0x42);
 pls(0,24,pf[2].f,0x42);
 pls(0,25,pf[3].f,0x42);
 pls(0,26,pf[4].f,0x42);

}
/*--------------------------------------------------------------------------*/
/* place one binary field; use TAB to select fields with keyboard           */
/*--------------------------------------------------------------------------*/
void one_binary_field(void)
{

  struct bfield bfield;
  int           i;
  int           fptr=0;
  int           fret;
  int           h1,h2;
  int           break_loop=NO;



  bfield.fcolor=0xF7;
  bfield.tcolor=0xF0;
  bfield.x=25;
  bfield.y=11;
  bfield.content=NO;

  strcpy(bfield.title,"Binary Field");


  show_window(20,7,40,10,"One Binary Field",ON);

  deactivate_activebuttons();

  h1=draw_button("^Continue",1206,14,84,23,ALTC,ALTC);
  h2=draw_button("Cancel"  ,1350,14,84,23,ESC,ESC  );

  binary_field(&bfield,INIT,-1);

  while(1)
  {
    gcurs(MOVE);

    if(fptr!=-1) /* when fptr==-1 buttons have been selected */
                 /* otherwise, binary field is selected      */
    {
      i=binary_field(&bfield,-2,-1);
    }
    else
    {
      i=getkey();
    }

    fret=check_button(i);
    
    if(fptr==-1 && fret!=EOF)
    {
      fptr=0;
    }

    switch(fret)
    {
      case ALTC:
       break_loop=YES;
       break;
      case ESC:
       break_loop=YES;
       break;

    }

    if(break_loop==YES)break;
  }

  disable_button2(h1,15);
  disable_button2(h2,15);
  reactivate_activebuttons();

  show_window(20,7,40,10,"One Binary Field",OFF);

  if(fret==ALTC)
  {
    if(bfield.content==YES)pls(0,20,"One Binary Field is Set     ",0xF4);
    else                   pls(0,20,"One Binary Field is not Set ",0xF4);
  }

}
/*--------------------------------------------------------------------------*/
void five_binary_fields(void)
{
  int           i;
  int           break_loop=NO;
  int           fptr=0;
  int           fret;
  int           h1,h2;
  char          buffer[50];
  struct bfield bfield[5];



  /*--------------------------*/
  /* show a message box first */
  /*--------------------------*/

  show_window(20,7,40,10,"Five Binary Fields",ON);

  bfield[0].x=22;
  bfield[0].y=10;
  strcpy(bfield[0].title," field zero");

  bfield[1].x=22;
  bfield[1].y=11;
  strcpy(bfield[1].title," field one");

  bfield[2].x=22;
  bfield[2].y=12;
  strcpy(bfield[2].title," field two");

  bfield[3].x=22;
  bfield[3].y=13;
  strcpy(bfield[3].title," field three");

  bfield[4].x=22;
  bfield[4].y=14;
  strcpy(bfield[4].title," field four");


  for(i=0;i<=4;i++)
  {
    bfield[i].fcolor=0xF7;
    bfield[i].tcolor=0xF0;
    if(fptr!=i)bfield[i].content=NO;
    else       bfield[i].content=YES;
    binary_field(&bfield[i],INIT,-1);
  }


  deactivate_activebuttons();

  h1=draw_button("^Continue",  47,10,84,21,ALTC,ALTC);
  h2=draw_button("Cancel",     47,12,84,21,ESC,ESC  );

  while(1)
  {
    gcurs(MOVE);

    if(fptr!=-1)
    {
      i=binary_field(&bfield[fptr],-2,-1);
    }
    else
    {
      i=getkey();
    }

    fret=check_button(i);
    
    if(fptr==-1 && fret!=EOF)
    {
      fptr=0;
    }

    
    /*----------------------------------------*/
    /* if this field is set, reset the reset  */
    /*----------------------------------------*/

    for(i=0;i<=4;i++)
    {
      if(bfield[fptr].content==YES)
      {
        for(i=0;i<=4;i++)
        { 
          if(bfield[i].content==YES)
          { if(i==fptr)continue;
            bfield[i].content=NO;
            binary_field(&bfield[i],INIT,-1);
          }
        }
      }
      else
      {
        break;
      }
    }
 
    mouse_state();
    if(ms==PRESSED)
    {
      get_mouse_location();
/*-------------------------------------------------------------------------*/
/* the following loop will reset any field that is set after another field */
/* is set.  This is necessary when only one field can be set at a time.    */
/* if multiple fields can be set, remove the following loop                */
/*-------------------------------------------------------------------------*/
      for(i=0;i<=4;i++)
      {
        if(bfield[i].y==mouse_y && 
                        mouse_x>=bfield[i].x && mouse_x<=bfield[i].x+2)
        {
          if(i!=fptr)
          {
            fptr=i;
            binary_field(&bfield[i],INIT,-1);
          }
          break;
        }
      }  
    }     


 
    switch(fret)
    {
      case ALTC:  // save the hcopy type

       break_loop=YES;
       break;
      case ESC:
       break_loop=YES;
       break;

      case TAB:
       fptr++;
       if(fptr>4)
       {  
         fptr=-1;
         check_button(CONTROL);
         check_button(TAB);
       }
       break;

      case SHIFTTAB:
       fptr--;
       if(fptr<0)fptr=4;
       break;

    }

    if(break_loop==YES)break;
  }


  disable_button(h1);
  disable_button(h2);

  show_window(20,7,40,10,"",OFF);

  reactivate_activebuttons();

  if(fret==ALTC)
  {
    strcpy(buffer,"No Field was selected    ");

    for(i=0;i<5;i++)
    {
      if(bfield[i].content==YES)
      {
        sprintf(buffer,"Field %d was selected",i);
      }
    }
    pls(0,20,buffer,0xF2);
  }

}
/*--------------------------------------------------------------------------*/
/* this function displays percentage of two numbers numerator, and          */
/* denominator.  Whitin any loop as a number changes, the bar will change   */
/* also, as a percentage of the two numbers.                                */
/*--------------------------------------------------------------------------*/
void percent_bar(void)
{
  int numerator=0;
  int denominator=20000;

  show_window(20,7,40,10,"Percent Display Bar",ON);

  /*----------------------------------------------------------------*/
  /* it is important that the first time show_percent_bar is called */
  /* the numerator to be 0                                          */
  /*----------------------------------------------------------------*/
  while(1)
  {
    if(show_percent_bar(numerator,denominator,190,185,252,30,0,1,15)==-1) 
    {
      /*-------------------------*/
      /* cancel has been pressed */
      /* or ESC key pressed.     */
      /*-------------------------*/
      break;
    }
    if(numerator==denominator)break;
    numerator++;
  }

  show_percent_bar(-1,0,190,185,252,30,0,1,15);

  show_window(20,7,40,10,"",OFF);
}
/*--------------------------------------------------------------------------*/
void scroll_bar_function(void)
{
  int maxnum=500;
  int num=0;
  int old_num=num;
  int x1=420;
  int y1=136;
  int x2=439;
  int y2=337;
  int border_color=0;
  int bar_color=7;
  int ch;
  int h1;
  int h2;
  char buffer[40];


  show_window(15,5,50,19,"Scroll Bar Demo",ON);
  scr_bar(INIT,0,maxnum,&num,x1,y1,x2,y2,border_color,bar_color,0);

  sprintf(buffer,"num=%d out of %d  ",num,maxnum);
  gpls(200,200,buffer,0xF2);

  h1=draw_button(" Exit ",36,21,60,23,ESC,ENTER);
  h2=draw_button("^Set Bar to 333",19,8,120,23,ALTS,'s');

  while(1)
  {
    ch=getkey();
    ch=check_button(ch);

    ch=scr_bar(ACTION,ch,maxnum,&num,x1,y1,x2,y2,border_color,bar_color,0);

    if(num!=old_num)
    {
      sprintf(buffer,"num=%d out of %d  ",num,maxnum);
      gpls(200,200,buffer,0xF2);
      old_num=num;
    }

    if(ch==ESC)break;
    if(ch==ALTS)
    {
      num=333;
    }
  }

  disable_button(h1);
  disable_button(h2);

  show_window(15,5,50,19,"",OFF);
}
/*--------------------------------------------------------------------------*/
