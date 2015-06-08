#include "htmlparser_p.h"

#include <QMetaEnum>
#include <QDebug>

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

HTMLParser::HTMLParser(QObject *parent) : QObject(parent)
  , d_ptr(new HTMLParserPrivate)
{
    Q_D(HTMLParser);

    d->tokenizer = new HTMLTokenizer(this);
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

HTMLParserPrivate::HTMLParserPrivate()
{
    imInitial = new HTMLInsertMode;
    imBeforeHTML = new HTMLInsertMode;
    imBeforeHead = new HTMLInsertMode;
    imInHead = new HTMLInsertMode;
    imInHeadNoScript = new HTMLInsertMode;
    imAfterHead = new HTMLInsertMode;
    imInBody = new HTMLInsertMode;
    imText = new HTMLInsertMode;
    imInTable = new HTMLInsertMode;
    imInTableText = new HTMLInsertMode;
    imInCaption = new HTMLInsertMode;
    imInColumGroup = new HTMLInsertMode;
    imInTableBody = new HTMLInsertMode;
    imInRow = new HTMLInsertMode;
    imInCell = new HTMLInsertMode;
    imInSelect = new HTMLInsertMode;
    imInSelectInTable = new HTMLInsertMode;
    imInTemplate = new HTMLInsertMode;
    imAfterBody = new HTMLInsertMode;
    imInFrameset = new HTMLInsertMode;
    imAfterFrameset = new HTMLInsertMode;
    imAfterAfterBody = new HTMLInsertMode;
    imAfterAfterFrameset = new HTMLInsertMode;
    insertionMode = imInitial;
}

bool HTMLParserPrivate::initial(HTMLToken *token)
{
    return true;
}

void HTMLParserPrivate::characterToken(const QChar &c)
{
//    qDebug() << c;
}

void HTMLParserPrivate::parserErrorToken(const QString &string)
{
    qDebug() << string;
}

void HTMLParserPrivate::parseToken(HTMLToken *token)
{
    qDebug() << token->type << token->name;
    bool ret = CALL_MEMBER_FN(*this, phaseFn)(token);
    qDebug() << ret << HTMLParser::staticQtMetaObject.enumerator(0).key(phase);
}
