#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include <QObject>

class HTMLParserPrivate;
class HTMLParser : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(HTMLParser)
    Q_ENUMS(InsertionMode)
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

    explicit HTMLParser(QObject *parent = 0);
    ~HTMLParser();

    void parse(const QString &html);

protected:
    HTMLParserPrivate *d_ptr;
};

#endif // HTMLPARSER_H
