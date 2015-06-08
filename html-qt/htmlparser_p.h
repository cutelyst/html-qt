#ifndef HTMLPARSER_P_H
#define HTMLPARSER_P_H

#include "htmlparser.h"
#include "htmltokenizer_p.h"
#include "htmltree.h"

#include "htmlinsertmode.h"

class HTMLParserPrivate : public QObject
{
    Q_OBJECT
public:
    QString html;
    HTMLTokenizer *tokenizer;
    HTMLTree *tree;
    HTMLInsertMode *insertionMode;
    HTMLParser::InsertionMode insertionModeEnum = HTMLParser::Initial;

    HTMLInsertMode *imInitial;
    HTMLInsertMode *imBeforeHTML;
    HTMLInsertMode *imBeforeHead;
    HTMLInsertMode *imInHead;
    HTMLInsertMode *imInHeadNoScript;
    HTMLInsertMode *imAfterHead;
    HTMLInsertMode *imInBody;
    HTMLInsertMode *imText;
    HTMLInsertMode *imInTable;
    HTMLInsertMode *imInTableText;
    HTMLInsertMode *imInCaption;
    HTMLInsertMode *imInColumGroup;
    HTMLInsertMode *imInTableBody;
    HTMLInsertMode *imInRow;
    HTMLInsertMode *imInCell;
    HTMLInsertMode *imInSelect;
    HTMLInsertMode *imInSelectInTable;
    HTMLInsertMode *imInTemplate;
    HTMLInsertMode *imAfterBody;
    HTMLInsertMode *imInFrameset;
    HTMLInsertMode *imAfterFrameset;
    HTMLInsertMode *imAfterAfterBody;
    HTMLInsertMode *imAfterAfterFrameset;
};

#endif // HTMLPARSER_P_H

