/*
  prgbc

  A Commodore 64 BASIC compiler. Translates BASIC source files into
  C64 PRG files.
*/

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


typedef int32_t   s32;

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;

typedef u8  byte_t;

#ifndef BOOL 
#define BOOL int
#endif
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif



#define NUM_BASIC_TOKENS  0x4C
#define MAX_LABEL_LENGTH  32


/* Translation table for keycodes between modern ASCII standard and
   C64 PETSCII */
char* PETSCII_table[] =
{
    /* 00 */  "{PETSCII_00}",
    /* 01 */  "{PETSCII_01}",
    /* 02 */  "{PETSCII_02}",
    /* 03 */  "{PETSCII_STOP}",
    /* 04 */  "{PETSCII_04}",
    /* 05 */  "{PETSCII_WHITE}",
    /* 06 */  "{PETSCII_06}",
    /* 07 */  "{PETSCII_07}",
    /* 08 */  "{PETSCII_DISABLE_SHIFT_CMDR}",
    /* 09 */  "{PETSCII_ENABLE_SHIFT_CMDR}",
    /* 0A */  "{PETSCII_0A}",
    /* 0B */  "{PETSCII_0B}",
    /* 0C */  "{PETSCII_0C}",
    /* 0D */  "{PETSCII_RETURN}",
    /* 0E */  "{PETSCII_LWR_UPR_CHARSET}",
    /* 0F */  "{PETSCII_0F}",
    /* 10 */  "{PETSCII_10}",
    /* 11 */  "{PETSCII_DOWN}",
    /* 12 */  "{PETSCII_RVS}",
    /* 13 */  "{PETSCII_HOME}",
    /* 14 */  "{PETSCII_DELETE}",
    /* 15 */  "{PETSCII_15}",
    /* 16 */  "{PETSCII_16}",
    /* 17 */  "{PETSCII_17}",
    /* 18 */  "{PETSCII_18}",
    /* 19 */  "{PETSCII_19}",
    /* 1A */  "{PETSCII_1A}",
    /* 1B */  "{PETSCII_1B}",
    /* 1C */  "{PETSCII_RED}",
    /* 1D */  "{PETSCII_RIGHT}",
    /* 1E */  "{PETSCII_GREEN}",
    /* 1F */  "{PETSCII_BLUE}",
    /* 20 */  " ",
    /* 21 */  "!",
    /* 22 */  "\"",
    /* 23 */  "#",
    /* 24 */  "$",
    /* 25 */  "%",
    /* 26 */  "&",
    /* 27 */  "'",
    /* 28 */  "(",
    /* 29 */  ")",
    /* 2A */  "*",
    /* 2B */  "+",
    /* 2C */  ",",
    /* 2D */  "-",
    /* 2E */  ".",
    /* 2F */  "/",
    /* 30 */  "0",
    /* 31 */  "1",
    /* 32 */  "2",
    /* 33 */  "3",
    /* 34 */  "4",
    /* 35 */  "5",
    /* 36 */  "6",
    /* 37 */  "7",
    /* 38 */  "8",
    /* 39 */  "9",
    /* 3A */  ":",
    /* 3B */  ";",
    /* 3C */  "<",
    /* 3D */  "=",
    /* 3E */  ">",
    /* 3F */  "?",
    /* 40 */  "@",
    /* 41 */  "A",
    /* 42 */  "B",
    /* 43 */  "C",
    /* 44 */  "D",
    /* 45 */  "E",
    /* 46 */  "F",
    /* 47 */  "G",
    /* 48 */  "H",
    /* 49 */  "I",
    /* 4A */  "J",
    /* 4B */  "K",
    /* 4C */  "L",
    /* 4D */  "M",
    /* 4E */  "N",
    /* 4F */  "O",
    /* 50 */  "P",
    /* 51 */  "Q",
    /* 52 */  "R",
    /* 53 */  "S",
    /* 54 */  "T",
    /* 55 */  "U",
    /* 56 */  "V",
    /* 57 */  "W",
    /* 58 */  "X",
    /* 59 */  "Y",
    /* 5A */  "Z",
    /* 5B */  "[",
    /* 5C */  "{PETSCII_POUNDSTERLING}",
    /* 5D */  "]",
    /* 5E */  "^", /* {PETSCII_ARROW_UP} */
    /* 5F */  "{PETSCII_ARROW_LEFT}",
    /* 60 */  "{PETSCII_GFX_60}",
    /* 61 */  "{PETSCII_GFX_61}",
    /* 62 */  "{PETSCII_GFX_62}",
    /* 63 */  "{PETSCII_GFX_63}",
    /* 64 */  "{PETSCII_GFX_64}",
    /* 65 */  "{PETSCII_GFX_65}",
    /* 66 */  "{PETSCII_GFX_66}",
    /* 67 */  "{PETSCII_GFX_67}",
    /* 68 */  "{PETSCII_GFX_68}",
    /* 69 */  "{PETSCII_GFX_69}",
    /* 6A */  "{PETSCII_GFX_6A}",
    /* 6B */  "{PETSCII_GFX_6B}",
    /* 6C */  "{PETSCII_GFX_6C}",
    /* 6D */  "{PETSCII_GFX_6D}",
    /* 6E */  "{PETSCII_GFX_6E}",
    /* 6F */  "{PETSCII_GFX_6F}",
    /* 70 */  "{PETSCII_GFX_70}",
    /* 71 */  "{PETSCII_GFX_71}",
    /* 72 */  "{PETSCII_GFX_72}",
    /* 73 */  "{PETSCII_GFX_73}",
    /* 74 */  "{PETSCII_GFX_74}",
    /* 75 */  "{PETSCII_GFX_75}",
    /* 76 */  "{PETSCII_GFX_76}",
    /* 77 */  "{PETSCII_GFX_77}",
    /* 78 */  "{PETSCII_GFX_78}",
    /* 79 */  "{PETSCII_GFX_79}",
    /* 7A */  "{PETSCII_GFX_7A}",
    /* 7B */  "{PETSCII_GFX_7B}",
    /* 7C */  "{PETSCII_GFX_7C}",
    /* 7D */  "{PETSCII_GFX_7D}",
    /* 7E */  "{PETSCII_GFX_7E}",
    /* 7F */  "{PETSCII_GFX_7F}",
    /* 80 */  "{PETSCII_GFX_80}",
    /* 81 */  "{PETSCII_ORANGE}",
    /* 82 */  "{PETSCII_GFX_82}",
    /* 83 */  "{PETSCII_RUN}",
    /* 84 */  "{PETSCII_GFX_84}",
    /* 85 */  "{PETSCII_F1}",
    /* 86 */  "{PETSCII_F3}",
    /* 87 */  "{PETSCII_F5}",
    /* 88 */  "{PETSCII_F7}",
    /* 89 */  "{PETSCII_F2}",
    /* 8A */  "{PETSCII_F4}",
    /* 8B */  "{PETSCII_F6}",
    /* 8C */  "{PETSCII_F8}",
    /* 8D */  "{PETSCII_SHIFT_RETURN}",
    /* 8E */  "{PETSCII_UPR_GFX_CHARSET}",
    /* 8F */  "{PETSCII_GFX_8F}",
    /* 90 */  "{PETSCII_BLACK}",
    /* 91 */  "{PETSCII_UP}",
    /* 92 */  "{PETSCII_OFF}",
    /* 93 */  "{PETSCII_CLR}",
    /* 94 */  "{PETSCII_INSERT}",
    /* 95 */  "{PETSCII_BROWN}",
    /* 96 */  "{PETSCII_PINK}",
    /* 97 */  "{PETSCII_DK_GREY}",
    /* 98 */  "{PETSCII_GREY}",
    /* 99 */  "{PETSCII_LT_GREEN}",
    /* 9A */  "{PETSCII_LT_BLUE}",
    /* 9B */  "{PETSCII_LT_GREY}",
    /* 9C */  "{PETSCII_PURPLE}",
    /* 9D */  "{PETSCII_LEFT}",
    /* 9E */  "{PETSCII_YELLOW}",
    /* 9F */  "{PETSCII_CYAN}",
    /* A0 */  "{PETSCII_GFX_F0}",
    /* A1 */  "{PETSCII_GFX_F1}",
    /* A2 */  "{PETSCII_GFX_F2}",
    /* A3 */  "{PETSCII_GFX_F3}",
    /* A4 */  "{PETSCII_GFX_F4}",
    /* A5 */  "{PETSCII_GFX_F5}",
    /* A6 */  "{PETSCII_GFX_F6}",
    /* A7 */  "{PETSCII_GFX_F7}",
    /* A8 */  "{PETSCII_GFX_F8}",
    /* A9 */  "{PETSCII_GFX_F9}",
    /* AA */  "{PETSCII_GFX_FA}",
    /* AB */  "{PETSCII_GFX_FB}",
    /* AC */  "{PETSCII_GFX_FC}",
    /* AD */  "{PETSCII_GFX_FD}",
    /* AE */  "{PETSCII_GFX_FE}",
    /* AF */  "{PETSCII_GFX_FF}",
    /* B0 */  "{PETSCII_GFX_B0}",
    /* B1 */  "{PETSCII_GFX_B1}",
    /* B2 */  "{PETSCII_GFX_B2}",
    /* B3 */  "{PETSCII_GFX_B3}",
    /* B4 */  "{PETSCII_GFX_B4}",
    /* B5 */  "{PETSCII_GFX_B5}",
    /* B6 */  "{PETSCII_GFX_B6}",
    /* B7 */  "{PETSCII_GFX_B7}",
    /* B8 */  "{PETSCII_GFX_B8}",
    /* B9 */  "{PETSCII_GFX_B9}",
    /* BA */  "{PETSCII_GFX_BA}",
    /* BB */  "{PETSCII_GFX_BB}",
    /* BC */  "{PETSCII_GFX_BC}",
    /* BD */  "{PETSCII_GFX_BD}",
    /* BE */  "{PETSCII_GFX_BE}",
    /* BF */  "{PETSCII_GFX_BF}",
    /* C0 */  "{PETSCII_GFX_00}",
    /* C1 */  "{PETSCII_GFX_01}",
    /* C2 */  "{PETSCII_GFX_02}",
    /* C3 */  "{PETSCII_GFX_03}",
    /* C4 */  "{PETSCII_GFX_04}",
    /* C5 */  "{PETSCII_GFX_05}",
    /* C6 */  "{PETSCII_GFX_06}",
    /* C7 */  "{PETSCII_GFX_07}",
    /* C8 */  "{PETSCII_GFX_08}",
    /* C9 */  "{PETSCII_GFX_09}",
    /* CA */  "{PETSCII_GFX_0A}",
    /* CB */  "{PETSCII_GFX_0B}",
    /* CC */  "{PETSCII_GFX_0C}",
    /* CD */  "{PETSCII_GFX_0D}",
    /* CE */  "{PETSCII_GFX_0E}",
    /* CF */  "{PETSCII_GFX_0F}",
    /* D0 */  "{PETSCII_GFX_00}",
    /* D1 */  "{PETSCII_GFX_01}",
    /* D2 */  "{PETSCII_GFX_02}",
    /* D3 */  "{PETSCII_GFX_03}",
    /* D4 */  "{PETSCII_GFX_04}",
    /* D5 */  "{PETSCII_GFX_05}",
    /* D6 */  "{PETSCII_GFX_06}",
    /* D7 */  "{PETSCII_GFX_07}",
    /* D8 */  "{PETSCII_GFX_08}",
    /* D9 */  "{PETSCII_GFX_09}",
    /* DA */  "{PETSCII_GFX_0A}",
    /* DB */  "{PETSCII_GFX_0B}",
    /* DC */  "{PETSCII_GFX_0C}",
    /* DD */  "{PETSCII_GFX_0D}",
    /* DE */  "{PETSCII_GFX_0E}",
    /* DF */  "{PETSCII_GFX_0F}",
    /* E0 */  "{PETSCII_GFX_00}",
    /* E1 */  "{PETSCII_GFX_01}",
    /* E2 */  "{PETSCII_GFX_02}",
    /* E3 */  "{PETSCII_GFX_03}",
    /* E4 */  "{PETSCII_GFX_04}",
    /* E5 */  "{PETSCII_GFX_05}",
    /* E6 */  "{PETSCII_GFX_06}",
    /* E7 */  "{PETSCII_GFX_07}",
    /* E8 */  "{PETSCII_GFX_08}",
    /* E9 */  "{PETSCII_GFX_09}",
    /* EA */  "{PETSCII_GFX_0A}",
    /* EB */  "{PETSCII_GFX_0B}",
    /* EC */  "{PETSCII_GFX_0C}",
    /* ED */  "{PETSCII_GFX_0D}",
    /* EE */  "{PETSCII_GFX_0E}",
    /* EF */  "{PETSCII_GFX_0F}",
    /* F0 */  "{PETSCII_GFX_00}",
    /* F1 */  "{PETSCII_GFX_01}",
    /* F2 */  "{PETSCII_GFX_02}",
    /* F3 */  "{PETSCII_GFX_03}",
    /* F4 */  "{PETSCII_GFX_04}",
    /* F5 */  "{PETSCII_GFX_05}",
    /* F6 */  "{PETSCII_GFX_06}",
    /* F7 */  "{PETSCII_GFX_07}",
    /* F8 */  "{PETSCII_GFX_08}",
    /* F9 */  "{PETSCII_GFX_09}",
    /* FA */  "{PETSCII_GFX_0A}",
    /* FB */  "{PETSCII_GFX_0B}",
    /* FC */  "{PETSCII_GFX_0C}",
    /* FD */  "{PETSCII_GFX_0D}",
    /* FE */  "{PETSCII_GFX_0E}",
    /* FF */  "{PETSCII_GFX_0F}",
};


