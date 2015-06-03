#ifndef HTMLTOKENIZER_P_H
#define HTMLTOKENIZER_P_H

#include "htmltokenizer.h"

#include <QPair>
#include <QMap>

typedef  bool (HTMLTokenizerPrivate::*HTMLTokenizerPrivateMemFn)();

class HTMLToken
{
public:
    enum Type {
        StartTagToken,
        EndTagToken,
        CommentToken,
        DocTypeToken
    };
    HTMLToken(Type tokenType) : type(tokenType) {}

    void appendDataCurrentAttributeValue(const QChar &c)
    {
        if (data.isEmpty()) {
            data.append(qMakePair<QString,QString>("", c));
        } else {
            data.last().second.append(c);
        }
    }

    void appendDataCurrentAttributeValue(const QString &s)
    {
        if (data.isEmpty()) {
            data.append(qMakePair<QString,QString>("", s));
        } else {
            data.last().second.append(s);
        }
    }

    QString name; // or data for comment
    Type type;
    QList<QPair<QString,QString> > data;
    bool selfClosing = false;
    bool selfClosingAcknowledged = false;
    bool forceQuirks = false;
    QString doctypePublicId;
    QString doctypeSystemId;
};

class HTMLTokenizerPrivate
{
    Q_DECLARE_PUBLIC(HTMLTokenizer)
public:
    // State methods
    bool dataState();
    bool characterReferenceInDataState();
    bool tagOpenState();
    bool endTagOpenState();
    bool tagNameState();
    // ... RC Raw Script
    bool beforeAttributeNameState();
    bool attributeNameState();
    bool afterAttributeNameState();
    bool beforeAttributeValueState();
    bool attributeValueDoubleQuotedState();
    bool attributeValueSingleQuotedState();
    bool attributeValueUnquotedState();
    // This method is special as for simplicity it is directly called by the callers
    void characterReferenceInAttributeValueState(QChar *additionalAllowedCharacter);
    bool afterAttributeValueQuotedState();
    bool selfClosingStartTagState();
    bool bogusCommentState();
    bool markupDeclarationOpenState();
    bool commentStartState();
    bool commentStartDashState();
    bool commentState();
    bool commentEndDashState();
    bool commentEndState();
    bool commentEndBangState();
    bool doctypeState();
    bool beforeDocTypeNameState();
    bool docTypeNameState();
    bool afterDocTypeNameState();
    bool afterDocTypePublicKeywordState();
    bool beforeDocTypePublicIdentifierState();
    bool docTypePublicIdentifierDoubleQuotedState();
    bool docTypePublicIdentifierSingleQuotedState();
    bool afterDocTypePublicIdentifierState();
    bool betweenDocTypePublicAndSystemIdentifierState();
    bool afterDocTypeSystemKeywordState();
    bool beforeDocTypeSystemIdentifierState();
    bool docTypeSystemIdentifierDoubleQuotedState();
    bool docTypeSystemIdentifierSingleQuotedState();
    bool afterDocTypeSystemIdentifierState();
    bool bogusDocTypeState();
    bool cDataSectionState();

    // auxiliary methods
    inline QChar consumeStream()
    {
        if (++htmlPos > html.size() || htmlPos < 0) {
            return QChar();
        } else {
            return html.at(htmlPos);
        }
    }

    inline int streamPos() {
        return htmlPos;
    }

    inline void streamSeek(int pos) {
        htmlPos = pos;
    }

    inline void streamUnconsume(int nChars = 1) {
        htmlPos -= nChars;
    }

    inline bool streamAtEnd() {
        return htmlPos > html.size();
    }

    QString consumeEntity(QChar *allowedChar = 0);
    QChar consumeNumberEntity(bool isHex);
    void emitCurrentTagToken();

    // current token
    HTMLToken *currentToken;

    HTMLTokenizer *q_ptr;
    QString html;
    int htmlPos = -1;
    HTMLTokenizer::State state = HTMLTokenizer::DataState;
    HTMLTokenizerPrivateMemFn stateFn = &HTMLTokenizerPrivate::dataState;
    QMap<int,int> replacementCharacters = {
        {0x00, 0xFFFD}, // REPLACEMENT CHARACTER
        {0x80, 0x20AC}, // EURO SIGN (€)
        {0x82, 0x201A}, // SINGLE LOW-9 QUOTATION MARK (‚)
        {0x83, 0x0192}, // LATIN SMALL LETTER F WITH HOOK (ƒ)
        {0x84, 0x201E}, // DOUBLE LOW-9 QUOTATION MARK („)
        {0x85, 0x2026}, // HORIZONTAL ELLIPSIS (…)
        {0x86, 0x2020}, // DAGGER (†)
        {0x87, 0x2021}, // DOUBLE DAGGER (‡)
        {0x88, 0x02C6}, // MODIFIER LETTER CIRCUMFLEX ACCENT (ˆ)
        {0x89, 0x2030}, // PER MILLE SIGN (‰)
        {0x8A, 0x0160}, // LATIN CAPITAL LETTER S WITH CARON (Š)
        {0x8B, 0x2039}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK (‹)
        {0x8C, 0x0152}, // LATIN CAPITAL LIGATURE OE (Œ)
        {0x8E, 0x017D}, // LATIN CAPITAL LETTER Z WITH CARON (Ž)
        {0x91, 0x2018}, // LEFT SINGLE QUOTATION MARK (‘)
        {0x92, 0x2019}, // RIGHT SINGLE QUOTATION MARK (’)
        {0x93, 0x201C}, // LEFT DOUBLE QUOTATION MARK (“)
        {0x94, 0x201D}, // RIGHT DOUBLE QUOTATION MARK (”)
        {0x95, 0x2022}, // BULLET (•)
        {0x96, 0x2013}, // EN DASH (–)
        {0x97, 0x2014}, // EM DASH (—)
        {0x98, 0x02DC}, // SMALL TILDE (˜)
        {0x99, 0x2122}, // TRADE MARK SIGN (™)
        {0x9A, 0x0161}, // LATIN SMALL LETTER S WITH CARON (š)
        {0x9B, 0x203A}, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK (›)
        {0x9C, 0x0153}, // LATIN SMALL LIGATURE OE (œ)
        {0x9E, 0x017E}, // LATIN SMALL LETTER Z WITH CARON (ž)
        {0x9F, 0x0178}, // LATIN CAPITAL LETTER Y WITH DIAERESIS (Ÿ)
    };


};

#endif // HTMLTOKENIZER_P_H

