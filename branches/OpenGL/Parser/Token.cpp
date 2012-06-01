/*  Project STARS
    John DiCamillo Software Consulting
    Copyright © 1997-2000. All Rights Reserved.

    SUBSYSTEM:    Stars
    FILE:         token.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Scanner class implementation
*/

#include "MemDebug.h"
#include "Token.h"
#include "Reader.h"
#include "Text.h"

#include <ctype.h>

// +-------------------------------------------------------------------+

bool        Token::hidecom   = true;
char        Token::combeg[3] = "//";
char        Token::comend[3] = "\n";
char        Token::altbeg[3] = "/*";
char        Token::altend[3] = "*/";
Dictionary<int>   Token::keymap;

// +-------------------------------------------------------------------+

Token::Token()
   : mType(Undefined), mKey(0), mLine(0), mColumn(0)
{
   mLength    = 0;
   mSymbol[0] = '\0';
}

Token::Token(const Token& rhs)
   : mType(rhs.mType), mKey(rhs.mKey), mLine(rhs.mLine), mColumn(rhs.mColumn)
{
   mLength = rhs.mLength;
   if (mLength < 8) {
      strcpy_s(mSymbol, rhs.mSymbol);
   }
   else {
      mFullSymbol = new(__FILE__, __LINE__) char[mLength + 1];
      strcpy(mFullSymbol, rhs.mFullSymbol);
   }
}

Token::Token(int t)
   : mType(t), mKey(0), mLine(0), mColumn(0)
{
   mLength    = 0;
   mSymbol[0] = '\0';
}

Token::Token(const char* s, int t, int k, int l, int c)
   : mType(t), mKey(k), mLine(l), mColumn(c)
{
   mLength = strlen(s);
   if (mLength < 8) {
      strcpy_s(mSymbol, s);
   }
   else {
      mFullSymbol = new(__FILE__, __LINE__) char[mLength + 1];
      strcpy(mFullSymbol, s);
   }
}

Token::Token(const Text& s, int t, int k, int l, int c)
   : mType(t), mKey(k), mLine(l), mColumn(c)
{
   mLength = s.length();
   if (mLength < 8) {
      strcpy_s(mSymbol, s.data());
   }
   else {
      mFullSymbol = new(__FILE__, __LINE__) char[mLength + 1];
      strcpy(mFullSymbol, s.data());
   }
}

Token::~Token()
{
   if (mLength >= 8)
      delete [] mFullSymbol;
}

// +-------------------------------------------------------------------+

void
Token::close()
{
   keymap.clear();
}

// +-------------------------------------------------------------------+

Token&
Token::operator = (const Token& rhs)
{
   if (mLength >= 8)
      delete [] mFullSymbol;

   mLength = rhs.mLength;
   if (mLength < 8) {
      strcpy_s(mSymbol, rhs.mSymbol);
   }
   else {
      mFullSymbol = new(__FILE__, __LINE__) char[mLength + 1];
      strcpy(mFullSymbol, rhs.mFullSymbol);
   }
   
   mType   = rhs.mType;
   mKey    = rhs.mKey;
   mLine   = rhs.mLine;
   mColumn = rhs.mColumn;
   
   return *this;
}

// +-------------------------------------------------------------------+

bool
Token::match(const Token& ref) const
{
   if (mType == ref.mType) {                    // if types match 
      if (ref.mLength == 0)                     // if no symbol to match
         return true;                           // match!

      else if (mLength == ref.mLength) {        // else if symbols match
         if (mLength < 8) {
            if (!strcmp(mSymbol, ref.mSymbol))
               return true;                     // match!
         }
         else {
            if (!strcmp(mFullSymbol, ref.mFullSymbol))
               return true;                     // match!
         }
      }
   }
   
   return false;
}

// +-------------------------------------------------------------------+

Text
Token::symbol() const
{
   if (mLength < 8)
      return Text(mSymbol);
   else
      return Text(mFullSymbol);
}

// +-------------------------------------------------------------------+

void
Token::addKey(const Text& k, int v)
{
   keymap.insert(k, v);
}

// +-------------------------------------------------------------------+

void
Token::addKeys(Dictionary<int>& keys)
{
   DictionaryIter<int> iter = keys;
   while (++iter)
      keymap.insert(iter.key(), iter.value());
}

// +-------------------------------------------------------------------+

bool
Token::findKey(const Text& k, int& v)
{
   if (keymap.contains(k)) {
      v = keymap.find(k, 0);
      return true;
   }
   else
      return false;
}

// +-------------------------------------------------------------------+

void
Token::comments(const Text& begin, const Text& end)
{
   combeg[0] = begin(0);
   if (begin.length() > 1) combeg[1] = begin(1);
   else                    combeg[1] = '\0';   
      
   comend[0] = end(0);
   if (end.length() > 1)   comend[1] = end(1);
   else                    comend[1] = '\0';   
}     

