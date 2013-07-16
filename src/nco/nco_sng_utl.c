/* $Header: /data/zender/nco_20150216/nco/src/nco/nco_sng_utl.c,v 1.45 2013-07-16 04:26:06 zender Exp $ */

/* Purpose: String utilities */

/* Copyright (C) 1995--2013 Charlie Zender
   License: GNU General Public License (GPL) Version 3
   See http://www.gnu.org/copyleft/gpl.html for full license text */

#include "nco_sng_utl.h" /* String utilities */

#ifdef NEED_STRCASECMP
int /* O [enm] [-1,0,1] sng_1 [<,=,>] sng_2 */
strcasecmp /* [fnc] Lexicographical case-insensitive string comparison */
(const char *sng_1, /* I [sng] First string */
 const char *sng_2) /* I [sng] Second string */
{
  char chr_1;
  char chr_2;
  while(1){
    chr_1=tolower(*sng_1++);
    chr_2=tolower(*sng_2++);
    if(chr_1 < chr_2) return -1;
    if(chr_1 > chr_2) return 1;
    if(chr_1 == 0) return 0;
  } /* end while */
} /* end strcasecmp() */
#endif /* !NEED_STRCASECMP */

#ifdef NEED_STRCASESTR
char * /* O [sng] Pointer to sng_2 in sng_1 */
strcasestr /* [fnc] Lexicographical case-insensitive string search */
(const char *sng_1, /* I [sng] First string */
 const char *sng_2) /* I [sng] Second string */
{
  /* 20120706 Initial version discards const, triggers compiler warnings
     20120803 Kludge with strdup() to keep const intact */
  char *hys_ptr; /* Haystack pointer */
  char *startn=0;
  char *np=0;
  /* char *sng_1_dpl;
  char *sng_2_dpl;
  sng_1_dpl=(char *)strdup(sng_1);
  sng_2_dpl=(char *)strdup(sng_2);
  sng_1_dpl=(char *)nco_free(sng_1_dpl);
  sng_2_dpl=(char *)nco_free(sng_2_dpl); */
  /* Loop exits on NUL */
  for(hys_ptr=(char *)sng_1;*hys_ptr;hys_ptr++){
    if(np){
      if(toupper(*hys_ptr) == toupper(*np)){
	if(!*++np) return startn;
      }else{
	np=0;
      } /* endif uppercases match */
    }else if(toupper(*hys_ptr) == toupper(*sng_2)){
      np=(char *)sng_2+1;
      startn=hys_ptr;
    } /* else if */
  } /* end loop over haystack */
  return 0;
} /* end strcasestr() */
#endif /* !NEED_STRCASESTR */

#ifdef NEED_STRDUP
char * /* [sng] Copy of input string */
strdup /* [fnc] Duplicate string */
(const char *sng_in) /* [sng] String to duplicate */
{
  /* Purpose: Provide strdup() for broken systems 
     Input string must be NUL-terminated */
  int sng_lng=strlen(sng_in)+1;
  /* Use nco_malloc() even though strdup() is system function 
     This ensures all NCO code goes through nco_malloc()  */
  char *sng_out=(char *)nco_malloc(sng_lng*sizeof(char));
  if(sng_out) strcpy(sng_out,sng_in);
  return sng_out;
} /* end strdup() */
#endif /* !NEED_STRDUP */

#ifdef NEED_STRTOLL
long long int /* O [nbr] String as long long integer */
strtoll /* [fnc] Convert string to a long long integer */
(const char *nptr, 
 char **endptr, 
 int base)
{
  /* Purpose: Compatibility function for strtoll()
     Needed by some C++ compilers, e.g., AIX xlC
     20120703: rewrite to cast result of strtol() to long long and return */
  long long nbr_out;
  nbr_out=(long long)strtol(nptr,endptr,base);
  return nbr_out;
} /* end strtoll() */
#endif /* !NEED_STRTOLL */