/* Offsets into this table equal the token value - 0x80 */
char* token_list[] =
{
    /* 80 */ "END",
    /* 81 */ "FOR",
    /* 82 */ "NEXT",
    /* 83 */ "DATA",
    /* 84 */ "INPUT#",
    /* 85 */ "INPUT",
    /* 86 */ "DIM",
    /* 87 */ "READ",
    /* 88 */ "LET",
    /* 89 */ "GOTO",
    /* 8A */ "RUN",
    /* 8B */ "IF",
    /* 8C */ "RESTORE",
    /* 8D */ "GOSUB",
    /* 8E */ "RETURN",
    /* 8F */ "REM",
    /* 90 */ "STOP",
    /* 91 */ "ON",
    /* 92 */ "WAIT",
    /* 93 */ "LOAD",
    /* 94 */ "SAVE",
    /* 95 */ "VERIFY",
    /* 96 */ "DEF",
    /* 97 */ "POKE",
    /* 98 */ "PRINT#",
    /* 99 */ "PRINT",
    /* 9A */ "CONT",
    /* 9B */ "LIST",
    /* 9C */ "CLR",
    /* 9D */ "CMD",
    /* 9E */ "SYS",
    /* 9F */ "OPEN",
    /* A0 */ "CLOSE",
    /* A1 */ "GET",
    /* A2 */ "NEW",
    /* A3 */ "TAB(",
    /* A4 */ "TO",
    /* A5 */ "FN",
    /* A6 */ "SPC(",
    /* A7 */ "THEN",
    /* A8 */ "NOT",
    /* A9 */ "STEP",
    /* AA */ "+",
    /* AB */ "-",
    /* AC */ "*",
    /* AD */ "/",
    /* AE */ "^",
    /* AF */ "AND",
    /* B0 */ "OR",
    /* B1 */ ">",
    /* B2 */ "=",
    /* B3 */ "<",
    /* B4 */ "SGN",
    /* B5 */ "INT",
    /* B6 */ "ABS",
    /* B7 */ "USR",
    /* B8 */ "FRE",
    /* B9 */ "POS",
    /* BA */ "SQR",
    /* BB */ "RND",
    /* BC */ "LOG",
    /* BD */ "EXP",
    /* BE */ "COS",
    /* BF */ "SIN",
    /* C0 */ "TAN",
    /* C1 */ "ATN",
    /* C2 */ "PEEK",
    /* C3 */ "LEN",
    /* C4 */ "STR$",
    /* C5 */ "VAL",
    /* C6 */ "ASC",
    /* C7 */ "CHR$",
    /* C8 */ "LEFT$",
    /* C9 */ "RIGHT$",
    /* CA */ "MID$",
    /* CB */ "GO"
};


