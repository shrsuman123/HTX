
/* @(#)38	1.12  src/htx/usr/lpp/htx/bin/stxclient/T_device.c, eserv_gui, htxubuntu 5/24/04 17:04:25 */

/*****************************************************************************/
/*                                                                           */
/* MODULE NAME    =    T_device.c                                            */
/* COMPONENT NAME =    hxssup (supervisor)                                   */
/* LPP NAME       =    HTX                                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Terminate device                                      */
/*                                                                           */
/* COPYRIGHT =         (C) COPYRIGHT IBM CORPORATION 2000                    */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           */
/*                                                                           */
/* STATUS =            Release 1 Version 0                                   */
/*                                                                           */
/* FUNCTION =          Terminates exerciser on user request.                 */
/*                                                                           */
/* COMPILER OPTIONS =                                                        */
/*                                                                           */
/* CHANGE ACTIVITY =                                                         */
/*    DATE    :LEVEL:PROGRAMMER:DESCRIPTION                                  */
/*    MMMMMMMMNMMMMMNMMMMMMMMMMNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM */
/*    11/08/99:1.0  :R GEBHARDT:INITIAL RELEASE                              */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*            :     :          :                                             */
/*****************************************************************************/

#include "hxssup.h"


#define NUM_ENTRIES info_rcv.cur_info.num_entries
#define MAX_ENTRIES info_rcv.cur_info.max_entries

#define NUM_COLS  (23)
#define NUM_ROWS  (81)

#define TD_IN_ROW  (21)
#define TD_IN_COL  (18)

//extern  char  *regcmp(char *, char *);  /* compile exp */
//extern  char  *regex(char *, char *);

extern tfull_info info_rcv;
extern char ecg_name[20];
char *fno;
int len;
int fno1;
extern int slow_shutd_wait;  /* configure from .htx_profile       */
extern char HTXPATH[];    /* HTX file system path         */

/*****************************************************************************/
/*****  g e t _ d s t  *******************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     get_dst                                               */
/*                                                                           */
/* DESCRIPTIVE NAME =  Get Device Status                                     */
/*                                                                           */
/* FUNCTION =          Determines the status of a device and returns it.     */
/*                                                                           */
/* INPUT =             Hardware Exerciser entry in shared memory structure.  */
/*                     Position of HE entry in shared memory structure.      */
/*                                                                           */
/* OUTPUT =            Device status.                                        */
/*                                                                           */
/* NORMAL RETURN =     Pointer to status string.                             */
/*                                                                           */
/* ERROR RETURNS =     None                                                  */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES =                                                       */
/*                                                                           */
/*    DATA AREAS =     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                     SEMHEKEY semaphores (identified by the semhe_id       */
/*                        variable)                                          */
/*                                                                           */
/*    MACROS =                                                               */
/*                                                                           */
/*****************************************************************************/

char *get_dst(int status)
{
  static char *devstatus[9] = {  /* storage for returned status string */
    /* 0 */ "UK",
    /* 1 */ "CP",
    /* 2 */ "DD",
    /* 3 */ "ER",
    /* 4 */ "HG",
    /* 5 */ "RN",
    /* 6 */ "SG",
    /* 7 */ "ST",
    /* 8 */ "TM"
  };

  return (devstatus[status]);
}        /* get_dst */

