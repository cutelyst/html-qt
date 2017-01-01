#include "htmlinsertmode.h"
#include "htmlparser.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(HTML_IM, "htmlqt.im")

HTMLInsertMode::HTMLInsertMode(HTMLParser *parser, HTMLTree *tree)
{
    this->tree = tree;
    this->parser = parser;
}

HTMLInsertMode::~HTMLInsertMode()
{

}

HTMLParserPrivate *HTMLInsertMode::parserPriv()
{
    return parser->d_ptr;
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