// +-------------------------------------------------------------------+

void
Token::altComments(const Text& begin, const Text& end)
{
   altbeg[0] = begin(0);
   if (begin.length() > 1) altbeg[1] = begin(1);
   else                    altbeg[1] = '\0';   
      
   altend[0] = end(0);
   if (end.length() > 1)   altend[1] = end(1);
   else                    altend[1] = '\0';   
}     

// +-------------------------------------------------------------------+

Text
Token::typestr() const
{
   Text t = "Unknown";
   switch (type()) {
   case Undefined:      t = "Undefined"; break;
   case Keyword:        t = "Keyword"; break;
   case AlphaIdent:     t = "AlphaIdent"; break;
   case SymbolicIdent:  t = "SymbolicIdent"; break;
   case Comment:        t = "Comment"; break;
   case IntLiteral:     t = "IntLiteral"; break;
   case FloatLiteral:   t = "FloatLiteral"; break;
   case StringLiteral:  t = "StringLiteral"; break;
   case CharLiteral:    t = "CharLiteral"; break;
   case Dot:            t = "Dot"; break;
   case Comma:          t = "Comma"; break;
   case Colon:          t = "Colon"; break;
   case Semicolon:      t = "Semicolon"; break;
   case LParen:         t = "LParen"; break;
   case RParen:         t = "RParen"; break;
   case LBracket:       t = "LBracket"; break;
   case RBracket:       t = "RBracket"; break;
   case LBrace:         t = "LBrace"; break;
   case RBrace:         t = "RBrace"; break;
   case EOT:            t = "EOT"; break;
   case LastTokenType:  t = "LastTokenType"; break;
   }
   
   return t;
}

// +-------------------------------------------------------------------+

Text
Token::describe(const Text& tok)
{
   Text d;

   switch (tok(0)) {
   case '.' : d = "Token::Dot"; break;
   case ',' : d = "Token::Comma"; break;
   case ';' : d = "Token::Semicolon"; break;
   case '(' : d = "Token::LParen"; break;
   case ')' : d = "Token::RParen"; break;
   case '[' : d = "Token::LBracket"; break;
   case ']' : d = "Token::RBracket"; break;
   case '{' : d = "Token::LBrace"; break;
   case '}' : d = "Token::RBrace"; break;
   default  :               break;
   }
   
   if (d.length() == 0) {
      if (isalpha(tok(0)))
         d = "\"" + tok + "\", Token::AlphaIdent";
      else if (isdigit(tok(0))) {
         if (tok.contains("."))
            d = "\"" + tok + "\", Token::FloatLiteral";
         else
            d = "\"" + tok + "\", Token::IntLiteral";
      }
      else
         d = "\"" + tok + "\", Token::SymbolicIdent";
   }
   
   return d;
}

// +-------------------------------------------------------------------+

Scanner::Scanner(Reader* r)
   : reader(r), str(0), index(0), old_index(0),
     length(0), line(0), old_line(0), lineStart(0)
{ }

Scanner::Scanner(const Scanner& rhs)
   : index(rhs.index), old_index(rhs.old_index), length(rhs.length),
     reader(rhs.reader),
     line(rhs.line), old_line(0), lineStart(rhs.lineStart)
{
   str = new(__FILE__, __LINE__) char [strlen(rhs.str) + 1];
   strcpy(str, rhs.str);
}

Scanner::Scanner(const Text& s)
   : reader(0), index(0), old_index(0), length(s.length()), line(0),
     old_line(0), lineStart(0)
{
   str = new(__FILE__, __LINE__) char [s.length() + 1];
   strcpy(str, s.data());
}

Scanner::~Scanner()
{
   delete [] str;
}

// +-------------------------------------------------------------------+

Scanner&
Scanner::operator = (const Scanner& rhs)
{
   delete [] str;
   str = new(__FILE__, __LINE__) char [strlen(rhs.str) + 1];
   strcpy(str, rhs.str);
   
   index     = rhs.index;
   old_index = rhs.old_index;
   length    = rhs.length;
   line      = rhs.line;
   old_line  = rhs.old_line;
   lineStart = rhs.lineStart;
   
   return *this;
}

// +-------------------------------------------------------------------+

void
Scanner::Load(const Text& s)
{
   delete [] str;
   str = new(__FILE__, __LINE__) char [s.length() + 1];
   strcpy(str, s.data());
   
   index       = 0;
   old_index   = 0;
   best        = Token();
   length      = s.length();
   line        = 0;
   old_line    = 0;
   lineStart   = 0;
}

// +-------------------------------------------------------------------+

