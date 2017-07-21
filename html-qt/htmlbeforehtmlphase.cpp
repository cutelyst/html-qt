#include "htmlbeforehtmlphase.h"

#include "htmltree.h"
#include "htmltokenizer_p.h"
#include "htmlparser_p.h"

HTMLBeforeHtmlPhase::HTMLBeforeHtmlPhase(HTMLParser *parser, HTMLTree *tree) : HTMLAbstractPhase(parser, tree)
{

}

void HTMLBeforeHtmlPhase::insertHtmlElement()
{
    tree->inserRoot(new HTMLToken(QStringLiteral("html"), HTMLToken::StartTagToken));
    parserPriv()->insertionModeEnum = HTMLParser::BeforeHead;
    parserPriv()->phase = parserPriv()->imBeforeHead;
}

bool HTMLBeforeHtmlPhase::processEOF()
{
    insertHtmlElement();
    return true;
}

bool HTMLBeforeHtmlPhase::processCharacter(QChar c)
{
    insertHtmlElement();
    return true;
}

bool HTMLBeforeHtmlPhase::processCommentTag(HTMLToken *token)
{
    tree->insertComment(token, tree->document());
}

bool HTMLBeforeHtmlPhase::processStartTag(HTMLToken *token)
{
    if (token->name == QLatin1String("html")) {
        parserPriv()->firstStartTag = true;
    }
    insertHtmlElement();
    return true;
}
