#ifndef HTMLINITIALPHASE_H
#define HTMLINITIALPHASE_H

#include "htmlabstractphase.h"

class HTMLInitialPhase : public HTMLAbstractPhase
{
public:
    HTMLInitialPhase(HTMLParser *parser, HTMLTree *tree);

    virtual bool processSpaceCharacters(HTMLToken *token) override;

    virtual bool processStartTag(HTMLToken *token) override;

    virtual bool processEndTag(HTMLToken *token) override;

    virtual bool processCommentTag(HTMLToken *token) override;

    virtual bool processDoctype(HTMLToken *token) override;

};

#endif // HTMLINITIALPHASE_H
