#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include <QObject>

class HTMLToken;
class HTMLParserPrivate;
class HTMLParser : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(HTMLParser)
public:
    enum InsertionMode {
        Initial,
        BeforeHTML,
        BeforeHead,
        InHead,
        InHeadNoScript,
        AfterHead,
        InBody,
        Text,
        InTable,
        InTableText,
        InCaption,
        InColumGroup,
        InTableBody,
        InRow,
        InCell,
        InSelect,
        InSelectInTable,
        InTemplate,
        AfterBody,
        InFrameset,
        AfterFrameset,
        AfterAfterBody,
        AfterAfterFrameset,
    };
    Q_ENUM(InsertionMode)

    explicit HTMLParser(QObject *parent = 0);
    ~HTMLParser();

    void parse(const QString &html);

    void reset();

protected:
    void characterToken(const QChar &c);
    void parserErrorToken(const QString &string, int pos);
    void parseToken(HTMLToken *token);

    friend class HTMLTokenizer;
    friend class HTMLAbstractPhase;

    HTMLParserPrivate *d_ptr;
};

#endif // HTMLPARSER_H
