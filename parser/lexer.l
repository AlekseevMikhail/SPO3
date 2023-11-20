%option noyywrap noinput nounput

%{
#include <string.h>

#include "parser.h"

void update_yylloc(void) {
    size_t i;

    yylloc.first_line = yylloc.last_line;
    yylloc.first_column = yylloc.last_column;

    for (i = 0; i < yyleng; ++i) {
        switch (yytext[i]) {
        case '\n':
            ++yylloc.last_line;
            yylloc.last_column = 1;
            break;

        default:
            ++yylloc.last_column;
        }
    }
}
%}

S [ \b\n\t\f\r]
W [a-zA-Z_]
D [0-9]
X [0-9a-fA-F]

%%

{S}     update_yylloc();
\/\/.*$ update_yylloc();

\"(\\.|[^"\\])*\"   update_yylloc(); yylval.token = strdup(yytext); return T_STR;
'[^']'              update_yylloc(); yylval.token = strdup(yytext); return T_CHAR;
0[xX]{X}+           update_yylloc(); yylval.token = strdup(yytext); return T_HEX;
0[bB][0-1]+         update_yylloc(); yylval.token = strdup(yytext); return T_BITS;
{D}+                update_yylloc(); yylval.token = strdup(yytext); return T_DEC;
true|false          update_yylloc(); yylval.token = strdup(yytext); return T_BOOL;

bool    update_yylloc(); return TL_BOOL;
byte    update_yylloc(); return TL_BYTE;
int     update_yylloc(); return TL_INT;
uint    update_yylloc(); return TL_UINT;
long    update_yylloc(); return TL_LONG;
ulong   update_yylloc(); return TL_ULONG;
char    update_yylloc(); return TL_CHAR;
string  update_yylloc(); return TL_STRING;
if      update_yylloc(); return TL_IF;
else    update_yylloc(); return TL_ELSE;
while   update_yylloc(); return TL_WHILE;
do      update_yylloc(); return TL_DO;
break   update_yylloc(); return TL_BREAK;
&&      update_yylloc(); return TL_AND;
"||"    update_yylloc(); return TL_OR;
==      update_yylloc(); return TL_EQ;
!=      update_yylloc(); return TL_NE;
"<="    update_yylloc(); return TL_LE;
">="    update_yylloc(); return TL_GE;
"<<"    update_yylloc(); return TL_LBS;
">>"    update_yylloc(); return TL_RBS;
"+="    update_yylloc(); return TL_PLUS_EQ;
-=      update_yylloc(); return TL_MINUS_EQ;
"*="    update_yylloc(); return TL_MULTIPLY_EQ;
"/="    update_yylloc(); return TL_DIVIDE_EQ;
%=      update_yylloc(); return TL_REMAINDER_EQ;
&=      update_yylloc(); return TL_BAND_EQ;
"|="    update_yylloc(); return TL_BOR_EQ;
^=      update_yylloc(); return TL_BXOR_EQ;
&&=     update_yylloc(); return TL_AND_EQ;
"||="   update_yylloc(); return TL_OR_EQ;
"++"    update_yylloc(); return TL_INC;
"--"    update_yylloc(); return TL_DEC;

{W}({W}|{D})*   update_yylloc(); yylval.token = strdup(yytext); return T_IDENTIFIER;

.       update_yylloc(); return yytext[0];
