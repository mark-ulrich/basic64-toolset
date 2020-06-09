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


/* Offsets in to this table equal the token value - 0x80 */
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
