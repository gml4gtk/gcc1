/* Parse command line arguments for bison,
   Copyright (C) 1984, 1986 Bob Corbett and Free Software Foundation, Inc.

BISON is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY.  No author or distributor accepts responsibility to anyone
for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.
Refer to the BISON General Public License for full details.

Everyone is granted permission to copy, modify and redistribute BISON,
but only under the conditions described in the BISON General Public
License.  A copy of this license is supposed to have been given to you
along with BISON so you can know your rights and responsibilities.  It
should be in a file named COPYING.  Among other things, the copyright
notice and this notice must be preserved on all copies.

 In other words, you are welcome to use, share and improve this program.
 You are forbidden to forbid anyone else to use, share and improve
 what you give them.   Help stamp out software-hoarding!  */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "files.h"

int verboseflag;
int definesflag;
extern int fixed_outfiles;/* JF */
extern void fatal (char *s);


void getargs (int argc, char *argv[])
{
  register int c;
  char *p = argv[0];
  char *lastcomponent;

  extern int optind;
  extern char *optarg;

  verboseflag = 0;
  definesflag = 0;
  fixed_outfiles = 0;

  /* See if the program was invoked as "yacc".  */

  lastcomponent = p;
  while (*p)
    {
      if (*p == '/')
	lastcomponent = p + 1;
      p++;
    }
  if (! strcmp (lastcomponent, "yacc"))
    /* If so, pretend we have "-y" as argument.  */
    fixed_outfiles = 1;

  while ((c = getopt (argc, argv, "yYvVdDo:O:")) != EOF)
    switch (c)
      {
      case 'y':
      case 'Y':
	fixed_outfiles = 1;
	break;

      case 'v':
      case 'V':
	verboseflag = 1;
	break;

      case 'd':
      case 'D':
	definesflag = 1;
	break;

      case 'o':
      case 'O':
	spec_outfile = optarg;
      }

  if (optind == argc)
    fatal("grammar file not specified");
  else
    infile = argv[optind];

  if (optind < argc - 1)
    fprintf(stderr, "bison: warning: extra arguments ignored\n");
}
