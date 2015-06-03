#ifndef HTMLPARSER_P_H
#define HTMLPARSER_P_H

#include "htmlparser.h"
#include "htmltokenizer_p.h"

class HTMLParserPrivate : public QObject
{
    Q_OBJECT
public:
    void characterToken(const QChar &c);
    void charactersToken(const QString &string);
    void parserErrorToken(const QString &string);
    void parseToken(HTMLToken *token);

    QString html;
    HTMLTokenizer *tokenizer;
};

#endif // HTMLPARSER_P_H

