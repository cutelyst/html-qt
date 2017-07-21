#include "htmlabstractphase.h"
#include "htmlparser.h"
#include "htmlparser_p.h"

#include "htmltree.h"

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

void HTMLAbstractPhase::insertHtmlElement()
{

}

void HTMLAbstractPhase::startTagHtml(HTMLToken *token)
{
    if (!parserPriv()->firstStartTag && token->name == QLatin1String("html")) {
        parser->parserErrorToken(QStringLiteral("non-html-root"), 0);
        return;
    }

    HTMLTreeNode *last = tree->openElements().last();

    auto it = token->data.constBegin();
    while (it != token->data.constEnd()) {
        const QString attr = it->first;
        const QString value = it->second;
        if (!last->attributes.contains(attr)) {
            last->attributes.insert(attr, value);
        }
        ++it;
    }
    parserPriv()->firstStartTag = false;
}

bool HTMLAbstractPhase::processCharacter(QChar c)
{
    tree->insertText(c);
    return true;
}

bool HTMLAbstractPhase::processSpaceCharacters(HTMLToken *token)
{
    Q_UNUSED(token)
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
    tree->insertComment(token, tree->openElements().last());
    return true;
}

bool HTMLAbstractPhase::processDoctype(HTMLToken *token)
{
    Q_UNUSED(token)
    return true;
}

bool HTMLAbstractPhase::processEOF()
{
    return true;
}