/* Maximum line length in C64 BASIC is 80 chars (two physical 40-char
   lines). Allocate a very large buffer for potential insertion of
   PETSCII placeholder strings. */
#define MAX_SOURCE_LINE_LEN  2048
#define MAX_LINE_NUMBER      63999
struct BASIC_line
{
  s32    line_no;
  u32    source_line_number;
  char   source_line[MAX_SOURCE_LINE_LEN];
  byte_t tokenized_line[MAX_SOURCE_LINE_LEN];
  char   label[MAX_LABEL_LENGTH];

  struct BASIC_line*   next;
};

struct BASIC_program
{
  s32    last_line_no;
  struct BASIC_line*   first_line;
};
struct BASIC_program program;

struct source_file
{
  char* buffer;
  u32   buf_len;
  u32   pos;
};

struct global_args
{
  char*   src_path;
  char*   prg_path;
  u16     load_address;
};
struct global_args args;


/*
  SyntaxError
  
  Displays a message for BASIC syntax errors.
*/
void
SyntaxError(s32 line_no, char* msg, ...)
{
  /* TODO: Rework this to use the line number within the source file
     instead of the BASIC line number */
  char buffer[512];
  va_list args;
  va_start(args, msg);
  vsprintf(buffer, msg, args);
  va_end(args);
  fprintf(stderr, "SYNTAX ERROR: ");
  if (line_no >= 0)
    fprintf(stderr, "Line %u: ", (u16)line_no);
  fprintf(stderr, "%s\n", buffer);

  exit(-1);
}


