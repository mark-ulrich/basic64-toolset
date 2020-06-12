/*
  prgdc

  A simple Commodore 64 PRG file decompiler.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>



typedef uint8_t   u8;
typedef uint16_t  u16;

typedef u8  byte_t;


#define GETWORD(buf,i) ((buf[i+1] << 8) | buf[i])


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
    /* 11 */  "{PETSCII_CRSR_DOWN}",
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
    /* 1D */  "{PETSCII_CRSR_RIGHT}",
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
    /* 5C */  "{PETSCII_POUND_STERLING}",
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
    /* 91 */  "{PETSCII_CRSR_UP}",
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
    /* 9D */  "{PETSCII_CRSR_LEFT}",
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
#define MAX_DATA_LINE_LEN  2048
struct basic_line
{
  u16    line_no;
  char   data[MAX_DATA_LINE_LEN];
};

/*
  {PETSCII_

  Translate a token into the corresponding BASIC keyword.
*/
char*
TranslateToken(byte_t token)
{
  if (token < 0x80 ||
      token > 0xcb)
    return 0;
  return token_list[token - 0x80];
}

/*
  MemInsert
  
  Insert len bytes from src into byte buffer dst at offset, replacing
  the single byte at offset, and all bytes after offset by len to
  accomodate the insertion of src.
  
  NOTE: This function *DOES NOT* perform any bounds checking. Use with
  caution.

*/
void
MemInsert(byte_t* dest, byte_t* src, int offset, u16 len)
{
  int line_end = 0;
  while (dest[line_end++]);
  for (int i = line_end;
       i >= offset;
       --i)
  {
    dest[i+len] = dest[i+1];
  }
  for (int i = 0;
       i < len;
       ++i)
  {
    dest[offset+i] = src[i];
  }
}

/*
  TranslatePETSCIIToASCII
  
  Translate a string from PETSCII encoding to ASCII, inserting
  placeholder strings where necessary.
*/
void
TranslatePETSCIIToASCII(char* line)
{
  for (u16 i = 0;
       i < strlen(line);
       ++i)
  {
    /* TODO: This is inefficient. Bytes in which PETSCII and ASCII are
       equivalent should be checked for and skipped */
    byte_t byte = (byte_t)line[i];
    char* ascii = PETSCII_table[byte];
    u16 ascii_len = strlen(ascii);
    MemInsert((byte_t*)line, (byte_t*)ascii, i, ascii_len);
    i += ascii_len - 1;
  }
}

/*
  DecodeLine

  Replace all BASIC tokens in line with the corresponding BASIC
  keyword/operator.
*/
void
DecodeLine(char* line)
{
  for (u8 i = 0;
       i < strlen(line);
       ++i)
  {
    /* Don't decode tokens in quotes */
    if (line[i] == '"')
    {
      while (line[++i] != '"');
      ++i;
    }

    byte_t byte = (unsigned char)line[i];
    if (byte < 0x80) continue;
    char* keyword = TranslateToken(byte);
    if (!keyword)
    {
      continue;
    }
    //  printf(": %s\n", keyword);
    MemInsert((byte_t*)line, (byte_t*)keyword, i, strlen(keyword));
  }
}

/*
  LoadPRGFile

  Load a PRG file located at path into a buffer.

  Return: Pointer to byte buffer
*/
byte_t*
LoadPRGFile(char* path)
{
  if (!path)
  {
    fprintf(stderr, "Please provide a path to a PRG file\n");
    exit(-1);
  }

  FILE* fp = fopen(path, "rb");
  if (!fp)
  {
    fprintf(stderr, "Failed to open file %s\n", path);
    exit(-1);
  }
  struct stat fs;
  fstat(fileno(fp), &fs);
  byte_t* buffer = (byte_t*)malloc(fs.st_size);
  fread(buffer, 1, fs.st_size, fp);
  fclose(fp);

  return buffer;
}


int
main(int argc, char* argv[])
{
  char* path = 0;
  /* allows for future expansion with command line args */
  for(u8 argi = 1;
      argi < argc;
      ++argi)
  {
    if (argv[argi][0] != '-')
    {
      /* NOTE: This should always save the *last* non-option
         (i.e. does not begin with '-') argument as the path of the
         PRG file to load. Is this really the desirable behavior? */
      path = argv[argi];
      continue;
    }
  }

  byte_t* buffer = LoadPRGFile(path);

  u16 load_address = GETWORD(buffer, 0);
  u16 line_offset = 2;
  while (line_offset)
  {
    struct basic_line line;
    memset(&line, 0, sizeof(struct basic_line));

    u16 next_line_offset = GETWORD(buffer, line_offset);
    if (!next_line_offset) break;

    /* grab line from buffer up to NULL terminator */
    line.line_no = GETWORD(buffer, line_offset+2);
    strncpy(line.data, (char*)&buffer[line_offset+4], MAX_DATA_LINE_LEN);

    DecodeLine(line.data);
    TranslatePETSCIIToASCII(line.data);
    printf("%u %s\n", line.line_no, line.data);

    /* Compute next line offset into buffer. The +2 accounts for first
       2 bytes of buffer (program load address) */
    line_offset = next_line_offset - load_address + 2; 
  }

  return 0;
}