/*****************************************************************************/
/*****  T _ d e v i c e  *****************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     T_device                                              */
/*                                                                           */
/* DESCRIPTIVE NAME =  Terminate hardware exerciser                          */
/*                                                                           */
/* FUNCTION =          Kills a hardware exerciser by sending it a SIGTERM    */
/*                     signal.                                               */
/*                                                                           */
/* INPUT =             Operator specifies device to terminate.               */
/*                                                                           */
/* OUTPUT =            kills hardware exerciser programs                     */
/*                                                                           */
/*                     Updated system semaphore structure to reflect the     */
/*                     state of the HE's which are killed.                   */
/*                                                                           */
/* NORMAL RETURN =     0 to indicate successful completion                   */
/*                                                                           */
/* ERROR RETURNS =     -1 to indicate no hardware exerciser programs         */
/*                         currently in memory.                              */
/*                     -2 to indicate an unsuccessful attempt to allocate    */
/*                         memory for the activate/halt device table.        */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = display_scn -  displays screen image                  */
/*                     init_ahd_tbl - initializes and sorts the list of      */
/*                                    devices (HEs)                          */
/*                     send_message - outputs a message to the screen and    */
/*                                    the message log file                   */
/*                     get_dst - returns status of a device in test.         */
/*                     terminate_exerciser - kills an HE and runs cleanup    */
/*                                    scripts by exec_HE_scripts().          */
/*                                                                           */
/*    DATA AREAS =     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                     SEMHEKEY semaphores (identified by the semhe_id       */
/*                        variable)                                          */
/*                     __loc1 - pointer to reg. expression matching string.  */
/*                                                                           */
/*    MACROS =         CLRLIN - Clear line CURSES macro                      */
/*                              (defined in common/hxiconv.h)                */
/*                     PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/

int  T_device(void)
{
  char *cmp_regexp;  /* ptr to compiled reg expression    */
  char input[32];    /* input string                      */
  char ret[3];    /* return array for regex()          */
  char *work;    /* work int */
  char workstr[128];  /* work string                       */
  char result_msg[80];
  int inp;
  char tempstr[80];
  enum t_dev_status dev_status;  /* return from get_dst               */
  extern char *__loc1;  /* beginning of reg exp match        */

  extern int COLS;  /* number of cols in CURSES screen   */
  extern int LINES;  /* number of lines in CURSES screen  */
  extern WINDOW *stdscr;  /* standard screen window            */


  int col = 0;    /* first column displayed on screen  */
  int i = 0;    /* loop counter                      */
  int max_strt_ent = 0;  /* max starting entry                */
  int num_disp = 0;    /* number of THE entries to show     */
  int num_entries = 0;  /* local number of shm HE entries    */
  int row = 0;    /* first row displayed on screen     */
  int strt_ent = 1;  /* first entry on screen             */
  int update_screen = 0;  /* update screen flag                */
  int workint = 0;
  int cur_page=1;
  int prev_page=1;
  int old_page = 1;

  /*
   * display screen outline
   */
  if (send_sockmsg(SCREEN_9_T,1,0,ecg_name,result_msg) !=0 ) {
     PRTMSG(MSGLINE, 0, ("%s",result_msg));
     getch();
     CLRLIN(MSGLINE, 0);
     return 0;
  }
    if (NUM_ENTRIES == 0) {  /* no HE programs?        */
      PRTMSG(MSGLINE, 0, ("There are no Hardware Exerciser programs currently defined."));
      getch();
      fflush(stdin);
      CLRLIN(MSGLINE, 0);
      return (0);
    }

  clear();
  refresh();
  display_scn(stdscr, 0, 0, LINES, COLS, "TD_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');

  /*
   * loop until operator says to quit
   */

  for (;;) {
	clear();
        refresh();
	 display_scn(stdscr, 0, 0, LINES,COLS, "TD_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');

        num_entries = NUM_ENTRIES;

        if (num_entries <= 0)  {
          return (num_entries);
        }

        /* problem in init fcn - bye!      */
        else {  /* init fcn ran ok.                */
          if (num_entries <= 13) {  /* 13 or fewer entries? */
            max_strt_ent = 1;
          }
          
          else {  /* more than 13 entries */
            max_strt_ent = num_entries;
          }  /* endif */
        }  /* endif */

    /*
     * build screen data for the current strt_ent
     */
    num_disp = num_entries - strt_ent + 1;
    if (num_disp > 13)  {
      num_disp = 13;
    }
    old_page = prev_page;
     prev_page = cur_page;
    cur_page = ((strt_ent-1)/13)+1;
    mvwaddstr(stdscr, 22, 74, "     ");
    sprintf(workstr, "T.%d", cur_page);
    mvwaddstr(stdscr, 22, 74, workstr);


      sprintf(workstr, "%s",info_rcv.sys_hdr_info.str_curr_time);
      #ifdef  __HTX_LINUX__
        wcolorout(stdscr, GREEN_BLACK | BOLD| NORMAL);
      #else
        wcolorout(stdscr, NORMAL | F_GREEN | B_BLACK);
      #endif
      mvwaddstr(stdscr,1,61,workstr);


    for (i = 1; i <=13; i++) {
      if (i <=num_disp) {  /* something there? */

        /*
         * screen entry number
         */
#ifdef  __HTX_LINUX__
        wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
        wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
        wmove(stdscr, (i + 6), 3);
        wprintw(stdscr, "%2d", i);

        /*
         * Device Status
         */
        work = info_rcv.scn_num.scn_2_4_info[i].status;
        workint = strlen(work);  /* length of status string */
        strncpy(workstr, "        ", 8 - workint);  /* spaces */
        workstr[8 - workint] = '\0';

        /* set display attributes based on status */
        if (dev_status == DST_CP || dev_status == DST_ST || dev_status == DST_RN)  {
#ifdef  __HTX_LINUX__
          wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
          wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
        }

        if (dev_status == DST_UK || dev_status == DST_SG || dev_status == DST_TM)  {
#ifdef  __HTX_LINUX__
          wcolorout(stdscr, WHITE_BLUE | STANDOUT);
#else
          wcolorout(stdscr, STANDOUT | F_WHITE | B_BLUE);
#endif
        }

        if (dev_status == DST_DD)  {
#ifdef  __HTX_LINUX__
          wcolorout(stdscr, RED_BLACK | STANDOUT);
#else
          wcolorout(stdscr, STANDOUT | F_RED | B_BLACK);
#endif
        }

        if (dev_status == DST_ER || dev_status == DST_HG)  {
#ifdef  __HTX_LINUX__
          wcolorout(stdscr, RED_BLACK | NORMAL);
#else
          wcolorout(stdscr, NORMAL | F_RED | B_BLACK);
#endif
        }

        /* display status */
        mvwaddstr(stdscr, (i + 6), 6, work);
        //mvwaddstr(stdscr, (i + 6), 6, "RUNNING ");

        /* pad field with spaces */

#ifdef  __HTX_LINUX__
        wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
        wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
        mvwaddstr(stdscr, (i + 6), 6 + workint, workstr);

        /*
         * Slot, Port, /dev/id, Adapt Desc, and Device
         * Desc
         */
        wmove(stdscr, i + 6, 15);
#ifdef  __HTX_LINUX__
        wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
        wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif

        sprintf(workstr, " %-11s ", info_rcv.scn_num.scn_2_4_info[i].slot_port);
        workstr[13] = '\0';
        mvwaddstr(stdscr, (i + 6), 15, workstr);
        /*sprintf(workstr, " %.4d ", info_rcv.scn_num.scn_2_4_info[i].slot);
        mvwaddstr(stdscr, (i + 6), 15, workstr);

        sprintf(workstr, " %.4d ", info_rcv.scn_num.scn_2_4_info[i].port);
        mvwaddstr(stdscr, (i + 6), 22, workstr);*/

        /* set color for device id based on status */
        if (dev_status == DST_CP || dev_status == DST_ST || dev_status == DST_RN)  {
#ifdef  __HTX_LINUX__
          wcolorout(stdscr, WHITE_BLUE | STANDOUT);
#else
          wcolorout(stdscr, STANDOUT | F_WHITE | B_BLUE);
#endif
        }

        else  {
#ifdef  __HTX_LINUX__
          wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
          wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
        }

        sprintf(workstr, " %-7s ", info_rcv.scn_num.scn_2_4_info[i].sdev_id);
        mvwaddstr(stdscr, (i + 6), 29, workstr);
#ifdef  __HTX_LINUX__
        wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
        wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
        sprintf(workstr, " %-11s ", info_rcv.scn_num.scn_2_4_info[i].adapt_desc);
        mvwaddstr(stdscr, (i + 6), 39, workstr);
        sprintf(workstr, " %-18s ", info_rcv.scn_num.scn_2_4_info[i].device_desc);
        mvwaddstr(stdscr, (i + 6), 53, workstr);
      }
      
      else {  /* no HE entry for this area of screen */
#ifdef  __HTX_LINUX__
        wcolorout(stdscr, WHITE_BLUE | NORMAL);
#else
        wcolorout(stdscr, NORMAL | F_WHITE | B_BLUE);
#endif
        mvwaddstr(stdscr, (i + 6), 3, "  ");  /* scn entry num */
        mvwaddstr(stdscr, (i + 6), 6, "        ");  /* COE Flag  */
        mvwaddstr(stdscr, (i + 6), 15, "      ");  /* slot fld */
        mvwaddstr(stdscr, (i + 6), 22, "      ");  /* port fld */
        mvwaddstr(stdscr, (i + 6), 29, "         ");  /* sdev_id */
        mvwaddstr(stdscr, (i + 6), 39, "             ");
        /* adapt_desc field */
        
        mvwaddstr(stdscr, (i + 6), 53, "                    ");      /* device_desc field */

      }  /* endif */

    }    /* endfor */

#ifdef  __HTX_LINUX__
    wcolorout(stdscr, NORMAL);
#else
    wcolorout(stdscr, NORMAL);
#endif

    for (update_screen = FALSE; (update_screen == FALSE);) {

      /*
       * Read input from keyboard...
       */
strncpy(input, "", DIM(input));  /* clear input */
get_string(stdscr, TD_IN_ROW, TD_IN_COL, input, (size_t) DIM(input), (char *) NULL, (tbool) TRUE);
      
  if((strcmp(input,"F(7)"))==0)
                {
                  if(strt_ent > 1)
                         {
                           if (strt_ent > 1)
                           strt_ent -= 13;
                           if (strt_ent < 1)
                              {
                                strt_ent = 1;
                              }
                           update_screen = TRUE;
                           send_sockmsg(SCREEN_9_T,'u',strt_ent,ecg_name,result_msg);
                         }
                      else {
                             beep();
                           }
    
                }
  else if ((strcmp(input,"F(8)"))==0)
                {
                    if(strt_ent < max_strt_ent)
                       {
                            if (strt_ent < max_strt_ent)
                            strt_ent += 13;
                            if (strt_ent > max_strt_ent)
                               {
                                  strt_ent -= 13;
                               }
                            update_screen = TRUE;
                        send_sockmsg(SCREEN_9_T,'l',strt_ent,ecg_name,result_msg);
                     }
                     else
                        {
                          beep();
                        }

		}
  else if ((strcmp(input,"F(3)"))==0)
                {
		if (prev_page == cur_page && cur_page != old_page)
		             {
			              prev_page = old_page;
			           }
              else {
                     old_page = prev_page;
                  }
                   strt_ent=(prev_page*13)-12;
                   if(strt_ent<=max_strt_ent)
                   {
                    update_screen= TRUE;
                   send_sockmsg(SCREEN_9_T,'p',strt_ent,ecg_name,result_msg);
                   }
                  else
                        {
                        beep();
                        }
                }
 else if(input[strlen(input)-1]=='f'||input[strlen(input)-1]=='F')
            {
            if (strlen(input) != 1)
               {
                 fno = (char *)strtok(input,"f");
                if(fno!=NULL)
                 {
                 len = strlen(fno);
                 for(i=0 ; i<len ; i++)
                   {
                      if ((fno[i] < 48) || (fno[i] > 57))
                        {
                        PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                         getch();
                         CLRLIN(MSGLINE, 0);
                         break;
                        }
                   }
                 if (i==len)
                   {
                 fno1= (int) atoi(fno);
                 if(strt_ent + (fno1 * 13) < max_strt_ent)
                   {
                     strt_ent += (fno1 * 13);
                     update_screen = TRUE;
                     send_sockmsg(SCREEN_9_T,'f',strt_ent,ecg_name,result_msg);
                   }
                 else
                   {
                     update_screen = TRUE;
                     if(strt_ent < max_strt_ent)
                       {
                            while (strt_ent < max_strt_ent)
                            strt_ent += 13;
                            if (strt_ent > max_strt_ent)
                               {
                                  strt_ent -= 13;
                               }
                            update_screen = TRUE;
                            send_sockmsg(SCREEN_9_T,'l',strt_ent,ecg_name,result_msg);
                       }
                     else
                       {
                            beep();
                       }
                 }
	}
         }
    else {
       PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
        getch();
        CLRLIN(MSGLINE, 0);
    }
}
             else
                  {
                      update_screen = TRUE;
                      if (strt_ent < max_strt_ent)
                       {
                          strt_ent += 13;
                          if (strt_ent > max_strt_ent)
                             {
                               strt_ent -= 13;
                             }
                          update_screen = TRUE;
                          send_sockmsg(SCREEN_9_T,'f',strt_ent,ecg_name,result_msg);
                       }
                      else
                        {
                          beep();
                       }
                  }
        }

        /*else if(input[strlen(input)-1]=='l'||input[strlen(input)-1]=='L')*/
       else if((strcmp(input,"l")==0)||(strcmp(input,"L")==0))
         {
              if (strlen(input) != 1)
                {
                   PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                   getch();
                   CLRLIN(MSGLINE, 0);
                }
              else
                {
                   update_screen = TRUE;
                   if(strt_ent < max_strt_ent)
                     {
                       while (strt_ent < max_strt_ent)
                       strt_ent += 13;

                       if (strt_ent > max_strt_ent)
                         {
                            strt_ent -= 13;
                         }
                       update_screen = TRUE;
                       send_sockmsg(SCREEN_9_T,'l',strt_ent,ecg_name,result_msg);
                     }
                  else {
                         beep();
                       }  /* endif */
                 }
         }

else if(input[strlen(input)-1]=='b'||input[strlen(input)-1]=='B')
           {
             if (strlen(input) != 1)
                {
                   fno = (char *)strtok(input,"b");
                if(fno!=NULL)
                 {
                 len = strlen(fno);
                 for(i=0 ; i<len ; i++)
                   {
                      if ((fno[i] < 48) || (fno[i] > 57))
                        {
                        PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                         getch();
                         CLRLIN(MSGLINE, 0);
                         break;
                        }
                   }
                 if (i==len)
                   {

                   fno1= (int) atoi(fno);
                   if(strt_ent - (fno1 * 13) > 1)
                     {
                       strt_ent -= (fno1 * 13);
                       update_screen = TRUE;
                       send_sockmsg(SCREEN_9_T,'b',strt_ent,ecg_name,result_msg);
                     }
                   else
                     {
                       update_screen = TRUE;
                       if(strt_ent > 1)
                         {
                           while (strt_ent > 1)
                           strt_ent -= 13;
                           if (strt_ent < 1)
                              {
                                strt_ent = 1;
                              }
                           update_screen = TRUE;
                           send_sockmsg(SCREEN_9_T,'u',strt_ent,ecg_name,result_msg);
                         }
                      else {
                             beep();
                           }
 }
 }}
           else
              {
                PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                getch();
                 CLRLIN(MSGLINE, 0);
             }
       }
         else
                {
                if(strt_ent > 1)
                  {
                     strt_ent -= 13;
                     if (strt_ent < 1)
                        {
                          strt_ent = 1;
                        }
                     update_screen = TRUE;
                     send_sockmsg(SCREEN_9_T,'b',strt_ent,ecg_name,result_msg);
                  }
                else
                  {
                     beep();
                     }
                }
        }


            /* else if(input[strlen(input)-1]=='h'||input[strlen(input)-1]=='H')*/
      else if((strcmp(input,"h")==0)||(strcmp(input,"H")==0))
            {
              if (strlen(input) != 1)
                 {
                    PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                    getch();
                    CLRLIN(MSGLINE, 0);
                 }
              else
                {
                  help(16, 50, 2, 2, "sThelp_scn", 3);
                  clear();
                  refresh();
          //        display_scn(stdscr, 0, 0, LINES,COLS, "TD_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
                   update_screen = TRUE;
                }
           }
       else if((strcmp(input,"u")==0)||(strcmp(input,"U")==0))
            {
              if (strlen(input) != 1)
                 {
                    PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                    getch();
                    CLRLIN(MSGLINE, 0);
                 }
              if(strt_ent > 1)
                {
                   while (strt_ent > 1)
                   strt_ent -= 13;
                   if (strt_ent < 1)
                      {
                        strt_ent = 1;
                      }
                   update_screen = TRUE;
                   send_sockmsg(SCREEN_9_T,'u',strt_ent,ecg_name,result_msg);
               }
              else
                {
                 beep();
                }  /* endif */
           }    

        /*else if(input[strlen(input)-1]=='q'||input[strlen(input)-1]=='Q')*/
      else if((strcmp(input,"q")==0)||(strcmp(input,"Q")==0))
            {
              if (strlen(input) != 1)
                 {
                   PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                   getch();
                   CLRLIN(MSGLINE, 0);
                 }
                   return (0);
             }
        /*else if(input[strlen(input)-1]=='r'||input[strlen(input)-1]=='R')*/
       else if((strcmp(input,"r")==0)||(strcmp(input,"R")==0))
             {
               if (strlen(input) != 1)
                  {
                      PRTMSG(MSGLINE, 0, ("Please enter a valid option"));
                      getch();
                      CLRLIN(MSGLINE, 0);
                  }
               else
                  {
                      clear();
                      refresh();
                      send_sockmsg(SCREEN_9_T,0,strt_ent,ecg_name,result_msg);
           //        display_scn(stdscr, 0, 0, LINES,COLS, "TD_scn", 1, &row, &col, NUM_COLS, NUM_ROWS, 'n');
                   update_screen = TRUE;
                  }
             }

else
   {
     if(strlen(input)>0)
      {
        update_screen = TRUE;
        sprintf(workstr,"%s@%s",ecg_name,input);
        if (send_sockmsg(SCREEN_9_T_D,'w',strt_ent,workstr,result_msg) != 0) {
           PRTMSG(MSGLINE, 0, ("%s",result_msg));
           //getch();
           CLRLIN(MSGLINE, 0);
        }
     }
	   ;
    }
    }/* end of for*/
  }      /* endfor */
}        /* T_device */