/*
  StripWhitespace

  Remove beginning and trailing whitespace from line.
*/
void
StripWhitespace(char* line)
{
  if (strlen(line) < 1) return;

  /* Skip past beginning whitespace */
  u16 begin = 0;
  while (line[begin] &&
         (line[begin] == ' ' ||
          line[begin] == '\t'))
    ++begin;

  /* Copy from beginning of non-whitespace to front of line */
  if (begin > 0)
  {
    char temp_buffer[MAX_SOURCE_LINE_LEN];
    memset(temp_buffer, 0, MAX_SOURCE_LINE_LEN);
    strncpy(temp_buffer, &line[begin], MAX_SOURCE_LINE_LEN);
    strncpy(line, temp_buffer, MAX_SOURCE_LINE_LEN);
  }
  if (strlen(line) < 1) return;

  u16 end = strlen(line)-1;
  while (end > 0 &&
         (line[end] == ' ' ||
          line[end] == '\t'))
    --end;
  line[end+1] = '\0';
}


/*
  ConvertLowercaseToUppercase

  Convert all lowercase alpabetic characters in line to
  uppercase. This is necessary for compatibility with the C64's
  PETSCII character set.
*/
void
ConvertLowercaseToUppercase(char* line)
{
  while (*line)
  {
    if (*line >= 'a' &&
        *line <= 'z')
      *line -= 0x20;
    ++line;
  }
}


/*
  FindTokenIndex

  Return the index of keyword in token_list.
*/
int
FindTokenIndex(char* keyword)
{
  for (u8 i = 0;
       i < (sizeof(token_list) / sizeof(char*));
       ++i)
  {
    if (strcmp(keyword, token_list[i]) == 0)
      return (int)i;
  }
  return -1;
}


/*
  IsValidLabelChar
  
  Checks if c is a valid label character.

  Returns TRUE if c is a valid label character, FALSE otherwise.
*/
BOOL
IsValidLabelChar(char c)
{
  if (isalnum(c) ||
      c == '_') 
    return TRUE;
  else
    return FALSE;
}


