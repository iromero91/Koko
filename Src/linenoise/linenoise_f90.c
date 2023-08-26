////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2020 The Koko Project Developers
//
// See the file COPYRIGHT.md in the top-level directory of this
// distribution
//
// This file is part of Koko.
//
// Koko is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Koko is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Koko; see the file COPYING.  If not, see
// <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////

// Provides a Fortran 90 - callable interface to the functions of the
// linenoise library for command line editing in a VT100 terminal.
//
// Ulf GRIESMANN, May 2020

#include "linenoise.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PROMPT_LEN 64   // max number of characters in prompt
#define FNAME_LEN 256   // max file name length
#define MAX_HISTORY 250 // max length of command history
#define ESC '\x1b'      // ESC character


// ANSI 8-bit foreground color code for prompt (default is black)
static int prompt_color = 0;


//--- For command completion ---------------------------------------------

static int hidx = -1;  // for command completion

// Is called every time <tab> is pressed. Looks through the command
// history beginning with the most recent command to find one that
// matches a prefix (partial command)
void
CompletionFunc(char const* prefix, linenoiseCompletions* completion_list)
{
   int i;
   char *history_line;

   // look up commands going back in history
   if ( hidx < 0 ) {
      hidx = linenoiseHistoryLen() -  1;  // index is 0-based
   }
   
   for (i = hidx; i >= 0; i--) {
      history_line = linenoiseHistoryLine(i);
      if ((history_line != NULL) && (strncmp(prefix, history_line, strlen(prefix)) == 0)) {
         linenoiseAddCompletion(completion_list, history_line);
         hidx = i - 1; // continue here at next <tab>
         break;
      }
      else {
         free(history_line);
      }
   }
}


//--- Prototypes ---------------------------------------------------------

void nextline( const char*, const int, char*, int );
void loadhistory( const char *, int );
void savehistory( const char *, int );
void promptcolor( int );
void init_tab_completion(void);


//--- Interface functions ------------------------------------------------

// Prompts a user and returns the user response
//
// INPUT
// prompt :   pointer to character string with prompt
// ncprs :    number of characters in prompt string
// lenrs :    (Fortran) length of the response string
//
// OUTPUT
// response : character string with user response
//
void
nextline( const char* prompt, const int ncprs, char *response, int lenrs )
{
   char tmp_prompt[PROMPT_LEN];
   char ln_prompt[PROMPT_LEN];
   int nc;
   char *ln_response;

   // copy Fortran prompt string to C string
   if ( ncprs > PROMPT_LEN - 1 ) {
      nc = PROMPT_LEN - 1;
   }
   else {
      nc = ncprs;
   }
   strncpy(tmp_prompt, prompt, nc);
   tmp_prompt[nc] = '\0';

   // construct prompt with 8-bit color ESC sequences
   sprintf(ln_prompt, "%c[38;5;%dm%s%c[38;5;0m", ESC, prompt_color, tmp_prompt, ESC);
   
   // prompt the user and return the answer
   ln_response = linenoise(ln_prompt);

   // copy response to calling subroutine
   if (ln_response != NULL) {
     nc = strlen(ln_response);
     strncpy(response, ln_response, lenrs);
   } else {
     // Returned empty response, exit
     strcpy(response, "exit");
     nc = 4;
   }

   if (nc < lenrs) {
      memset(response+nc, ' ', lenrs-nc);
   }

   // add response to command history
   if(ln_response)
     linenoiseHistoryAdd(ln_response);

   // next completion re-starts at last command
   hidx = -1;

   // free tmp result buffer
   free(ln_response);
}


//------------------------------------------------------------------------

// Loads the contents of the ~/.koko_history file
// The function is only called when the history file
// exists. When an error occurs while reading the
// history file it is ignored.
//
// INPUT
// fname :  Fortran string with the fully qualified name
//          of the history file ( not \0-terminated )
// ncs :    number of characters in file name.
void
loadhistory( const char *fname, int ncs )
{
   char cfname[FNAME_LEN];
   
   // make C-style file name string
   strncpy(cfname, fname, ncs);
   cfname[ncs] = '\0';
   linenoiseHistoryLoad(cfname);
}


//------------------------------------------------------------------------

// Saves up to MAX_HISTORY commands from the command history in
// the ~/.koko_history file
//
// INPUT
// 
// fname :  Fortran string with the fully qualified name
//          of the history file ( not \0-terminated )
// ncs :    number of characters in file name.
void
savehistory( const char *fname, int ncs )
{
   char cfname[FNAME_LEN];
   
   // make C-style file name string
   strncpy(cfname, fname, ncs);
   cfname[ncs] = '\0';

   // save history
   linenoiseHistorySave( cfname, MAX_HISTORY );
   linenoiseHistoryFree(); // called only on Koko exit
}


//------------------------------------------------------------------------

// Sets the ANSI color code for the prompt
//
// INPUT
// acc  :  ANSI color code
void
promptcolor( int acc )
{
   prompt_color = acc;
}


//------------------------------------------------------------------------

// Installs the callback function for TAB command completion
void
init_tab_completion(void)
{
   // set up command completion
   linenoiseSetCompletionCallback( CompletionFunc );
}
