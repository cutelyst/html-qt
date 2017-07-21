#include "htmlinitialphase.h"

#include "htmltree.h"
#include "htmltokenizer_p.h"
#include "htmlparser_p.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(HTML_IM_INITIAL, "htmlqt.im.initial")

HTMLInitialPhase::HTMLInitialPhase(HTMLParser *parser, HTMLTree *tree) : HTMLAbstractPhase(parser, tree)
{

}

bool HTMLInitialPhase::processSpaceCharacters(HTMLToken *token)
{
    qCCritical(HTML_IM_INITIAL) << Q_FUNC_INFO << token->name;
    return true;
}

bool HTMLInitialPhase::processStartTag(HTMLToken *token)
{
    qCCritical(HTML_IM_INITIAL) << Q_FUNC_INFO << token;
    return true;
}

bool HTMLInitialPhase::processEndTag(HTMLToken *token)
{
    qCCritical(HTML_IM_INITIAL) << Q_FUNC_INFO << token;
    return true;
}

bool HTMLInitialPhase::processCommentTag(HTMLToken *token)
{
    qCCritical(HTML_IM_INITIAL) << Q_FUNC_INFO << token;
    tree->insertComment(token, tree->document());
    return true;
}

bool HTMLInitialPhase::processDoctype(HTMLToken *token)
{
    qCCritical(HTML_IM_INITIAL) << Q_FUNC_INFO << token;
    const QString &name = token->name;
    QString publicId = token->doctypePublicId;
    const QString &systemId = token->doctypeSystemId;
    if (name != QLatin1String("html") ||
            !publicId.isNull() ||
            (!systemId.isNull() && systemId != QLatin1String("about:legacy-compat"))) {
//        parser->parserErrorToken("unknown-doctype");
    }

    if (publicId.isNull()) {
        publicId = QLatin1String("");
    }

    tree->insertDoctype(token);

    qCCritical(HTML_IM_INITIAL) << Q_FUNC_INFO << token;

    // TODO

    parserPriv()->insertionModeEnum = HTMLParser::BeforeHTML;
    parserPriv()->phase = parserPriv()->imBeforeHTML;
    return true;
}