/*
  IsLabel

  Determine if a line is a label. Labels must follow the following
  format:

   - Must be alone on a line Must be no longer than MAX_LABEL_LENGTH
     (32) *without* the trailing colon
   - Are case insensitive
   - Must not be identical to a reserved BASIC keyword
   - Must be begin with an alphabetic character
   - Must only contain alphanumeric characters or underscores
   - Must end with a colon
   
  Returns TRUE if line designates a label, FALSE otherwise
*/
BOOL
IsLabel(char* line)
{
  StripWhitespace(line);

  char* c = line;
  if (strlen(line) > MAX_LABEL_LENGTH+1)
    return FALSE;
  if (!isalpha(*c))
    return FALSE;
  while (*c != ':')
  {
    if (!IsValidLabelChar(*c))
      return FALSE;
    ++c;
  }
  if (*(++c) != '\0')
    return FALSE;

  if (strlen(line) > MAX_LABEL_LENGTH+1)
  {
    SyntaxError(-1, "Label length too long (maximum: %u)", MAX_LABEL_LENGTH);
    return FALSE;  /* This shouldn't actually return. (SyntaxError exits) */
  }

  /* Check for conflict with BASIC keywords */
  char temp_label[MAX_LABEL_LENGTH+1];
  /* -1 to remove trailing colon */
  strncpy(temp_label, line, strlen(line)-1);
  if (FindTokenIndex(temp_label) >= 0)
  {
    SyntaxError(-1, "Label conflicts with BASIC keyword: %s\n", temp_label);
    return FALSE;  /* This shouldn't actually return. (SyntaxError exits) */
  }

  return TRUE;
}


/*
  ReplaceStringWithByte

  Replace first occurance of target in line with byte.
  
  Returns non-zero if a replacement occurs, zero in no replacement
  occurs.
*/
int
ReplaceStringWithByte(byte_t* line, char* target, byte_t byte)
{
  char* location = strstr((char*)line, target);
  if (!location) return 0;

  u16 target_len = strlen(target);
  if (target_len == 1)
  {
    location[0] = byte;
    return 1;
  }

  char buffer[MAX_SOURCE_LINE_LEN];
  buffer[0] = byte;
  buffer[1] = 0;
  strcat(buffer, &location[target_len]);
  strcpy(location, buffer);

  return 1;
}


/*
  ReplaceStringWithString
  
  Replace first occurance in dst of target with sub.
  
  Returns non-zero if a replacement occurs, zero in no replacement
  occurs.
*/
int
ReplaceStringWithString(char* dst, char* target, char* sub)
{
  char* location = strstr(dst, target);
  if (!location) return 0;

  u16 target_len = strlen(target);
  u16 sub_len    = strlen(sub);

  char buffer[MAX_SOURCE_LINE_LEN];
  memset(buffer, 0, MAX_SOURCE_LINE_LEN);
  strncpy(buffer, sub, sub_len);
  strcat(buffer, &location[target_len]);
  strcpy(location, buffer);

  return 1;
}

/*
  FindPETSCIIPlaceholderIndex

  Return the index of placeholder in PETSCII_table.
*/
int
FindPETSCIIPlaceholderIndex(char* placeholder)
{
  for (u16 i = 0;
       i < (sizeof(PETSCII_table) / sizeof(char*));
       ++i)
  {
    if (strcmp(placeholder, PETSCII_table[i]) == 0)
      return (int)i;
  }
  return -1;
}


/*
  TranslateToken

  Translate a BASIC keyword/operator into the corresponding BASIC
  token.
*/
int
TranslateToken(char* keyword)
{
  byte_t token_index = FindTokenIndex(keyword);
  return token_index + 0x80;
}


/*
  TranslatePETSCIIPlaceholder

  Translate a PETSCII placeholder string to the correct PETSCII byte.
*/
int
TranslatePETSCIIPlaceholder(char* string)
{
  if (string[0] != '{')
    return -1;

  int len = strlen(string);
  char* begin = &string[1];
  char* end   = &string[len-1];
  *end = 0;
  char temp_buffer[32];
  strncpy(temp_buffer, begin, len-2);
  temp_buffer[len-1] = '\0';

  return 0;
}


/*
  TranslateASCIIToPETSCII
  
  Translate a string from ASCII encoding to PETSCII, translating
  placeholder strings where necessary.
*/
void
TranslateASCIIToPETSCII(byte_t* line)
{
  /* For each possible placeholder string, scan the line for the
     placeholder, and replace any found placeholder with the correct
     string */
  for (u16 petscii_index = 0;
       petscii_index < (sizeof(PETSCII_table) / sizeof(char*));
       ++petscii_index)
  {
    char* location = (char*)line;
    while ((location = strstr(location, PETSCII_table[petscii_index])))
    {
      ReplaceStringWithByte((byte_t*)location, PETSCII_table[petscii_index], (byte_t)petscii_index);
      ++location;
    }
  }
}

/*
  Program_PrintLines

  Print each line of the program. Used for debugging purposes.
*/
void
Program_PrintLines(struct BASIC_program* program)
{
  assert(program);
  assert(program->first_line);

  struct BASIC_line* curr_line = program->first_line;
  while (curr_line)
  {
    if (strlen(curr_line->label))
      printf("\n%s:\n", curr_line->label);
    int digits_printed = 0;
    printf("%d%n %s\n", curr_line->line_no, &digits_printed, curr_line->source_line);
    if (strlen((char*)curr_line->tokenized_line) > 0)
    {
      while (digits_printed-- >= 0)
        printf(" ");
      printf("%s\n", (char*)curr_line->tokenized_line);
    }
    curr_line = curr_line->next;
  }
}

