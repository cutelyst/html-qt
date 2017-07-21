#ifndef HTMLTOKENIZER_H
#define HTMLTOKENIZER_H

#include <QObject>

class HTMLParser;
class HTMLToken;
class HTMLTokenizerPrivate;
class HTMLTokenizer : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(HTMLTokenizer)
public:
    enum State {
        DataState,
        CharacterReferenceInDataState,
        RCDataState,
        CharacterReferenceInRCDataState,
        RawTextState,
        ScriptDataState,
        PlainTextState,
        TagOpenState,
        EndTagOpenState,
        TagNameState,
        RCDataLessThanSignState,
        RCDataEndTagOpenState,
        RCDataEndTagNameState,
        RawTextLessThanSignState,
        RawTextEndTagOpenState,
        RawTextEndTagNameState,
        ScriptDataLessThanSignState,
        ScriptDataEndTagOpenState,
        ScriptDataEndTagNameState,
        ScriptDataEscapeStartState,
        ScriptDataEscapeStartDashState,
        ScriptDataEscapedState,
        ScriptDataEscapedDashState,
        ScriptDataEscapedDashDashState,
        ScriptDataEscapedLessThanSignState,
        ScriptDataEscapedEndTagOpenState,
        ScriptDataEscapedEndTagNameState,
        ScriptDataDoubleEscapeStartState,
        ScriptDataDoubleEscapedState,
        ScriptDataDoubleEscapedDashState,
        ScriptDataDoubleEscapedDashDashState,
        ScriptDataDoubleEscapedLessThanSignState,
        ScriptDataDoubleEscapeEndState,
        BeforeAttributeNameState,
        AttributeNameState,
        AfterAttributeNameState,
        BeforeAttributeValueState,
        AttributeValueDoubleQuotedState,
        AttributeValueSingleQuotedState,
        AttributeValueUnquotedState,
        CharacterReferenceInAttributeValueState,
        AfterAttributeValueQuotedState,
        SelfClosingStartTagState,
        BogusCommentState,
        MarkupDeclarationOpenState,
        CommentStartState,
        CommentStartDashState,
        CommentState,
        CommentEndDashState,
        CommentEndState,
        CommentEndBangState,
        DocTypeState,
        BeforeDocTypeNameState,
        DocTypeNameState,
        AfterDocTypeNameState,
        AfterDocTypePublicKeywordState,
        BeforeDocTypePublicIdentifierState,
        DocTypePublicIdentifierDoubleQuotedState,
        DocTypePublicIdentifierSingleQuotedState,
        AfterDocTypePublicIdentifierState,
        BetweenDocTypePublicAndSystemIdentifierState,
        AfterDocTypeSystemKeywordState,
        BeforeDocTypeSystemIdentifierState,
        DocTypeSystemIdentifierDoubleQuotedState,
        DocTypeSystemIdentifierSingleQuotedState,
        AfterDocTypeSystemIdentifierState,
        BogusDocTypeState,
        CDataSectionState,
    };
    Q_ENUM(State)
    HTMLTokenizer(HTMLParser *parser);
    ~HTMLTokenizer();

    void setHtmlText(const QString &html);

    State state() const;

    void start();

protected:
    void character(QChar c);
    void parserError(const QString &error);
    void token(HTMLToken *token);

    HTMLTokenizerPrivate *d_ptr;
};

#endif // HTMLTOKENIZER_H
