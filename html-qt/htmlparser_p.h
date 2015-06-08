#ifndef HTMLPARSER_P_H
#define HTMLPARSER_P_H

#include "htmlparser.h"
#include "htmltokenizer_p.h"
#include "htmltree.h"

typedef  bool (HTMLParserPrivate::*HTMLParserPrivateMemFn)(HTMLToken *);

class HTMLParserPrivate : public QObject
{
    Q_OBJECT
public:
    bool initial(HTMLToken *token);

    void characterToken(const QChar &c);
    void parserErrorToken(const QString &string);
    void parseToken(HTMLToken *token);

    QString html;
    HTMLTokenizer *tokenizer;
    HTMLTree *tree;
    HTMLParser::InsertionMode phase = HTMLParser::Initial;
    HTMLParserPrivateMemFn phaseFn = &HTMLParserPrivate::initial;
};

#endif // HTMLPARSER_P_H