/*
  Program_AddLine
  
  Add line to program line list
*/
void
Program_AddLine(struct BASIC_program* program, struct BASIC_line* line)
{
  /* Generate a line number if none was provided */
  if (line->line_no < 0)
    line->line_no = program->last_line_no + 1;

  if (line->line_no > MAX_LINE_NUMBER)
  {
    SyntaxError(line->line_no, "Line number too high (maximum: %d)", MAX_LINE_NUMBER);
  }

  /* Are we the first line? */
  if (!program->first_line)
  {
    program->first_line = line;
    program->last_line_no = line->line_no;
    return;
  }

  /* We *are NOT* the first line */
  struct BASIC_line* prev_line = 0;
  struct BASIC_line* curr_line = program->first_line;
  while (curr_line)
  {
    /* Check for duplicate line numbers */
    if (line->line_no == curr_line->line_no)
    {
      fprintf(stderr, "%s\n", (char*)line->source_line);
      SyntaxError(line->line_no, "Duplicate line number");
    }
    /* Sort by line numbers, ascending */
    if (line->line_no < curr_line->line_no) 
    {
      prev_line->next = line;
      line->next = curr_line;
      program->last_line_no = line->line_no;
      return;
    }

    prev_line = curr_line;
    curr_line = curr_line->next;
  }
  /* We've reached the end of the list; append line */
  prev_line->next = line;
  program->last_line_no = line->line_no;
}


/*
  Program_FindLineNumberByLabel
  
  Find the BASIC line number in program associated with label.
  
  Returns the associated line number if the label exists, or -1 if the
  label does not exist.
*/
s32
Program_FindLineNumberByLabel(struct BASIC_program* program, char* label)
{
  if (!program ||
      !program->first_line)
    return -1; /* Empty or non-existing program */

  struct BASIC_line* line = program->first_line;
  while(line)
  {
    if (strncmp(line->label, label, MAX_LABEL_LENGTH) == 0)
      return line->line_no;
    line = line->next;
  }

  return -1;
}


/*
  TokenizeLine

  Replace all BASIC keywords and operators in line with the
  corresponding BASIC token.
*/
void
TokenizeLine(byte_t* line)
{
  /* 
     For each byte, do a string compare against every
     keyword/operator. If matched, replace the keyword/operator with
     the correct token.

     Note the following conditions:
     
     - If a REM statement is encountered, all remaining characters on
     the line should be copied directly (not tokenized)
       
     - If a DATA statement is encountered, all characters should be
     copied directly (not tokenized) up until either a ':' or end of
     line
  */
  char* location = (char*)line;
  while (*location)
  {
    /* Don't tokenize anything within quotes */
    if (*location == '"')
    {
      ++location;
      while (*location &&
             *location != '"')
        ++location;
      ++location;
      continue;
    }

    /* Test against all possible keywords */
    for (int i = 0;
         i < NUM_BASIC_TOKENS;
         ++i)
    {
      char* keyword = token_list[i];
      if (strncmp(location, keyword, strlen(keyword)) == 0)
      {
        byte_t token = TranslateToken(keyword);
        ReplaceStringWithByte(line, keyword, token);

        if (token == TranslateToken("REM"))
        {
          /* Skip entire line after REM */
          return;
        }
        else if (token == TranslateToken("DATA"))
        {
          ++location;
          /* Skip to colon or newline after DATA */
          while (*location &&
                 *location != ':')
            ++location;
          if (!*location)
            return;
        }

        /* Keyword tokenized; break for loop */
        break;
      }
    }
      
    ++location;
  }
}


/*
  LoadSrc

  Load a BASIC source file located at path into a source_file struct.

  Return: Pointer to byte buffer
*/
void      
LoadSrc(struct source_file* source_file, char* path)
{
  assert(source_file);

  FILE* fp = fopen(path, "rb");
  if (!fp)
  {
    fprintf(stderr, "Failed to open file %s\n", path);
    exit(-1);
  }
  struct stat fs;
  fstat(fileno(fp), &fs);
  source_file->buf_len = fs.st_size;
  source_file->buffer = (char*)malloc(source_file->buf_len);
  fread(source_file->buffer, 1, fs.st_size, fp);
  fclose(fp);
}


/*
  ReadLine

  Read the next line from the source file into line.
*/
int
ReadLine(struct source_file* source_file, char* line)
{
  if (source_file->pos >= source_file->buf_len) return -1;

  int i;
  for (i = 0;
       source_file->buffer[source_file->pos+i] &&
       source_file->buffer[source_file->pos+i] != '\n';
       ++i)
  {
    line[i] = source_file->buffer[source_file->pos+i]; 
  }
  line[i] = 0;
  source_file->pos += i+1;
  return i;
}


/*
  WritePRG

  Output program to file in C64 PRG format.
*/
BOOL
WritePRG(struct BASIC_program* program, u16 load_address, char* path)
{
  if (!program ||
      !program->first_line)
  {
    fprintf(stderr, "ERROR: Empty program\n");
    return FALSE;
  }

  FILE* fp;
  if (path)
    fp = fopen(path, "wb");
  else
    fp = stdout;
  if (!fp)
  {
    fprintf(stderr, "ERROR: Unable to open %s for writing\n", path);
    return FALSE;
  }

#define DEFAULT_LOAD_ADDRESS 0x0801
  if (load_address == 0)
    load_address = DEFAULT_LOAD_ADDRESS;

  fwrite(&load_address, 2, 1, fp);
  u16 next_line_addr = load_address;
  struct BASIC_line* curr_line = program->first_line;
  while (curr_line)
  {
    u16 line_length = strlen((char*)curr_line->tokenized_line);
    next_line_addr += 4 + line_length + 1;
    fwrite(&next_line_addr, 2, 1, fp);
    fwrite(&curr_line->line_no, 2, 1, fp);
    /* Line data including NULL byte */
    fwrite(&curr_line->tokenized_line, 1, line_length+1, fp); 
    curr_line = curr_line->next;
  }
  /* NULL address to terminate program */
  fputc(0, fp);
  fputc(0, fp);

  return TRUE;
}


