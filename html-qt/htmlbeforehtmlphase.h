#ifndef HTMLBEFOREHTMLPHASE_H
#define HTMLBEFOREHTMLPHASE_H

#include "htmlabstractphase.h"

class HTMLBeforeHtmlPhase : public HTMLAbstractPhase
{
public:
    HTMLBeforeHtmlPhase(HTMLParser *parser, HTMLTree *tree);

    virtual void insertHtmlElement() override;

    virtual bool processEOF();

    virtual bool processCharacter(QChar c) override;

    virtual bool processCommentTag(HTMLToken *token) override;

    virtual bool processStartTag(HTMLToken *token) override;
};

#endif // HTMLBEFOREHTMLPHASE_H
