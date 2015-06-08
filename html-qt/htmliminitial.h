#ifndef HTMLIMINITIAL_H
#define HTMLIMINITIAL_H

#include "htmlinsertmode.h"

class HTMLIMInitial : public HTMLInsertMode
{
public:
    HTMLIMInitial(HTMLParser *parser, HTMLTree *tree);

    virtual bool processCharacter(const QChar &c);

    virtual bool processCommentTag(HTMLToken *token);

    virtual bool processDoctype(HTMLToken *token);

};

#endif // HTMLIMINITIAL_H
