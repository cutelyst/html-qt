#include "htmlparser_p.h"

#include "htmlinitialphase.h"

#include <QMetaEnum>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(HTML_PARSER, "htmlqt.parser")

HTMLParser::HTMLParser(QObject *parent) : QObject(parent)
  , d_ptr(new HTMLParserPrivate)
{
    Q_D(HTMLParser);

    d->tokenizer = new HTMLTokenizer(this);

    HTMLTree *tree = new HTMLTree;
    d->imInitial = new HTMLInitialPhase(this, tree);
    d->imBeforeHTML = new HTMLAbstractPhase(this, tree);
    d->imBeforeHead = new HTMLAbstractPhase(this, tree);
    d->imInHead = new HTMLAbstractPhase(this, tree);
    d->imInHeadNoScript = new HTMLAbstractPhase(this, tree);
    d->imAfterHead = new HTMLAbstractPhase(this, tree);
    d->imInBody = new HTMLAbstractPhase(this, tree);
    d->imText = new HTMLAbstractPhase(this, tree);
    d->imInTable = new HTMLAbstractPhase(this, tree);
    d->imInTableText = new HTMLAbstractPhase(this, tree);
    d->imInCaption = new HTMLAbstractPhase(this, tree);
    d->imInColumGroup = new HTMLAbstractPhase(this, tree);
    d->imInTableBody = new HTMLAbstractPhase(this, tree);
    d->imInRow = new HTMLAbstractPhase(this, tree);
    d->imInCell = new HTMLAbstractPhase(this, tree);
    d->imInSelect = new HTMLAbstractPhase(this, tree);
    d->imInSelectInTable = new HTMLAbstractPhase(this, tree);
    d->imInTemplate = new HTMLAbstractPhase(this, tree);
    d->imAfterBody = new HTMLAbstractPhase(this, tree);
    d->imInFrameset = new HTMLAbstractPhase(this, tree);
    d->imAfterFrameset = new HTMLAbstractPhase(this, tree);
    d->imAfterAfterBody = new HTMLAbstractPhase(this, tree);
    d->imAfterAfterFrameset = new HTMLAbstractPhase(this, tree);
    d->insertionMode = d->imInitial;
    d->tree = tree;
}

HTMLParser::~HTMLParser()
{
    delete d_ptr;
}

void HTMLParser::parse(const QString &html)
{
    Q_D(HTMLParser);

    d->tokenizer->setHtmlText(html);
    d->tokenizer->start();
    d->tree->dump();
}

void HTMLParser::characterToken(const QChar &c)
{
    Q_D(HTMLParser);
    d->insertionMode->processCharacter(c);
}

void HTMLParser::parserErrorToken(const QString &string, int pos)
{
    qCCritical(HTML_PARSER) << "parser-error" << string << pos;
}

void HTMLParser::parseToken(HTMLToken *token)
{
    qCCritical(HTML_PARSER) << "parseToken" << token << token->type;
    Q_D(HTMLParser);
    switch (token->type) {
    case HTMLToken::StartTagToken:
        d->insertionMode->processStartTag(token);
        break;
    case HTMLToken::EndTagToken:
        d->insertionMode->processEndTag(token);
        break;
    case HTMLToken::CommentToken:
        d->insertionMode->processCommentTag(token);
        break;
    case HTMLToken::DocTypeToken:
        d->insertionMode->processDoctype(token);
        break;
    }
}