char * /* O [sng] Parsed command line */
nco_cmd_ln_sng /* [fnc] Re-construct command line from arguments */
(const int argc, /* I [nbr] Argument count */
 CST_X_PTR_CST_PTR_CST_Y(char,argv)) /* I [sng] Command line argument values */
{
  /* Purpose: Re-construct command line from argument list and number */
  char *cmd_ln; /* [sng] Parsed command line */
  
  int cmd_ln_sz=0;
  int idx;

  for(idx=0;idx<argc;idx++){
    /* Add one to size of each argument for a space character */
    cmd_ln_sz+=(int)strlen(argv[idx])+1;
  } /* end loop over args */
  if(argc <= 0){
    cmd_ln=(char *)nco_malloc(sizeof(char));
    cmd_ln[0]='\0';
  }else{
    cmd_ln=(char *)nco_malloc(cmd_ln_sz*sizeof(char));
    (void)strcpy(cmd_ln,argv[0]);
    for(idx=1;idx<argc;idx++){
      (void)strcat(cmd_ln," ");
      (void)strcat(cmd_ln,argv[idx]);
    } /* end loop over args */
  } /* end else */

  return cmd_ln; /* [sng] Parsed command line */
} /* end nco_cmd_ln_sng() */

int /* O [nbr] Number of escape sequences translated */
sng_ascii_trn /* [fnc] Replace C language '\X' escape codes in string with ASCII bytes */
(char * const sng) /* I/O [sng] String to process */
{
  /* Purpose: Replace C language '\X' escape codes in string with ASCII bytes 
     Return number of escape sequences found and actually translated
     This should be same as number of bytes by which string length has shrunk
     For example, consecutive characters "\n" are translated into ASCII '\n' = 10
     Each such translation diminishes string length by one
     Function works for arbitrary number of escape codes in input string
     The escape sequence for NUL itself, \0, causes a warning and is not translated
     Input string must be NUL-terminated or NULL
     This procedure can only diminish, not lengthen, input string size
     Therefore it may safely be performed in-place, i.e., no string copy is necessary
     Translation is done in-place so copy original prior to calling sng_ascii_trn()!!!

     Address pointed to by sng does not change, but memory at that address is altered
     when characters are "moved to the left" if C language escape sequences are embedded.
     Thus string length may shrink */

  const char fnc_nm[]="sng_ascii_trn()"; /* [sng] Function name */

  nco_bool trn_flg; /* Translate this escape sequence */

  char *backslash_ptr; /* [ptr] Pointer to backslash character */
  char backslash_chr='\\'; /* [chr] Backslash character */

  int esc_sqn_nbr=0; /* Number of escape sequences found */
  int trn_nbr=0; /* Number of escape sequences translated */
  
  /* ncatted allows character attributes of 0 length
     Such "strings" do not have NUL-terminator and so may not safely be tested by strchr() */
  if(sng == NULL) return trn_nbr;
  
  /* C language '\X' escape codes are always preceded by a backslash */
  /* Check if control codes are embedded once before entering loop */
  backslash_ptr=strchr(sng,backslash_chr);

  while(backslash_ptr){
    /* Default is to translate this escape sequence */
    trn_flg=True;
    /* Replace backslash character by corresponding control code */
    switch(*(backslash_ptr+1)){ /* man ascii:Oct   Dec   Hex   Char \X  */
    case 'a': *backslash_ptr='\a'; break; /* 007   7     07    BEL '\a' Bell */
    case 'b': *backslash_ptr='\b'; break; /* 010   8     08    BS  '\b' Backspace */
    case 'f': *backslash_ptr='\f'; break; /* 014   12    0C    FF  '\f' Formfeed */
    case 'n': *backslash_ptr='\n'; break; /* 012   10    0A    LF  '\n' Linefeed */
    case 'r': *backslash_ptr='\r'; break; /* 015   13    0D    CR  '\r' Carriage return */
    case 't': *backslash_ptr='\t'; break; /* 011   9     09    HT  '\t' Horizontal tab */
    case 'v': *backslash_ptr='\v'; break; /* 013   11    0B    VT  '\v' Vertical tab */
    case '\\': *backslash_ptr='\\'; break; /* 134   92    5C    \   '\\' */
    case '\?': *backslash_ptr='\?'; break; /* Unsure why or if this works! */
    case '\'': *backslash_ptr='\''; break; /* Unsure why or if this works! */
    case '\"': *backslash_ptr='\"'; break; /* Unsure why or if this works! */
    case '0':	
      /* Translating \0 to NUL makes subsequent portion of input string invisible to all string functions */
      (void)fprintf(stderr,"%s: WARNING C language escape code %.2s found in string, not translating to NUL since this would make the subsequent portion of the string invisible to all C Standard Library string functions\n",prg_nm_get(),backslash_ptr); 
      trn_flg=False;
      /* 20101013: Tried changing above behavior to following, and it opened a Hornet's nest of problems... */
      /* *backslash_ptr='\0'; *//* 000   0     00    NUL '\0' */
      /*      (void)fprintf(stderr,"%s: WARNING translating C language escape code \"\\0\" found in user-supplied string to NUL. This will make the subsequent portion of the string, if any, invisible to C Standard Library string functions. And that may cause unintended consequences.\n",prg_nm_get());*/
      break;
    default: 
      (void)fprintf(stderr,"%s: WARNING No ASCII equivalent to possible C language escape code %.2s so no action taken\n",prg_nm_get(),backslash_ptr);
      trn_flg=False;
      break;
    } /* end switch */
    if(trn_flg){
      /* Remove character after backslash character */
      (void)memmove(backslash_ptr+1,backslash_ptr+2,(strlen(backslash_ptr+2)+1)*sizeof(char));
      /* Count translations performed */
      trn_nbr++;
    } /* end if */
    /* Look for next backslash starting at character following current escape sequence (but remember that not all escape sequences are translated) */
    if(trn_flg) backslash_ptr=strchr(backslash_ptr+1,backslash_chr); else backslash_ptr=strchr(backslash_ptr+2,backslash_chr);
    /* Count escape sequences */
    esc_sqn_nbr++;
  } /* end if */

  /* Usually there are no escape codes and sng still equals input value */
  if(dbg_lvl_get() > 3) (void)fprintf(stderr,"%s: DEBUG %s Found %d C-language escape sequences, translated %d of them\n",prg_nm_get(),fnc_nm,esc_sqn_nbr,trn_nbr);

  return trn_nbr;
} /* end sng_ascii_trn() */

