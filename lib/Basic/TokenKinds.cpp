//
// Created by BY210033 on 2022/11/7.
//
#include "tinylang/Basic/TokenKinds.h"

#include <llvm/Support/ErrorHandling.h>

using namespace tinylang;
using namespace llvm;

static const char * const TokNames[] = {
#define TOK(ID) #ID,
#define KEYWORD(ID, FLAG) # ID,
#include "tinylang/Basic/TokenKinds.def"
        nullptr
};

const char *tok::getTokenName(tok::TokenKind Kind) {
    return TokNames[Kind];
}

const char *tok::getPunctuatorSpelling(tok::TokenKind Kind) {
    switch (Kind) {
#define PUNCTUATOR(ID, SP) case ID: return SP;
#include "tinylang/Basic/TokenKinds.def"
        default:
            break;
    }
    return nullptr;
}

const char *tok::getKeywordSpelling(tok::TokenKind Kind) {
    switch (Kind) {
#define KEYWORD(ID, FLAG) case kw_ ## ID: return #ID;
#include "tinylang/Basic/TokenKinds.def"
        default:
            break;

    }
    return nullptr;
}
