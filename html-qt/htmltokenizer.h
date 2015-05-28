#ifndef HTMLTOKENIZER_H
#define HTMLTOKENIZER_H

#include <QObject>
#include <QTextStream>

class HTMLTokenizerPrivate;
class HTMLTokenizer
{
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
    HTMLTokenizer();
    ~HTMLTokenizer();

    void setTextStream(QTextStream *stream);

    State state() const;

    void start();

Q_SIGNALS:
    void character(const QChar &c);
    void parserError(const QString &error);

protected:
    HTMLTokenizerPrivate *d_ptr;
};

#endif // HTMLTOKENIZER_H
