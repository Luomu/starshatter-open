/*  Project STARSHATTER
    John DiCamillo
    Copyright © 1997-2001. All Rights Reserved.

    SUBSYSTEM:    Parser
    FILE:         Token.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Scanner class definition
*/

#ifndef Token_h
#define Token_h

#include "Text.h"
#include "Dictionary.h"

#pragma warning( disable : 4237)
 
// +-------------------------------------------------------------------+

class Reader;
class Token;
class Scanner;

// +-------------------------------------------------------------------+

class Token
{
   friend class Scanner;

public:
   // keywords must be alphanumeric identifiers or symbolic identifiers
   enum Types { Undefined, Keyword, AlphaIdent, SymbolicIdent, Comment,
                IntLiteral, FloatLiteral, StringLiteral, CharLiteral,
                Dot, Comma, Colon, Semicolon,
                LParen, RParen, LBracket, RBracket, LBrace, RBrace,
                EOT, LastTokenType };

   enum Alias { CompoundSeparator   = Dot,
                ItemSeparator       = Comma,
                StatementTerminator = Semicolon,
                TypeIndicator       = Colon,
                Lambda              = LastTokenType + 1 };

   Token();
   Token(const Token& rhs);
   Token(int t);
   Token(const char* s, int t, int k=0, int l=0, int c=0);
   Token(const Text& s, int t, int k=0, int l=0, int c=0);
   ~Token();
   
   Token& operator = (const Token& rhs);
   
   bool     match(const Token& ref) const;

   Text     symbol()   const;
   int      type()     const { return mType;   }
   int      key()      const { return mKey;    }
   int      line()     const { return mLine;   }
   int      column()   const { return mColumn; }
   
   Text     typestr()  const;

   static Text   describe(const Text& tok);
   static void   addKey(const Text& k, int v);
   static void   addKeys(Dictionary<int>& keys);
   static bool   findKey(const Text& k, int& v);
   static void   comments(const Text& begin, const Text& end);
   static void   altComments(const Text& begin, const Text& end);
   static void   hideComments(bool hide = true) { hidecom = hide; }

   static char   comBeg(unsigned int i) { return combeg[i]; }
   static char   comEnd(unsigned int i) { return comend[i]; }
   static char   altBeg(unsigned int i) { return altbeg[i]; }
   static char   altEnd(unsigned int i) { return altend[i]; }
   
   static void   close();

protected:
   int      mLength;
   union {
      char  mSymbol[8];
      char* mFullSymbol;
   };
   int      mType;
   int      mKey;
   int      mLine;
   int      mColumn;
    
   static bool             hidecom;
   static char             combeg[3];
   static char             comend[3];
   static char             altbeg[3];
   static char             altend[3];

   static Dictionary<int> keymap;
};

// +-------------------------------------------------------------------+

class Scanner
{
public:
   Scanner(Reader* r = 0);
   Scanner(const Text&  s);
   Scanner(const Scanner& rhs);
   virtual ~Scanner();
   
   Scanner& operator = (const Scanner& rhs);

   void           Load(const Text& s);

   enum Need { Demand, Request };
   virtual Token  Get(Need n = Demand);

   void           PutBack()     { index = old_index; line = old_line; }
   int            GetCursor()   { return index;     }
   int            GetLine()     { return line;      }
   void           Reset(int c, int l) { index = old_index = c; line = old_line = l; }
   Token          Best() const  { return best;      }

protected:
   virtual int    GetNumeric();
   virtual bool   IsSymbolic(char c);
   virtual bool   IsAlpha(char c);

   Reader*     reader;
   char*       str;

   const char* p;
   const char* eos;

   size_t      index;
   size_t      old_index;
   Token       best;
   size_t      length;
   size_t      line;
   size_t      old_line;
   size_t      lineStart;
};

#endif // TOKEN_H