void /* O [nbr]  */
sng_trm_trl_zro /* [fnc] Trim zeros trailing the decimal point from floating point string */
(char * const sng, /* I/O [sng] String to process */
 const int trl_zro_max) /* [nbr] Maximum number of trailing zeros allowed */
{
  /* Purpose: Trim zeros trailing decimal point from floating point string
     Source: 20130715 CSZ modified code that does not handle exponents at 
     http://stackoverflow.com/questions/277772/avoid-trailing-zeroes-in-printf */
  char *trl_zro_ptr; /* [sng] */
  char *dcm_ptr; /* [sng] */
  char *vld_ptr=NULL; /* [sng] */
  char chr_val; /* [chr] Character value */

  int cnt_zro_rmn; /* [nbr] Number of trailing zeros remaining until maximum reached */
  int nbr_vld_chr; /* [nbr] Number of valid characters to copy */
  
  /* Find decimal point, if any */
  dcm_ptr=strchr(sng,'.'); 
  if(dcm_ptr){
    /* Find last zero after decimal point, if any */
    trl_zro_ptr=strrchr(dcm_ptr,'0'); 
    if(trl_zro_ptr){
      /* Next character must be NUL or exponent (d,D,e,E) */
      chr_val=*(trl_zro_ptr+1);
      if(chr_val != '\0' || !isdigit(chr_val)) return;
      /* This is a trailing zero. Allow given number of trailing zeros. */
      cnt_zro_rmn=trl_zro_max;
      while(cnt_zro_rmn > 0){
	cnt_zro_rmn--;
	/* Shift pointer back one character. If that is zero, continue else return. */
	if(*trl_zro_ptr-- != '0') return;
      } /* end while */

      /* All characters to right are valid */
      vld_ptr=trl_zro_ptr+1;
      nbr_vld_chr=strlen(sng)-strlen(vld_ptr);

      /* Trim all remaining consecutive zeros leftward */
      while(*trl_zro_ptr == '0') *trl_zro_ptr--='\0';

      /* Copy allowed zeros and/or exponent, if any, to current location */
      strncpy(trl_zro_ptr,vld_ptr,nbr_vld_chr);
    } /* end if trl_zro_ptr */
  } /* end if dcm_ptr */

  return;
} /* end sng_trm_trl_zro() */