/*
  DoLinesPass

  Parse a source file and split it into BASIC lines, filling out a
  program structure.
*/
void
DoLinesPass(struct BASIC_program* program, struct source_file* source_file)
{
  /* TODO: Clean up this mess */
  memset(program, 0, sizeof(struct BASIC_program));

  char current_label[MAX_LABEL_LENGTH+1];
  memset(current_label, 0, MAX_LABEL_LENGTH+1);

  int len = 0;
  char line_buffer[MAX_SOURCE_LINE_LEN];
  int source_line_number = 0;
  while ((len = ReadLine(source_file, line_buffer)) > -1)
  {
    StripWhitespace(line_buffer);
    len = strlen(line_buffer);
    if (len < 1) continue;
    
    ++source_line_number;

    if (len == 0) continue;
    
    struct BASIC_line* line = (struct BASIC_line*)malloc(sizeof(struct BASIC_line));
    memset(line, 0, sizeof(struct BASIC_line));
    line->source_line_number = source_line_number;

    /* Check if line is label. Store label and advance to next
       non-blank line if so. */
    if (IsLabel(line_buffer))
    {
      /* Labels are case-insensitive */
      ConvertLowercaseToUppercase(line_buffer);
      /* Remove trailing colon from label */
      line_buffer[strlen(line_buffer)-1] = '\0';
      strncpy(current_label, line_buffer, MAX_LABEL_LENGTH);
      continue;
    }

    if (strlen(current_label) > 0)
    {
      /* Store label in line. Fail if duplicate. */
      if (Program_FindLineNumberByLabel(program, current_label) >= 0)
      {
        SyntaxError(-1, "Duplicate label: \"%s\"", current_label);
      }
      strncpy(line->label, current_label, MAX_LABEL_LENGTH);
    }


    char* line_ptr = line_buffer;
    StripWhitespace(line_ptr);

    /* Grab line number, then skip past digits. If no line number,
       we'll set it to a negative to be automatically generated. */
    if (!isdigit(*line_ptr))
      line->line_no = -1;
    else
      line->line_no = atoi(line_ptr);
    while (isdigit(*line_ptr)) ++line_ptr;

    StripWhitespace(line_ptr);
    strncpy(line->source_line, line_ptr, MAX_SOURCE_LINE_LEN);

    strncpy((char*)line->tokenized_line, line->source_line, MAX_SOURCE_LINE_LEN);
    ConvertLowercaseToUppercase((char*)line->tokenized_line);

    Program_AddLine(program, line);
    strncpy(current_label, "", 1);
  }
}

/*
  DoTokenizePass
  
  Tokenize all BASIC keywords and operators in each line of program.
*/
void
DoTokenizePass(struct BASIC_program* program)
{
  struct BASIC_line* line = program->first_line;
  while (line)
  {
    TokenizeLine(line->tokenized_line);

    line = line->next;
  }
}

/*
  TranslateLabels

  Replace all occurances of valid labels (in valid locations) with the
  corresponding BASIC line number.
*/
void
TranslateLabels(struct BASIC_program* program, byte_t* line)
{
  byte_t* line_ptr = line;

  while (*line_ptr)
  {
    /* If token accepts labels (GOTO, GOSUB), check for label, and
       replace with target line number if necessary */
    byte_t token = *line_ptr;
    ++line_ptr;
    if (TranslateToken("GOTO")  != token &&
        TranslateToken("GOSUB") != token)
    {
      continue;
    }

    /* First, skip whitespace, then continue looping if we don't
       have an alphabetic character (i.e. we cannot be dealing with
       a label). */
    ++line_ptr;
    char* label_start = (char*)line_ptr;
    while (isspace(*label_start)) ++label_start;
    if (!isalpha(*label_start))
    {
      ++label_start;
    }

    /* Find the longest possible string that could be a valid label */
    char* label_end = label_start;
    while (*label_end &&
           IsValidLabelChar(*label_end))
      ++label_end;
    while (label_end > label_start)
    {
      char  temp_label[MAX_LABEL_LENGTH+1];

      /* Is string too long to be a label? */
      int temp_label_len = label_end - label_start + 1;
      if (temp_label_len > MAX_LABEL_LENGTH)
      {
        --label_end;
        continue;
      }

      strncpy(temp_label, label_start, temp_label_len);
      s32 target_line_no = Program_FindLineNumberByLabel(program, temp_label);
      if (target_line_no < 0)
      {
        /* Not an existing label */
        --label_end;
        continue;
      }
      /* We've found a label. Replace the text with the corresponding
         line number */
      char line_number_string[6];
      sprintf(line_number_string, "%d", target_line_no);
      ReplaceStringWithString(label_start, temp_label, line_number_string);
      break;
    }
  }
}


