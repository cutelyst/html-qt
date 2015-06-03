#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include <QObject>

class HTMLParserPrivate;
class HTMLParser : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(HTMLParser)
public:
    explicit HTMLParser(QObject *parent = 0);
    ~HTMLParser();

    void parse(const QString &html);

protected:
    HTMLParserPrivate *d_ptr;
};

#endif // HTMLPARSER_H