Token
Scanner::Get(Need need)
{
   int   type = Token::EOT;
   old_index  = index;
   old_line   = line;

   eos = str + length;
   p   = str + index;

   if (p >= eos) {
      if (need == Demand && reader) {
         Load(reader->more());
         if (length > 0)
            return Get(need);
      }
      return Token("", type, 0, line, 0);
   }

   while (isspace(*p) && p < eos) { // skip initial white space
      if (*p == '\n') {
         line++;
         lineStart = p - str;
      }
      p++;
   }
   
   if (p >= eos) {
      if (need == Demand && reader) {
         Load(reader->more());
         if (length > 0)
            return Get(need);
      }
      return Token("", type, 0, line, 0);
   }

   Token  result;
   size_t start = p - str;

   if (*p == '"' || *p == '\'') {   // special case for quoted tokens

      if (*p == '"') type = Token::StringLiteral;
      else           type = Token::CharLiteral;

      char match = *p;
      while (++p < eos) {
         if (*p == match) {         // find matching quote
            if (*(p-1) != '\\') {   // if not escaped
               p++;                 // token includes matching quote
               break;
            }
         }
      }
   }
   
   // generic delimited comments
   else if (*p == Token::comBeg(0) &&
                (!Token::comBeg(1) || *(p+1) == Token::comBeg(1))) {
      type = Token::Comment;
      while (++p < eos) {
         if (*p == Token::comEnd(0) &&
                 (!Token::comEnd(1) || *(p+1) == Token::comEnd(1))) {
            p++; if (Token::comEnd(1)) p++;
            break;
         }
      }
   }

   // alternate form delimited comments
   else if (*p == Token::altBeg(0) &&
                (!Token::altBeg(1) || *(p+1) == Token::altBeg(1))) {
      type = Token::Comment;
      while (++p < eos) {
         if (*p == Token::altEnd(0) &&
                 (!Token::altEnd(1) || *(p+1) == Token::altEnd(1))) {
            p++; if (Token::altEnd(1)) p++;
            break;
         }
      }
   }

   else if (*p == '.')  type = Token::Dot;
   else if (*p == ',')  type = Token::Comma;
   else if (*p == ';')  type = Token::Semicolon;
   else if (*p == '(')  type = Token::LParen;
   else if (*p == ')')  type = Token::RParen;
   else if (*p == '[')  type = Token::LBracket;
   else if (*p == ']')  type = Token::RBracket;
   else if (*p == '{')  type = Token::LBrace;
   else if (*p == '}')  type = Token::RBrace;

   // use lexical sub-parser for ints and floats
   else if (isdigit(*p))
      type = GetNumeric();
   
   else if (IsSymbolic(*p)) {
      type = Token::SymbolicIdent;
      while (IsSymbolic(*p)) p++;
   }
   
   else {
      type = Token::AlphaIdent;
      while (IsAlpha(*p)) p++;
   }

   size_t extent = (p - str) - start;

   if (extent < 1) extent = 1;      // always get at least one character

   index  = start + extent;         // advance the cursor
   int col = start - lineStart;
   if (line == 0) col++;
   
   char* buf = new(__FILE__, __LINE__) char [extent + 1];
   strncpy(buf, str + start, extent);
   buf[extent] = '\0';

   if (type == Token::Comment && Token::hidecom) {
      delete [] buf;
      if (Token::comEnd(0) == '\n') {
         line++;
         lineStart = p - str;
      }
      return Get(need);
   }

   if (type == Token::AlphaIdent || // check for keyword
       type == Token::SymbolicIdent) {
       int val;
       if (Token::findKey(Text(buf), val))
         result = Token(buf, Token::Keyword, val, line+1, col);
   }

   if (result.mType != Token::Keyword)
      result = Token(buf, type, 0, line+1, col);
   
   if (line+1 >  (size_t) best.mLine ||
      (line+1 == (size_t) best.mLine && col > best.mColumn))
      best = result;

   delete [] buf;
   return result;
}

// +-------------------------------------------------------------------+

int
Scanner::GetNumeric()
{
   int type = Token::IntLiteral;             // assume int

   if (*p == '0' && *(p+1) == 'x') {         // check for hex:
      p += 2;
      while (isxdigit(*p)) p++;
      return type;
   }

   while (isdigit(*p) || *p == '_') p++;     // whole number part
   
   if (*p == '.') { p++;                     // optional fract part
      type = Token::FloatLiteral;            // implies float

      while (isdigit(*p) || *p == '_') p++;  // fractional part
   }

   if (*p == 'E' || *p == 'e') {  p++;       // optional exponent
      if (*p == '+' || *p == '-') p++;       // which may be signed
      while (isdigit(*p)) p++;

      type = Token::FloatLiteral;            // implies float
   }

   return type;
}

// +-------------------------------------------------------------------+

bool
Scanner::IsAlpha(char c)
{
   return (isalpha(*p) || isdigit(*p) || (*p == '_'))?true:false;
}

// +-------------------------------------------------------------------+

bool
Scanner::IsSymbolic(char c)
{
   const char* s = "+-*/\\<=>~!@#$%^&|:";
   return strchr(s, c)?true:false;
}