/*
  DoLabelPass

  Run through program and replace label strings with the corresponding
  line numbers where necessary.
*/
void
DoLabelPass(struct BASIC_program* program)
{
  struct BASIC_line* curr_line = program->first_line;
  while (curr_line)
  {
    TranslateLabels(program, curr_line->tokenized_line);
    curr_line = curr_line->next;
  }
}

/*
  DoPETSCIIPlaceholderPass

  Replace all PETSCII placeholder strings with correct PETSCII
  code. This needs to be called before the tokenize pass so that the
  curly braces are not translated to PETSCII.
*/

void
DoPETSCIIPlaceholderPass(struct BASIC_program* program)
{
  struct BASIC_line* curr_line = program->first_line;
  while (curr_line)
  {
    TranslateASCIIToPETSCII((byte_t*)curr_line->tokenized_line);
    curr_line = curr_line->next;
  }
}


/*
  DoPETSCIIPass

  Convert each program line from ASCII to C64 PETSCII
*/
void
DoPETSCIIPass(struct BASIC_program* program)
{
  struct BASIC_line* curr_line = program->first_line;
  while (curr_line)
  {
    TranslateASCIIToPETSCII((byte_t*)curr_line->tokenized_line);
    curr_line = curr_line->next;
  }
}


/*
  ProgramCompile

  Run through the necessary passes to compile a source file into a
  tokenized, PETSCII-compatible program.
 */
void
Program_Compile(struct BASIC_program* program, struct source_file* source_file)
{
  DoLinesPass(program, source_file);
  DoPETSCIIPlaceholderPass(program);
  DoTokenizePass(program);
  DoLabelPass(program);
  DoPETSCIIPass(program);
}


/* 
   FixupOutputPath

   Fixup path if no output path was specified. Remove extension. Also
   remove directories if present so output file is in our program's
   working directory.
*/
void
FixupOutputPath(struct global_args* args)
{
  if (args->prg_path) return;

  int path_len = strlen(args->src_path);
  args->prg_path = (char*)malloc(path_len+1);
  strncpy(args->prg_path, args->src_path , path_len);
  char* path = args->prg_path;

  /* Remove leading directory in path (*nix and Windows path
     separators) */
  /* *nix */
  char* slash = strrchr(path, '/');
  if (slash &&
      slash[1])    /* In case slash is final character */
  {
    path = slash;
  }
  /* Windows */
  slash = strrchr(path, '\\');
  if (slash &&
      slash[1])    /* In case slash is final character */
  {
    path = slash;
  }

  char* dot = strrchr(path, '.');
  if (dot) *dot = '\0';

  /* Check if file extension exists in source file path so we don't
     overwrite the source file */
  if (strcmp(path, args->src_path) == 0)
  {
    fprintf(stderr, "ERROR: Attempting to overwrite source file. Please provide an output file path.\n");
    exit(-1);
  }
}


/*
  ProcessArgs

  Process command line arguments. Store relevant arguments in args.
*/
void
ProcessArgs(struct global_args* args, int argc, char* argv[])
{
  for(u8 argi = 1;
      argi < argc;
      ++argi)
  {
    char* arg = argv[argi];

    if (arg[0] != '-')
    {
      /* NOTE: This should always save the *last*
         non-option/non-option-argument (i.e. does not begin with '-'
         and is not an argument to a preceeding argument that *does*
         begin with '-') argument as the src_path of the source file
         to load. Is this really the desirable behavior? Perhaps save
         a list of non-option arguments? */
      args->src_path = arg;
      continue;
    }

    else if (strcmp(arg, "--output-file") == 0 ||
             strcmp(arg, "-o") == 0)
    {
      /* TODO: Refactor this into a function for reuse with other
         options */
      char* equals = strchr(arg, '=');
      if (!equals &&
          argi == argc)
      {
        fprintf(stderr, "Option %s requires an argument\n", arg);
        exit(-1);
      }
      if (equals)
      {
        args->prg_path = &equals[1];
      }
      else
      {
        args->prg_path = argv[argi+1];
        ++argi;
      }
    }

    else if (strcmp(arg, "--load-address") == 0 ||
             strcmp(arg, "-l") == 0)
    {
      char* equals = strchr(arg, '=');
      if (!equals &&
          argi == argc)
      {
        fprintf(stderr, "Option %s requires an argument\n", arg);
        exit(-1);
      }
      if (equals)
      {
        args->load_address = atoi(&equals[1]);
      }
      else
      {
        args->load_address = atoi(argv[argi+1]);
        ++argi;
      }
    }
  }
}



int
main(int argc, char* argv[])
{
  ProcessArgs(&args, argc, argv);
  if (!args.src_path)
  {
    fprintf(stderr, "Please provide a path to a BASIC source file\n");
    exit(-1);
  }

  struct source_file source_file;
  memset(&source_file, 0, sizeof(source_file));
  LoadSrc(&source_file, args.src_path);
  Program_Compile(&program, &source_file);
  printf("Compilation successful!\n");
  FixupOutputPath(&args);
  if (WritePRG(&program, args.load_address, args.prg_path))
    printf("Wrote PRG file to \"%s\"\n", args.prg_path);

  return 0;
}

