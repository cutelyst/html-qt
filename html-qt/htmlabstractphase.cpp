#include "htmlabstractphase.h"
#include "htmlparser.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(HTML_IM, "htmlqt.im")

HTMLAbstractPhase::HTMLAbstractPhase(HTMLParser *parser, HTMLTree *tree)
{
    this->tree = tree;
    this->parser = parser;
}

HTMLAbstractPhase::~HTMLAbstractPhase()
{

}

HTMLParserPrivate *HTMLAbstractPhase::parserPriv()
{
    return parser->d_ptr;
}

bool HTMLAbstractPhase::processCharacter(const QChar &c)
{
    Q_UNUSED(c)
    return true;
}

bool HTMLAbstractPhase::processSpaceCharacter(const QChar &c)
{
    Q_UNUSED(c)
    return true;
}

bool HTMLAbstractPhase::processStartTag(HTMLToken *token)
{
    Q_UNUSED(token)
    return true;
}

bool HTMLAbstractPhase::processEndTag(HTMLToken *token)
{
    Q_UNUSED(token)
    return true;
}

bool HTMLAbstractPhase::processCommentTag(HTMLToken *token)
{
    Q_UNUSED(token)
    return true;
}

bool HTMLAbstractPhase::processDoctype(HTMLToken *token)
{
    Q_UNUSED(token)
    return true;
}

