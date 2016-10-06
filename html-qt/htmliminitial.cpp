#include "htmliminitial.h"

#include "htmltree.h"
#include "htmltokenizer_p.h"
#include "htmlparser_p.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(HTML_IM_INITIAL, "htmlqt.im.initial")

HTMLIMInitial::HTMLIMInitial(HTMLParser *parser, HTMLTree *tree) : HTMLInsertMode(parser, tree)
{

}

bool HTMLIMInitial::processCharacter(const QChar &c)
{
    qCCritical(HTML_IM_INITIAL) << Q_FUNC_INFO << c;
    tree->inserText();
    return true;
}

bool HTMLIMInitial::processSpaceCharacter(const QChar &c)
{
    qCCritical(HTML_IM_INITIAL) << Q_FUNC_INFO << c;
    return true;
}

bool HTMLIMInitial::processStartTag(HTMLToken *token)
{
    qCCritical(HTML_IM_INITIAL) << Q_FUNC_INFO << token;
    return true;
}

bool HTMLIMInitial::processEndTag(HTMLToken *token)
{
    qCCritical(HTML_IM_INITIAL) << Q_FUNC_INFO << token;
    return true;
}

bool HTMLIMInitial::processCommentTag(HTMLToken *token)
{
    qCCritical(HTML_IM_INITIAL) << Q_FUNC_INFO << token;
    tree->insertComment(token->name, tree->document());
    return true;
}

bool HTMLIMInitial::processDoctype(HTMLToken *token)
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

    // TODO

    parserPriv()->insertionModeEnum = HTMLParser::BeforeHTML;
    parserPriv()->insertionMode = parserPriv()->imBeforeHTML;
    return true;
}

