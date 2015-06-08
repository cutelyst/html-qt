#ifndef HTMLINSERTMODE_H
#define HTMLINSERTMODE_H

#include <QObject>

class HTMLToken;
class HTMLInsertMode
{
public:
    HTMLInsertMode();
    ~HTMLInsertMode();

    virtual bool processCharacter(const QChar &c);

    virtual bool processSpaceCharacter(const QChar &c);

    virtual bool processStartTag(HTMLToken *token);

    virtual bool processEndTag(HTMLToken *token);

    virtual bool processCommentTag(HTMLToken *token);

    virtual bool processDoctype(HTMLToken *token);
};

#endif // HTMLINSERTMODE_H
