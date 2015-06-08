#ifndef HTMLINSERTMODE_H
#define HTMLINSERTMODE_H

#include <QObject>

class HTMLParserPrivate;
class HTMLParser;
class HTMLTree;
class HTMLToken;
class HTMLInsertMode
{
public:
    HTMLInsertMode(HTMLParser *parser, HTMLTree *tree);
    virtual ~HTMLInsertMode();

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

#endif // HTMLINSERTMODE_H
