#ifndef HTMLIMINITIAL_H
#define HTMLIMINITIAL_H

#include "htmlinsertmode.h"

class HTMLIMInitial : public HTMLInsertMode
{
public:
    HTMLIMInitial(HTMLParser *parser, HTMLTree *tree);

    virtual bool processCharacter(const QChar &c) override;

    virtual bool processSpaceCharacter(const QChar &c) override;

    virtual bool processStartTag(HTMLToken *token) override;

    virtual bool processEndTag(HTMLToken *token) override;

    virtual bool processCommentTag(HTMLToken *token) override;

    virtual bool processDoctype(HTMLToken *token) override;

};

#endif // HTMLIMINITIAL_H
