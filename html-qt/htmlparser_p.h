#ifndef HTMLPARSER_P_H
#define HTMLPARSER_P_H

#include "htmlparser.h"
#include "htmltokenizer_p.h"
#include "htmltree.h"

#include "htmlabstractphase.h"

class HTMLParserPrivate : public QObject
{
    Q_OBJECT
public:
    QString html;
    HTMLTokenizer *tokenizer;
    HTMLTree *tree;
    HTMLAbstractPhase *phase;
    HTMLParser::InsertionMode insertionModeEnum = HTMLParser::Initial;

    HTMLAbstractPhase *imInitial;
    HTMLAbstractPhase *imBeforeHTML;
    HTMLAbstractPhase *imBeforeHead;
    HTMLAbstractPhase *imInHead;
    HTMLAbstractPhase *imInHeadNoScript;
    HTMLAbstractPhase *imAfterHead;
    HTMLAbstractPhase *imInBody;
    HTMLAbstractPhase *imText;
    HTMLAbstractPhase *imInTable;
    HTMLAbstractPhase *imInTableText;
    HTMLAbstractPhase *imInCaption;
    HTMLAbstractPhase *imInColumGroup;
    HTMLAbstractPhase *imInTableBody;
    HTMLAbstractPhase *imInRow;
    HTMLAbstractPhase *imInCell;
    HTMLAbstractPhase *imInSelect;
    HTMLAbstractPhase *imInSelectInTable;
    HTMLAbstractPhase *imInTemplate;
    HTMLAbstractPhase *imAfterBody;
    HTMLAbstractPhase *imInFrameset;
    HTMLAbstractPhase *imAfterFrameset;
    HTMLAbstractPhase *imAfterAfterBody;
    HTMLAbstractPhase *imAfterAfterFrameset;

    bool firstStartTag = false;
};

#endif // HTMLPARSER_P_H

