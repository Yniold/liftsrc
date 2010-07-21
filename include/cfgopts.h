/*
**  CFGOPTS.H
*/

#ifndef Boolean_T_defined
#define Boolean_T_defined
#undef ERROR
#undef FALSE
#undef TRUE

typedef enum {ERROR = -1,FALSE, TRUE} Boolean_T;
#endif

#ifndef TAG_TYPE_defined
#define TAG_TYPE_defined
typedef enum {
      Error_Tag,
      Byte_Tag,
      Boolean_Tag,
      Word_Tag,
      DWord_Tag,
      OctWord_Tag,
      DOctWord_Tag,
      HexWord_Tag,
      DHexWord_Tag,
      Float_Tag,
      Double_Tag,
      String_Tag,
      Function_Tag
      } TAG_TYPE;
#endif

struct Config_Tag {
      char        *code;                /* Option switch        */
      TAG_TYPE    type;                 /* Type of option       */
      void        *buf;                 /* Storage location     */
};

int input_config(char *, struct Config_Tag *, char *);
int update_config(char *, struct Config_Tag *, char *);
