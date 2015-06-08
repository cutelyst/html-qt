#include "htmliminitial.h"

#include "htmltree.h"
#include "htmltokenizer_p.h"
#include "htmlparser_p.h"

HTMLIMInitial::HTMLIMInitial(HTMLParser *parser, HTMLTree *tree) : HTMLInsertMode(parser, tree)
{

}

bool HTMLIMInitial::processCharacter(const QChar &c)
{
    return true;
}

bool HTMLIMInitial::processCommentTag(HTMLToken *token)
{
    tree->insertComment(token->name, tree->document());
    return true;
}

bool HTMLIMInitial::processDoctype(HTMLToken *token)
{
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

