/*  Starshatter OpenSource Distribution
     Copyright (c) 1997-2004, Destroyer Studios LLC.
     All Rights Reserved.

     Redistribution and use in source and binary forms, with or without
     modification, are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.
     * Neither the name "Destroyer Studios" nor the names of its contributors
        may be used to endorse or promote products derived from this software
        without specific prior written permission.

     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
     AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
     ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
     LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
     CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
     SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
     INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
     ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
     POSSIBILITY OF SUCH DAMAGE.

     SUBSYSTEM:    Parser
     FILE:         Parser.cpp
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     Implementation of the generic Parser class
*/

#include "MemDebug.h"
#include "reader.h"
#include "token.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

enum KEYS { KEY_TRUE, KEY_FALSE, KEY_DEF, KEY_MINUS };

void Print(const char* fmt, ...);

static int dump_tokens = 0;

// +-------------------------------------------------------------------+

Term* error(char* msg, const Token& token)
{
    static char buf[1024];
    sprintf_s(buf, " near '%s' in line %d.", (const char*) token.symbol(), token.line());

    return error(msg, buf);
}

// +-------------------------------------------------------------------+

Parser::Parser(Reader* r)
{
    reader = r ? r : new(__FILE__, __LINE__) ConsoleReader;
    lexer  = new(__FILE__, __LINE__) Scanner(reader);

    Token::addKey("true",   KEY_TRUE);
    Token::addKey("false",  KEY_FALSE);
    Token::addKey(":",      KEY_DEF);
    Token::addKey("-",      KEY_MINUS);
}

Parser::~Parser()
{
    delete lexer;
    delete reader;
    //Token::close();
}

Term*
Parser::ParseTerm()
{
    Term* t = ParseTermBase();
    if (t == 0) return t;
    
    Term* t2 = ParseTermRest(t);
    
    return t2;
}

Term*
Parser::ParseTermRest(Term* base)
{
    Token    t = lexer->Get();
    
    switch (t.type()) {
    default:
        lexer->PutBack();
        return base;

    case Token::StringLiteral: {
            // concatenate adjacent string literal tokens:
            TermText* text  = base->isText();
            if (text) {
                TermText* base2 = new(__FILE__, __LINE__) TermText(text->value() + t.symbol()(1, t.symbol().length()-2));
                delete base;
                return ParseTermRest(base2);
            }
            else {
                lexer->PutBack();
            }
        }
        break;

    case Token::Keyword:
        switch (t.key()) {
        case KEY_DEF:
            if (base->isText())
                return new(__FILE__, __LINE__) TermDef(base->isText(), ParseTerm());
            else
                return error("(Parse) illegal lhs in def", t);

        default:
            lexer->PutBack();
            return base;
        }
        break;
    }
    
    return base;
}

static int xtol(const char* p)
{
    int n = 0;

    while (*p) {
        char digit = *p++;
        n *= 16;

        if (digit >= '0' && digit <= '9')
            n += digit - '0';

        else if (digit >= 'a' && digit <= 'f')
            n += digit - 'a' + 10;

        else if (digit >= 'A' && digit <= 'F')
            n += digit - 'A' + 10;
    }

    return n;
}

