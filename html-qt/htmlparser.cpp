#include "htmlparser_p.h"

#include "htmliminitial.h"

#include <QMetaEnum>
#include <QDebug>

HTMLParser::HTMLParser(QObject *parent) : QObject(parent)
  , d_ptr(new HTMLParserPrivate)
{
    Q_D(HTMLParser);

    d->tokenizer = new HTMLTokenizer(this);

    HTMLTree *tree = new HTMLTree;
    d->imInitial = new HTMLIMInitial(this, tree);
    d->imBeforeHTML = new HTMLInsertMode(this, tree);
    d->imBeforeHead = new HTMLInsertMode(this, tree);
    d->imInHead = new HTMLInsertMode(this, tree);
    d->imInHeadNoScript = new HTMLInsertMode(this, tree);
    d->imAfterHead = new HTMLInsertMode(this, tree);
    d->imInBody = new HTMLInsertMode(this, tree);
    d->imText = new HTMLInsertMode(this, tree);
    d->imInTable = new HTMLInsertMode(this, tree);
    d->imInTableText = new HTMLInsertMode(this, tree);
    d->imInCaption = new HTMLInsertMode(this, tree);
    d->imInColumGroup = new HTMLInsertMode(this, tree);
    d->imInTableBody = new HTMLInsertMode(this, tree);
    d->imInRow = new HTMLInsertMode(this, tree);
    d->imInCell = new HTMLInsertMode(this, tree);
    d->imInSelect = new HTMLInsertMode(this, tree);
    d->imInSelectInTable = new HTMLInsertMode(this, tree);
    d->imInTemplate = new HTMLInsertMode(this, tree);
    d->imAfterBody = new HTMLInsertMode(this, tree);
    d->imInFrameset = new HTMLInsertMode(this, tree);
    d->imAfterFrameset = new HTMLInsertMode(this, tree);
    d->imAfterAfterBody = new HTMLInsertMode(this, tree);
    d->imAfterAfterFrameset = new HTMLInsertMode(this, tree);
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
}

void HTMLParser::characterToken(const QChar &c)
{
    Q_D(HTMLParser);
    d->insertionMode->processCharacter(c);
}

void HTMLParser::parserErrorToken(const QString &string)
{
    qDebug() << "parser-error" << string;
}

void HTMLParser::parseToken(HTMLToken *token)
{
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
