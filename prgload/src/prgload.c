/*
  prgload

  A utlitiy to translate a Commodore 64 PRG file into a
  BASIC source file.
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

char* mnemonics[] =
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


#define MAX_DATA_LEN  80
struct basic_line
{
  u16    next_line_offset;
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
  return mnemonics[token - 0x80];
}

void
MemInsert(byte_t* dest, byte_t* src, int offset, u16 len)
{
  int i;
  i = 0;
  /* printf("%u\n", offset); */
  while (dest[i++]);
  /* printf("%u\n", i); */
  for (i;
       i >= offset;
       --i)
  {
    /* printf("%u\n", i); */
    dest[i+len] = dest[i+1];
  }
  for (i = 0;
       i < len;
       ++i)
  {
    dest[offset+i] = src[i];
  }
  /* printf("%s\n", (char*)dest); */
}

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


int
main(int argc, char* argv[])
{
  char* path;
  byte_t* buffer;
  FILE* fp;
  struct stat fs;

  path = 0;
  for(u8 argi = 1;
      argi < argc;
      ++argi)
  {
    if (argv[argi][0] != '-')
    {
      path = argv[argi];
      continue;
    }
  }

  if (!path)
  {
    fprintf(stderr, "Please provide a path to a PRG file\n");
    exit(-1);
  }

  fp = fopen(path, "rb");
  if (!fp)
  {
    fprintf(stderr, "Failed to open file %s\n", path);
    exit(-1);
  }
  fstat(fileno(fp), &fs);
  buffer = (byte_t*)malloc(fs.st_size);
  fread(buffer, 1, fs.st_size, fp);
  fclose(fp);

  u16 load_address = (buffer[1] << 8) |  buffer[0];
  /* printf("Load address: 0x%04x\n", load_address); */
  int i = 2;                /* skip load address */
  while (i < fs.st_size)
  {
    struct basic_line line;
    
    memset(&line, 0, sizeof(struct basic_line));
    /* printf("i: 0x%04x\n", i); */
    line.next_line_offset = ((buffer[i+1] << 8) | buffer[i]);
    if (line.next_line_offset == 0)
      break;
    line.next_line_offset -= load_address;
    i+=2;
    line.line_no = (buffer[i+1] << 8) | buffer[i];
    i+=2;
    strncpy((char*)line.data, (char*)&buffer[i], MAX_DATA_LEN);
    i += strlen((char*)line.data)+1;
    /* printf("%s\n", (char*)line.data); */
    DecodeLine(line.data);
    printf("%u %s\n", line.line_no, line.data);
  }

  return 0;
}
