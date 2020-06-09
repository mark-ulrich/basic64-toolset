/*
  prgload

  A simple utlitiy to translate a Commodore 64 PRG file into a BASIC
  source file.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>



typedef int8_t    s8;
typedef int16_t   s16;
typedef uint8_t   u8;
typedef uint16_t  u16;

typedef u8  byte_t;


#define GETWORD(buf,i) ((buf[i+1] << 8) | buf[i])


/* Translation table for keycodes between modern ASCII standard and
   C64 PETSCII */
char* PETSCII_table[] =
{
    /* 00 */  "{PETSCHII_00}",
    /* 01 */  "{PETSCHII_01}",
    /* 02 */  "{PETSCHII_02}",
    /* 03 */  "{PETSCHII_STOP}",
    /* 04 */  "{PETSCHII_04}",
    /* 05 */  "{PETSCHII_WHITE}",
    /* 06 */  "{PETSCHII_06}",
    /* 07 */  "{PETSCHII_07}",
    /* 08 */  "{PETSCHII_DISABLE_SHIFT_CMDR}",
    /* 09 */  "{PETSCHII_ENABLE_SHIFT_CMDR}",
    /* 0A */  "{PETSCHII_0A}",
    /* 0B */  "{PETSCHII_0B}",
    /* 0C */  "{PETSCHII_0C}",
    /* 0D */  "{PETSCHII_RETURN}",
    /* 0E */  "{PETSCHII_TOGGLE_CHARSET}",
    /* 0F */  "{PETSCHII_0F}",
    /* 10 */  "{PETSCHII_10}",
    /* 11 */  "{PETSCHII_CRSR_DOWN}",
    /* 12 */  "{PETSCHII_RVS_ON}",
    /* 13 */  "{PETSCHII_HOME}",
    /* 14 */  "{PETSCHII_DELETE}",
    /* 15 */  "{PETSCHII_15}",
    /* 16 */  "{PETSCHII_CRSR_UP}",
    /* 17 */  "{PETSCHII_17}",
    /* 18 */  "{PETSCHII_18}",
    /* 19 */  "{PETSCHII_19}",
    /* 1A */  "{PETSCHII_1A}",
    /* 1B */  "{PETSCHII_1B}",
    /* 1C */  "{PETSCHII_RED}",
    /* 1D */  "{PETSCHII_CRSR_RIGHT}",
    /* 1E */  "{PETSCHII_GREEN}",
    /* 1F */  "{PETSCHII_BLUE}",
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
    /* 5C */  "{PETSCHII_POUND_STERLING}",
    /* 5D */  "]",
    /* 5E */  "^", /* {PETSCII_ARROW_UP} */
    /* 5F */  "{PETSCHII_ARROW_LEFT}",
    /* 60 */  "{PETSCHII_GFX_60}",
    /* 61 */  "{PETSCHII_GFX_61}",
    /* 62 */  "{PETSCHII_GFX_62}",
    /* 63 */  "{PETSCHII_GFX_63}",
    /* 64 */  "{PETSCHII_GFX_64}",
    /* 65 */  "{PETSCHII_GFX_65}",
    /* 66 */  "{PETSCHII_GFX_66}",
    /* 67 */  "{PETSCHII_GFX_67}",
    /* 68 */  "{PETSCHII_GFX_68}",
    /* 69 */  "{PETSCHII_GFX_69}",
    /* 6A */  "{PETSCHII_GFX_6A}",
    /* 6B */  "{PETSCHII_GFX_6B}",
    /* 6C */  "{PETSCHII_GFX_6C}",
    /* 6D */  "{PETSCHII_GFX_6D}",
    /* 6E */  "{PETSCHII_GFX_6E}",
    /* 6F */  "{PETSCHII_GFX_6F}",
    /* 70 */  "{PETSCHII_GFX_70}",
    /* 71 */  "{PETSCHII_GFX_71}",
    /* 72 */  "{PETSCHII_GFX_72}",
    /* 73 */  "{PETSCHII_GFX_73}",
    /* 74 */  "{PETSCHII_GFX_74}",
    /* 75 */  "{PETSCHII_GFX_75}",
    /* 76 */  "{PETSCHII_GFX_76}",
    /* 77 */  "{PETSCHII_GFX_77}",
    /* 78 */  "{PETSCHII_GFX_78}",
    /* 79 */  "{PETSCHII_GFX_79}",
    /* 7A */  "{PETSCHII_GFX_7A}",
    /* 7B */  "{PETSCHII_GFX_7B}",
    /* 7C */  "{PETSCHII_GFX_7C}",
    /* 7D */  "{PETSCHII_GFX_7D}",
    /* 7E */  "{PETSCHII_GFX_7E}",
    /* 7F */  "{PETSCHII_GFX_7F}",
    /* 80 */  "{PETSCHII_GFX_80}",
    /* 81 */  "{PETSCHII_GFX_81}",
    /* 82 */  "{PETSCHII_GFX_82}",
    /* 83 */  "{PETSCHII_GFX_83}",
    /* 84 */  "{PETSCHII_GFX_84}",
    /* 85 */  "{PETSCHII_GFX_85}",
    /* 86 */  "{PETSCHII_GFX_86}",
    /* 87 */  "{PETSCHII_GFX_87}",
    /* 88 */  "{PETSCHII_GFX_88}",
    /* 89 */  "{PETSCHII_GFX_89}",
    /* 8A */  "{PETSCHII_GFX_8A}",
    /* 8B */  "{PETSCHII_GFX_8B}",
    /* 8C */  "{PETSCHII_GFX_8C}",
    /* 8D */  "{PETSCHII_GFX_8D}",
    /* 8E */  "{PETSCHII_GFX_8E}",
    /* 8F */  "{PETSCHII_GFX_8F}",
    /* 90 */  "{PETSCHII_GFX_90}",
    /* 91 */  "{PETSCHII_GFX_91}",
    /* 92 */  "{PETSCHII_GFX_92}",
    /* 93 */  "{PETSCHII_GFX_93}",
    /* 94 */  "{PETSCHII_GFX_94}",
    /* 95 */  "{PETSCHII_GFX_95}",
    /* 96 */  "{PETSCHII_GFX_96}",
    /* 97 */  "{PETSCHII_GFX_97}",
    /* 98 */  "{PETSCHII_GFX_98}",
    /* 99 */  "{PETSCHII_GFX_99}",
    /* 9A */  "{PETSCHII_GFX_9A}",
    /* 9B */  "{PETSCHII_GFX_9B}",
    /* 9C */  "{PETSCHII_GFX_9C}",
    /* 9D */  "{PETSCHII_GFX_9D}",
    /* 9E */  "{PETSCHII_GFX_9E}",
    /* 9F */  "{PETSCHII_GFX_9F}",
    /* A0 */  "{PETSCHII_GFX_F0}",
    /* A1 */  "{PETSCHII_GFX_F1}",
    /* A2 */  "{PETSCHII_GFX_F2}",
    /* A3 */  "{PETSCHII_GFX_F3}",
    /* A4 */  "{PETSCHII_GFX_F4}",
    /* A5 */  "{PETSCHII_GFX_F5}",
    /* A6 */  "{PETSCHII_GFX_F6}",
    /* A7 */  "{PETSCHII_GFX_F7}",
    /* A8 */  "{PETSCHII_GFX_F8}",
    /* A9 */  "{PETSCHII_GFX_F9}",
    /* AA */  "{PETSCHII_GFX_FA}",
    /* AB */  "{PETSCHII_GFX_FB}",
    /* AC */  "{PETSCHII_GFX_FC}",
    /* AD */  "{PETSCHII_GFX_FD}",
    /* AE */  "{PETSCHII_GFX_FE}",
    /* AF */  "{PETSCHII_GFX_FF}",
    /* B0 */  "{PETSCHII_GFX_B0}",
    /* B1 */  "{PETSCHII_GFX_B1}",
    /* B2 */  "{PETSCHII_GFX_B2}",
    /* B3 */  "{PETSCHII_GFX_B3}",
    /* B4 */  "{PETSCHII_GFX_B4}",
    /* B5 */  "{PETSCHII_GFX_B5}",
    /* B6 */  "{PETSCHII_GFX_B6}",
    /* B7 */  "{PETSCHII_GFX_B7}",
    /* B8 */  "{PETSCHII_GFX_B8}",
    /* B9 */  "{PETSCHII_GFX_B9}",
    /* BA */  "{PETSCHII_GFX_BA}",
    /* BB */  "{PETSCHII_GFX_BB}",
    /* BC */  "{PETSCHII_GFX_BC}",
    /* BD */  "{PETSCHII_GFX_BD}",
    /* BE */  "{PETSCHII_GFX_BE}",
    /* BF */  "{PETSCHII_GFX_BF}",
    /* C0 */  "{PETSCHII_GFX_00}",
    /* C1 */  "{PETSCHII_GFX_01}",
    /* C2 */  "{PETSCHII_GFX_02}",
    /* C3 */  "{PETSCHII_GFX_03}",
    /* C4 */  "{PETSCHII_GFX_04}",
    /* C5 */  "{PETSCHII_GFX_05}",
    /* C6 */  "{PETSCHII_GFX_06}",
    /* C7 */  "{PETSCHII_GFX_07}",
    /* C8 */  "{PETSCHII_GFX_08}",
    /* C9 */  "{PETSCHII_GFX_09}",
    /* CA */  "{PETSCHII_GFX_0A}",
    /* CB */  "{PETSCHII_GFX_0B}",
    /* CC */  "{PETSCHII_GFX_0C}",
    /* CD */  "{PETSCHII_GFX_0D}",
    /* CE */  "{PETSCHII_GFX_0E}",
    /* CF */  "{PETSCHII_GFX_0F}",
    /* D0 */  "{PETSCHII_GFX_00}",
    /* D1 */  "{PETSCHII_GFX_01}",
    /* D2 */  "{PETSCHII_GFX_02}",
    /* D3 */  "{PETSCHII_GFX_03}",
    /* D4 */  "{PETSCHII_GFX_04}",
    /* D5 */  "{PETSCHII_GFX_05}",
    /* D6 */  "{PETSCHII_GFX_06}",
    /* D7 */  "{PETSCHII_GFX_07}",
    /* D8 */  "{PETSCHII_GFX_08}",
    /* D9 */  "{PETSCHII_GFX_09}",
    /* DA */  "{PETSCHII_GFX_0A}",
    /* DB */  "{PETSCHII_GFX_0B}",
    /* DC */  "{PETSCHII_GFX_0C}",
    /* DD */  "{PETSCHII_GFX_0D}",
    /* DE */  "{PETSCHII_GFX_0E}",
    /* DF */  "{PETSCHII_GFX_0F}",
    /* E0 */  "{PETSCHII_GFX_00}",
    /* E1 */  "{PETSCHII_GFX_01}",
    /* E2 */  "{PETSCHII_GFX_02}",
    /* E3 */  "{PETSCHII_GFX_03}",
    /* E4 */  "{PETSCHII_GFX_04}",
    /* E5 */  "{PETSCHII_GFX_05}",
    /* E6 */  "{PETSCHII_GFX_06}",
    /* E7 */  "{PETSCHII_GFX_07}",
    /* E8 */  "{PETSCHII_GFX_08}",
    /* E9 */  "{PETSCHII_GFX_09}",
    /* EA */  "{PETSCHII_GFX_0A}",
    /* EB */  "{PETSCHII_GFX_0B}",
    /* EC */  "{PETSCHII_GFX_0C}",
    /* ED */  "{PETSCHII_GFX_0D}",
    /* EE */  "{PETSCHII_GFX_0E}",
    /* EF */  "{PETSCHII_GFX_0F}",
    /* F0 */  "{PETSCHII_GFX_00}",
    /* F1 */  "{PETSCHII_GFX_01}",
    /* F2 */  "{PETSCHII_GFX_02}",
    /* F3 */  "{PETSCHII_GFX_03}",
    /* F4 */  "{PETSCHII_GFX_04}",
    /* F5 */  "{PETSCHII_GFX_05}",
    /* F6 */  "{PETSCHII_GFX_06}",
    /* F7 */  "{PETSCHII_GFX_07}",
    /* F8 */  "{PETSCHII_GFX_08}",
    /* F9 */  "{PETSCHII_GFX_09}",
    /* FA */  "{PETSCHII_GFX_0A}",
    /* FB */  "{PETSCHII_GFX_0B}",
    /* FC */  "{PETSCHII_GFX_0C}",
    /* FD */  "{PETSCHII_GFX_0D}",
    /* FE */  "{PETSCHII_GFX_0E}",
    /* FF */  "{PETSCHII_GFX_0F}",
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
   lines). Add one byte for NULL terminator */
