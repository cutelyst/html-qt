#include "htmlinsertmode.h"

HTMLInsertMode::HTMLInsertMode()
{

}

HTMLInsertMode::~HTMLInsertMode()
{

}

bool HTMLInsertMode::processCharacter(const QChar &c)
{
    Q_UNUSED(c)
    return true;
}

bool HTMLInsertMode::processSpaceCharacter(const QChar &c)
{
    Q_UNUSED(c)
    return true;
}

bool HTMLInsertMode::processStartTag(HTMLToken *token)
{
    Q_UNUSED(token)
    return true;
}

bool HTMLInsertMode::processEndTag(HTMLToken *token)
{
    Q_UNUSED(token)
    return true;
}

bool HTMLInsertMode::processCommentTag(HTMLToken *token)
{
    Q_UNUSED(token)
    return true;
}

bool HTMLInsertMode::processDoctype(HTMLToken *token)
{
    Q_UNUSED(token)
    return true;
}

