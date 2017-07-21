#ifndef HTMLABSTRACTPHASE_H
#define HTMLABSTRACTPHASE_H

#include <QObject>

class HTMLParserPrivate;
class HTMLParser;
class HTMLTree;
class HTMLToken;
class HTMLAbstractPhase
{
public:
    HTMLAbstractPhase(HTMLParser *parser, HTMLTree *tree);
    virtual ~HTMLAbstractPhase();

    HTMLTree *tree;
    HTMLParser *parser;
    HTMLParserPrivate *parserPriv();

    virtual bool processCharacter(const QChar &c);

    virtual bool processSpaceCharacter(const QChar &c);

    virtual bool processStartTag(HTMLToken *token);

    virtual bool processEndTag(HTMLToken *token);

    virtual bool processCommentTag(HTMLToken *token);

    virtual bool processDoctype(HTMLToken *token);
};

#endif // HTMLABSTRACTPHASE_H