#define MAX_DATA_LEN  81
struct basic_line
{
  u16    line_no;
  byte_t data[MAX_DATA_LEN];
};

/*
  TranslateToken

  Translate a token into the corresponding BASIC keyword.
*/
char*
TranslateToken(byte_t token)
{
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
  DecodeLine

  Replace all BASIC tokens in line with the corresponding BASIC
  keyword/operator.
*/
void
DecodeLine(byte_t* line)
{
  for (u8 i = 0;
       i < strlen((char*)line);
       ++i)
  {
    if (line[i] < 0x80) continue;
    char* keyword = TranslateToken(line[i]);
    if (!keyword)
    {
      continue;
    }
    //  printf(": %s\n", keyword);
    MemInsert(line, (byte_t*)keyword, i, strlen(keyword));
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
      /* NOTE: This will always save the *last* non-option (i.e. does
         not begin with '-') argument as the path of the PRG file to
         load. Is this really the desirable behavior? */
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

    /* printf("Current line offset: %u\n", line_offset); */

    u16 next_line_offset = GETWORD(buffer, line_offset);
    if (!next_line_offset) break;

    /* grab line from buffer up to NULL terminator */
    line.line_no = GETWORD(buffer, line_offset+2);
    for (int i = 0;
         buffer[line_offset+i+4];
         ++i)
    {
      line.data[i] = buffer[line_offset+i+4];
    }

    DecodeLine(line.data);
    printf("%u %s\n", line.line_no, line.data);

    /* Compute next line offset into buffer. The +2 accounts for first
       2 bytes of buffer (program load address) */
    line_offset = next_line_offset - load_address + 2; 
    /* printf("Next line offset: %u\n", line_offset); */
  }

  return 0;
}