Term*
Parser::ParseTermBase()
{
    Token    t = lexer->Get();
    int      n = 0;
    double   d = 0.0;
    
    switch (t.type()) {
    case Token::IntLiteral: {
        if (dump_tokens)
            Print("%s", t.symbol().data());

        char nstr[256], *p = nstr;
        for (int i = 0; i < (int) t.symbol().length(); i++)
            if (t.symbol()[i] != '_')
                *p++ = t.symbol()[i];
        *p++ = '\0';
        
        // handle hex notation:
        if (nstr[1] == 'x')
            n = xtol(nstr+2);

        else
            n = atol(nstr);

        return new(__FILE__, __LINE__) TermNumber(n);
        }

    case Token::FloatLiteral: {
        if (dump_tokens)
            Print("%s", t.symbol().data());

        char nstr[256], *p = nstr;
        for (int i = 0; i < (int) t.symbol().length(); i++)
            if (t.symbol()[i] != '_')
                *p++ = t.symbol()[i];
        *p++ = '\0';
        
        d = atof(nstr);
        return new(__FILE__, __LINE__) TermNumber(d);
        }

    case Token::StringLiteral:
        if (dump_tokens)
            Print("%s", t.symbol().data());

        return new(__FILE__, __LINE__) TermText(t.symbol()(1, t.symbol().length()-2));

    case Token::AlphaIdent:
        if (dump_tokens)
            Print("%s", t.symbol().data());

        return new(__FILE__, __LINE__) TermText(t.symbol());

    case Token::Keyword:
        if (dump_tokens)
            Print("%s", t.symbol().data());

        switch (t.key()) {
        case KEY_FALSE:   return new(__FILE__, __LINE__) TermBool(0);
        case KEY_TRUE:    return new(__FILE__, __LINE__) TermBool(1);
        
        case KEY_MINUS: {
                Token next = lexer->Get();
                if (next.type() == Token::IntLiteral) {
                    if (dump_tokens)
                        Print("%s", next.symbol().data());

                    char nstr[256], *p = nstr;
                    for (int i = 0; i < (int) next.symbol().length(); i++)
                        if (next.symbol()[i] != '_')
                            *p++ = next.symbol()[i];
                    *p++ = '\0';
                    
                    n = -1 * atol(nstr);
                    return new(__FILE__, __LINE__) TermNumber(n);
                }
                else if (next.type() == Token::FloatLiteral) {
                    if (dump_tokens)
                        Print("%s", next.symbol().data());

                    char nstr[256], *p = nstr;
                    for (int i = 0; i < (int) next.symbol().length(); i++)
                        if (next.symbol()[i] != '_')
                            *p++ = next.symbol()[i];
                    *p++ = '\0';
                    
                    d = -1.0 * atof(nstr);
                    return new(__FILE__, __LINE__) TermNumber(d);
                }
                else {
                    lexer->PutBack();
                    return error("(Parse) illegal token '-': number expected", next);
                }
            }
            break;
        
        default:
            lexer->PutBack();
            return 0;
        }

    case Token::LParen:  return ParseArray();

    case Token::LBrace:  return ParseStruct();

    case Token::CharLiteral:
        return error("(Parse) illegal token ", t);

    default:
        lexer->PutBack();
        return 0;
    }
}

TermArray*
Parser::ParseArray()
{
    TermList*   elems = ParseTermList(0);
    Token       end = lexer->Get();

    if (end.type() != Token::RParen)
        return (TermArray*) error("(Parse) ')' missing in array-decl", end);

    return new(__FILE__, __LINE__) TermArray(elems);
}

TermStruct*
Parser::ParseStruct()
{
    TermList*   elems = ParseTermList(1);
    Token       end = lexer->Get();

    if (end.type() != Token::RBrace)
        return (TermStruct*) error("(Parse) '}' missing in struct", end);

    return new(__FILE__, __LINE__) TermStruct(elems);
}

TermList*
Parser::ParseTermList(int for_struct)
{
    TermList*   tlist = new(__FILE__, __LINE__) TermList;
    
    Term* term = ParseTerm();
    while (term) {
        if (for_struct && !term->isDef()) {
            return (TermList*) error("(Parse) non-definition term in struct");
        }
        else if (!for_struct && term->isDef()) {
            return (TermList*) error("(Parse) illegal definition in array");
        }

        tlist->append(term);
        Token t = lexer->Get();

        /*** OLD WAY: COMMA SEPARATORS REQUIRED ***
        if (t.type() != Token::Comma) {
            lexer->PutBack();
            term = 0;
        }
        else
            term = ParseTerm();
        /*******************************************/

        // NEW WAY: COMMA SEPARATORS OPTIONAL:
        if (t.type() != Token::Comma) {
            lexer->PutBack();
        }

        term = ParseTerm();
    }
    
    return tlist;
}



