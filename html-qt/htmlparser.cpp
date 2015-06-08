#include "htmlparser_p.h"

#include <QMetaEnum>
#include <QDebug>

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

HTMLParser::HTMLParser(QObject *parent) : QObject(parent)
  , d_ptr(new HTMLParserPrivate)
{
    Q_D(HTMLParser);

    d->tokenizer = new HTMLTokenizer(this);
    connect(d->tokenizer, &HTMLTokenizer::character, d, &HTMLParserPrivate::characterToken);
    connect(d->tokenizer, &HTMLTokenizer::characterString, d, &HTMLParserPrivate::charactersToken);
    connect(d->tokenizer, &HTMLTokenizer::parserError, d, &HTMLParserPrivate::parserErrorToken);
    connect(d->tokenizer, &HTMLTokenizer::token, d, &HTMLParserPrivate::parseToken);
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

bool HTMLParserPrivate::initial(HTMLToken *token)
{
    return true;
}

void HTMLParserPrivate::characterToken(const QChar &c)
{
    qDebug() << c;
}

void HTMLParserPrivate::charactersToken(const QString &string)
{
    qDebug() << string;
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
