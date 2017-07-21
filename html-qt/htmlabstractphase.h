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

    virtual void insertHtmlElement();

    virtual void startTagHtml(HTMLToken *token);

    virtual bool processCharacter(QChar c);

    virtual bool processSpaceCharacters(HTMLToken *token);

    virtual bool processStartTag(HTMLToken *token);

    virtual bool processEndTag(HTMLToken *token);

    virtual bool processCommentTag(HTMLToken *token);

    virtual bool processDoctype(HTMLToken *token);

    virtual bool processEOF();

};

#endif // HTMLABSTRACTPHASE_H
