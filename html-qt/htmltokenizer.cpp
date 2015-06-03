#include "htmltokenizer_p.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaEnum>
#include <QFile>
#include <QDebug>

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

#define IS_STREAM_EOF(stream) (stream->status() != QTextStream::Ok)

#define IS_ASCII_UPPERCASE(c) ('A' <= c && c <= 'Z')
#define IS_ASCII_LOWERCASE(c) ('a' <= c && c <= 'z')
#define IS_ASCII_DIGITS(c) ('0' <= c && c <= '9')
#define IS_ASCII_HEX_DIGITS(c) (IS_ASCII_DIGITS(c) || \
    ('A' <= c && c <= 'F') || \
    ('a' <= c && c <= 'f'))
#define IS_SPACE_CHARACTER(c) (data == QChar::Tabulation || /* CHARACTER TABULATION (tab) */ \
    data == QChar::LineFeed || /* LINE FEED (LF) */ \
    data == 0x000C || /* FORM FEED (FF) */ \
    data == QChar::Space) // SPACE

HTMLTokenizer::HTMLTokenizer() : d_ptr(new HTMLTokenizerPrivate)
{
    d_ptr->q_ptr = this;

    // TODO https://html.spec.whatwg.org/multipage/entities.json
    // get from the url and/or keep a local copy
    QFile entitiesFile("/home/daniel/code/html-qt/entities.json");
    if (!entitiesFile.open(QFile::ReadOnly)) {
        return;
    }
    QJsonDocument entities = QJsonDocument::fromBinaryData(entitiesFile.readAll());
    qDebug() << entities.object();
}

HTMLTokenizer::~HTMLTokenizer()
{
    delete d_ptr;
}

void HTMLTokenizer::setTextStream(QTextStream *stream)
{
    Q_D(HTMLTokenizer);
    d->stream = stream;
}

HTMLTokenizer::State HTMLTokenizer::state() const
{
    Q_D(const HTMLTokenizer);
    return d->state;
}

void HTMLTokenizer::start()
{
    Q_D(HTMLTokenizer);
    if (!d->stream) {
        qCritical() << "No text stream set";
        return;
    }

    qint64 lastPos = d->stream->pos();
    int repeatedPos = 0;
    while (CALL_MEMBER_FN(*d, d->stateFn)() && !d->stream->atEnd()) {
        // dunno what to do here :)
        qDebug() << d->stream->pos() << metaObject()->enumerator(0).key(d->state) << d->stream->atEnd();
        if (lastPos == d->stream->pos()) {
            if (++repeatedPos > 10) {
                qFatal("Infinite loop detected on state: %s, at position: %lld",
                       metaObject()->enumerator(0).key(d->state),
                       lastPos);
            }
        } else {
            lastPos = d->stream->pos();
            repeatedPos = 0;
        }
    }
    qDebug() << "finished";
}

// https://html.spec.whatwg.org/multipage/syntax.html#data-state
bool HTMLTokenizerPrivate::dataState()
{
    Q_Q(HTMLTokenizer);

    QChar data;
    *stream >> data;
    if (data == '&') {
        state = HTMLTokenizer::CharacterReferenceInDataState;
        stateFn = &HTMLTokenizerPrivate::characterReferenceInDataState;
    } else if (data == '<') {
        state = HTMLTokenizer::TagOpenState;
        stateFn = &HTMLTokenizerPrivate::tagOpenState;
    } else if (data.isNull()) {
        state = HTMLTokenizer::TagOpenState;
        Q_EMIT q->parserError("invalid-codepoint");
        Q_EMIT q->character(data);
    } else if (IS_STREAM_EOF(stream)) {
        // Tokenization ends.
        return false;
    } else {
        Q_EMIT q->character(data);
    }

    return true;
}

// https://html.spec.whatwg.org/multipage/syntax.html#character-reference-in-data-state
bool HTMLTokenizerPrivate::characterReferenceInDataState()
{
    Q_Q(HTMLTokenizer);

    QString ret = consumeEntity();
    if (ret.isNull()) {
        q->character(0x0026); // U+0026 AMPERSAND character (&) token
    } else {
        q->characterString(ret);
    }
    state = HTMLTokenizer::DataState;
    stateFn = &HTMLTokenizerPrivate::dataState;
    return true;
}

// https://html.spec.whatwg.org/multipage/syntax.html#tag-open-state
bool HTMLTokenizerPrivate::tagOpenState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;
    if (data == '!') {
        state = HTMLTokenizer::MarkupDeclarationOpenState;
        stateFn = &HTMLTokenizerPrivate::markupDeclarationOpenState;
    } else if (data == '/') {
        state = HTMLTokenizer::EndTagOpenState;
        stateFn = &HTMLTokenizerPrivate::endTagOpenState;
    } else if (IS_ASCII_UPPERCASE(data)) {
        state = HTMLTokenizer::TagNameState;
        stateFn = &HTMLTokenizerPrivate::tagNameState;
        currentToken = new HTMLToken(HTMLToken::StartTagToken);
        currentToken->name = data + 0x0020;
    } else if (IS_ASCII_LOWERCASE(data)) {
        state = HTMLTokenizer::TagNameState;
        stateFn = &HTMLTokenizerPrivate::tagNameState;
        currentToken = new HTMLToken(HTMLToken::StartTagToken);
        currentToken->name = data;
    } else if (data == '?') {
        q->parserError(QStringLiteral("expected-tag-name-but-got-question-mark"));
        state = HTMLTokenizer::BogusCommentState;
        stateFn = &HTMLTokenizerPrivate::bogusCommentState;
    } else {
        q->parserError(QStringLiteral("expected-tag-name"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        q->character(0x003C); // LESS-THAN SIGN <
        stream->seek(initalPos);
    }

    return true;
}

// https://html.spec.whatwg.org/multipage/syntax.html#end-tag-open-state
bool HTMLTokenizerPrivate::endTagOpenState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    if (IS_ASCII_UPPERCASE(data)) {
        currentToken = new HTMLToken(HTMLToken::EndTagToken);
        currentToken->name = data + 0x0020;
        currentToken->selfClosing = false;
        state = HTMLTokenizer::TagNameState;
        stateFn = &HTMLTokenizerPrivate::tagNameState;
    } else if (IS_ASCII_LOWERCASE(data)) {
        currentToken = new HTMLToken(HTMLToken::EndTagToken);
        currentToken->name = data + 0x0020;
        currentToken->selfClosing = false;
        state = HTMLTokenizer::TagNameState;
        stateFn = &HTMLTokenizerPrivate::tagNameState;
    } else if (data == '>') {
        Q_EMIT q->parserError(QStringLiteral("expected-closing-tag-but-got-right-bracket"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("expected-closing-tag-but-got-eof"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        // 0x003C and // 0x002F
        Q_EMIT q->characterString(QStringLiteral("</"));
        stream->seek(initalPos);
    } else {
        Q_EMIT q->parserError(QStringLiteral("expected-closing-tag-but-got-char"));
        state = HTMLTokenizer::BogusCommentState;
        stateFn = &HTMLTokenizerPrivate::bogusCommentState;
    }

    return true;
}

bool HTMLTokenizerPrivate::tagNameState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;
    if (IS_SPACE_CHARACTER(data)) {
        state = HTMLTokenizer::BeforeAttributeNameState;
        stateFn = &HTMLTokenizerPrivate::beforeAttributeNameState;
    } else if (data == '/') {
        state = HTMLTokenizer::SelfClosingStartTagState;
        stateFn = &HTMLTokenizerPrivate::selfClosingStartTagState;
    } else if (data == '>') {
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_ASCII_UPPERCASE(data)) {
        // Appending the lower case version
        currentToken->name.append(data + 0x0020);
    } else if (data.isNull()) {
        Q_EMIT q->parserError(QStringLiteral("invalid-codepoint"));
        currentToken->name.append(QChar::ReplacementCharacter);
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-tag-name"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        stream->seek(initalPos);
    } else {
        currentToken->name.append(data);
    }

    return true;
}

bool HTMLTokenizerPrivate::beforeAttributeNameState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    // Ignore all space characters
    while (IS_SPACE_CHARACTER(data)) {
        initalPos = stream->pos();
        *stream >> data;
    }

    qDebug() << "beforeAttributeNameState" << data;
    if (data == '/') {
        state = HTMLTokenizer::SelfClosingStartTagState;
        stateFn = &HTMLTokenizerPrivate::selfClosingStartTagState;
    } else if (data == '>') {
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_ASCII_UPPERCASE(data)) {
        // Appending the lower case version
        currentToken->data.append(qMakePair(data + 0x0020, QString()));
        state = HTMLTokenizer::AttributeNameState;
        stateFn = &HTMLTokenizerPrivate::attributeNameState;
    } else if (data.isNull()) {
        Q_EMIT q->parserError(QStringLiteral("invalid-codepoint"));
        currentToken->data.append(qMakePair(QChar::ReplacementCharacter, QString()));
        state = HTMLTokenizer::AttributeNameState;
        stateFn = &HTMLTokenizerPrivate::attributeNameState;
    } else if (data == '"' ||
               data == '\'' ||
               data == '<' ||
               data == '=') {
        Q_EMIT q->parserError(QStringLiteral("invalid-character-in-attribute-name"));
        currentToken->data.append(qMakePair(data, QString()));
        state = HTMLTokenizer::AttributeNameState;
        stateFn = &HTMLTokenizerPrivate::attributeNameState;
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("expected-attribute-name-but-got-eof"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        stream->seek(initalPos);
    } else {
        currentToken->data.append(qMakePair(data, QString()));
        state = HTMLTokenizer::AttributeNameState;
        stateFn = &HTMLTokenizerPrivate::attributeNameState;
    }

    return true;
}

bool HTMLTokenizerPrivate::attributeNameState()
{
    Q_Q(HTMLTokenizer);

//    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

//    bool leavingThisState = true;
//    bool emitToken = false;
    if (IS_SPACE_CHARACTER(data)) {
        state = HTMLTokenizer::AfterAttributeNameState;
        stateFn = &HTMLTokenizerPrivate::afterAttributeNameState;
    } else if (data == '/') {
        state = HTMLTokenizer::SelfClosingStartTagState;
        stateFn = &HTMLTokenizerPrivate::selfClosingStartTagState;
    } else if (data == '=') {
        state = HTMLTokenizer::BeforeAttributeValueState;
        stateFn = &HTMLTokenizerPrivate::beforeAttributeValueState;
    } else if (data == '>') {
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_ASCII_UPPERCASE(data)) {

    } else if (data.isNull()) {
        state = HTMLTokenizer::BeforeAttributeValueState;
        stateFn = &HTMLTokenizerPrivate::beforeAttributeValueState;
    } else if (data == '"' ||
               data == '\'' ||
               data == '<') {

    } else if (IS_STREAM_EOF(stream)) {

    } else {

    }

    return true;
}

bool HTMLTokenizerPrivate::afterAttributeNameState()
{
    return true;
}

bool HTMLTokenizerPrivate::beforeAttributeValueState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    // Ignore all space characters
    while (IS_SPACE_CHARACTER(data)) {
        initalPos = stream->pos();
        *stream >> data;
    }

    if (data == '"') {
        state = HTMLTokenizer::AttributeValueDoubleQuotedState;
        stateFn = &HTMLTokenizerPrivate::attributeValueDoubleQuotedState;
    } else if (data == '&') {
        state = HTMLTokenizer::AttributeValueUnquotedState;
        stateFn = &HTMLTokenizerPrivate::attributeValueUnquotedState;
    } else if (data == '\'') {
        state = HTMLTokenizer::AttributeValueSingleQuotedState;
        stateFn = &HTMLTokenizerPrivate::attributeValueSingleQuotedState;
    } else if (data.isNull()) {
        Q_EMIT q->parserError(QStringLiteral("expected-attribute-value-but-got-right-bracket"));
        emitCurrentTagToken();
    } else if (data == '>') {
        Q_EMIT q->parserError(QStringLiteral("expected-attribute-value-but-got-right-bracket"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (data == '<' || data == '=' || data == '`') {
        Q_EMIT q->parserError(QStringLiteral("equals-in-unquoted-attribute-value"));
        currentToken->appendDataCurrentAttributeValue(data);
        state = HTMLTokenizer::AttributeValueUnquotedState;
        stateFn = &HTMLTokenizerPrivate::attributeValueUnquotedState;
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("expected-attribute-value-but-got-eof"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        stream->seek(initalPos);
    } else {
        currentToken->appendDataCurrentAttributeValue(data);
        state = HTMLTokenizer::AttributeValueUnquotedState;
        stateFn = &HTMLTokenizerPrivate::attributeValueUnquotedState;
    }

    return true;
}

bool HTMLTokenizerPrivate::attributeValueDoubleQuotedState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    if (data == '"') {
        state = HTMLTokenizer::AfterAttributeValueQuotedState;
        stateFn = &HTMLTokenizerPrivate::afterAttributeValueQuotedState;
    } else if (data == '&') {
        // TODO process next "
        state = HTMLTokenizer::CharacterReferenceInAttributeValueState;
        stateFn = &HTMLTokenizerPrivate::characterReferenceInAttributeValueState;
    } else if (data.isNull()) {
        Q_EMIT q->parserError(QStringLiteral("invalid-codepoint"));
        currentToken->appendDataCurrentAttributeValue(QChar::ReplacementCharacter);
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-attribute-value-double-quote"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        stream->seek(initalPos);
    } else {
        currentToken->appendDataCurrentAttributeValue(data);
    }

    return true;
}

bool HTMLTokenizerPrivate::attributeValueSingleQuotedState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    if (data == '\'') {
        state = HTMLTokenizer::AfterAttributeValueQuotedState;
        stateFn = &HTMLTokenizerPrivate::afterAttributeValueQuotedState;
    } else if (data == '&') {
        // TODO process next '
        state = HTMLTokenizer::CharacterReferenceInAttributeValueState;
        stateFn = &HTMLTokenizerPrivate::characterReferenceInAttributeValueState;
    } else if (data.isNull()) {
        Q_EMIT q->parserError(QStringLiteral("invalid-codepoint"));
        currentToken->appendDataCurrentAttributeValue(QChar::ReplacementCharacter);
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-attribute-value-single-quote"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        stream->seek(initalPos);
    } else {
        currentToken->appendDataCurrentAttributeValue(data);
    }

    return true;
}

bool HTMLTokenizerPrivate::attributeValueUnquotedState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    if (IS_SPACE_CHARACTER(data)) {
        state = HTMLTokenizer::BeforeAttributeNameState;
        stateFn = &HTMLTokenizerPrivate::beforeAttributeNameState;
    } else if (data == '&') {
        // TODO process next >
        state = HTMLTokenizer::CharacterReferenceInAttributeValueState;
        stateFn = &HTMLTokenizerPrivate::characterReferenceInAttributeValueState;
    } else if (data == '>') {
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (data.isNull()) {
        Q_EMIT q->parserError(QStringLiteral("invalid-codepoint"));
        currentToken->appendDataCurrentAttributeValue(QChar::ReplacementCharacter);
    } else if (data == '"' || data == '\'' || data == '<' || data == '`') {
        Q_EMIT q->parserError(QStringLiteral("unexpected-character-in-unquoted-attribute-value"));
        currentToken->appendDataCurrentAttributeValue(data);
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-attribute-value-no-quotes"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        stream->seek(initalPos);
    } else {
        currentToken->appendDataCurrentAttributeValue(data);
    }

    return true;
}

bool HTMLTokenizerPrivate::characterReferenceInAttributeValueState()
{
    return true;
}

bool HTMLTokenizerPrivate::afterAttributeValueQuotedState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    if (IS_SPACE_CHARACTER(data)) {
        state = HTMLTokenizer::BeforeAttributeNameState;
        stateFn = &HTMLTokenizerPrivate::beforeAttributeNameState;
    } else if (data == '/') {
        state = HTMLTokenizer::SelfClosingStartTagState;
        stateFn = &HTMLTokenizerPrivate::selfClosingStartTagState;
    } else if (data == '>') {
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("unexpected-EOF-after-attribute-value"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        stream->seek(initalPos);
    } else {
        Q_EMIT q->parserError(QStringLiteral("unexpected-character-after-attribute-value"));
        state = HTMLTokenizer::BeforeAttributeNameState;
        stateFn = &HTMLTokenizerPrivate::beforeAttributeNameState;
        stream->seek(initalPos);
    }

    return true;
}

bool HTMLTokenizerPrivate::selfClosingStartTagState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    if (data == '>') {
        currentToken->selfClosing = true;
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("unexpected-EOF-after-solidus-in-tag"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        stream->seek(initalPos);
    } else {
        Q_EMIT q->parserError(QStringLiteral("unexpected-character-after-solidus-in-tag"));
        state = HTMLTokenizer::BeforeAttributeNameState;
        stateFn = &HTMLTokenizerPrivate::beforeAttributeNameState;
        stream->seek(initalPos);
    }

    return true;
}

bool HTMLTokenizerPrivate::bogusCommentState()
{
    return true;
}

// https://html.spec.whatwg.org/multipage/syntax.html#markup-declaration-open-state
bool HTMLTokenizerPrivate::markupDeclarationOpenState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;
    QString charStack = data;

    if (data == '-') {
        *stream >> data;
        charStack.append(data);
        if (data == '-') {
            currentToken = new HTMLToken(HTMLToken::CommentToken);
            state = HTMLTokenizer::CommentStartState;
            stateFn = &HTMLTokenizerPrivate::commentStartState;
            return true;
        }
    } else if (data == 'd' || data == 'D') {
        // consume more 6 chars
        for (int i = 0; i < 6; ++i) {
            *stream >> data;
            charStack.append(data);
        }

        if (charStack.compare(QLatin1String("DOCTYPE"), Qt::CaseInsensitive) == 0) {
//            currentToken = new HTMLToken(HTMLToken::CommentToken);
            qDebug() << "markupDeclarationOpenState" << charStack;
            state = HTMLTokenizer::DocTypeState;
            stateFn = &HTMLTokenizerPrivate::doctypeState;
            return true;
        }
    } else if (data == '[') {
        qWarning() << "markupDeclarationOpenState CDATA TODO";
    }

    Q_EMIT q->parserError(QStringLiteral("expected-dashes-or-doctype"));
    state = HTMLTokenizer::BogusCommentState;
    stateFn = &HTMLTokenizerPrivate::bogusCommentState;
    stream->seek(initalPos);

    return true;
}

bool HTMLTokenizerPrivate::commentStartState()
{
    return true;
}

bool HTMLTokenizerPrivate::commentStartDashState()
{
    return true;
}

bool HTMLTokenizerPrivate::commentState()
{
    return true;
}

bool HTMLTokenizerPrivate::commentEndBangState()
{
    return true;
}

// https://html.spec.whatwg.org/multipage/syntax.html#doctype-state
bool HTMLTokenizerPrivate::doctypeState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    if (IS_SPACE_CHARACTER(data)) {
        state = HTMLTokenizer::BeforeDocTypeNameState;
        stateFn = &HTMLTokenizerPrivate::beforeDocTypeNameState;
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("expected-doctype-name-but-got-eof"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken = new HTMLToken(HTMLToken::DocTypeToken);
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        Q_EMIT q->parserError(QStringLiteral("need-space-after-doctype"));
        state = HTMLTokenizer::BeforeDocTypeNameState;
        stateFn = &HTMLTokenizerPrivate::beforeDocTypeNameState;
        stream->seek(initalPos);
    }

    return true;
}

bool HTMLTokenizerPrivate::beforeDocTypeNameState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    // Ignore all space characters
    while (IS_SPACE_CHARACTER(data)) {
        initalPos = stream->pos();
        *stream >> data;
    }

    if (IS_ASCII_UPPERCASE(data)) {
        currentToken = new HTMLToken(HTMLToken::DocTypeToken);
        currentToken->name = data + 0x0020;
        state = HTMLTokenizer::DocTypeNameState;
        stateFn = &HTMLTokenizerPrivate::docTypeNameState;
    } else if (data.isNull()) {
        Q_EMIT q->parserError(QStringLiteral("invalid-codepoint"));
        currentToken = new HTMLToken(HTMLToken::DocTypeToken);
        currentToken->name = QChar(QChar::ReplacementCharacter);
        state = HTMLTokenizer::DocTypeNameState;
        stateFn = &HTMLTokenizerPrivate::docTypeNameState;
    } else if (data == '>') {
        Q_EMIT q->parserError(QStringLiteral("expected-doctype-name-but-got-right-bracket"  ));
        currentToken = new HTMLToken(HTMLToken::DocTypeToken);
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("expected-doctype-name-but-got-eof"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken = new HTMLToken(HTMLToken::DocTypeToken);
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        currentToken = new HTMLToken(HTMLToken::DocTypeToken);
        currentToken->name = data;
        state = HTMLTokenizer::DocTypeNameState;
        stateFn = &HTMLTokenizerPrivate::docTypeNameState;
    }

    return true;
}

bool HTMLTokenizerPrivate::docTypeNameState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();

    QChar data;
    *stream >> data;
    if (IS_SPACE_CHARACTER(data)) {
        state = HTMLTokenizer::AfterDocTypeNameState;
        stateFn = &HTMLTokenizerPrivate::afterDocTypeNameState;
    } else if (data == '>') {
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_ASCII_UPPERCASE(data)) {
        currentToken->name.append(data + 0x0020);
    } else if (data.isNull()) {
        Q_EMIT q->parserError(QStringLiteral("invalid-codepoint"));
        currentToken->name.append(QChar::ReplacementCharacter);
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-doctype-name"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken = new HTMLToken(HTMLToken::DocTypeToken);
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        currentToken->name.append(data);
    }

    return true;
}

// https://html.spec.whatwg.org/multipage/syntax.html#after-doctype-name-state
bool HTMLTokenizerPrivate::afterDocTypeNameState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    // Ignore all space characters
    while (IS_SPACE_CHARACTER(data)) {
        initalPos = stream->pos();
        *stream >> data; // Ignore the character.
    }

    if (data == '>') {
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-doctype"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        initalPos = stream->pos();
        if (data == 'p' || data == 'P' ||
                data == 's' || data == 'S') {
            QString charStack = data;
            // consume more 5 chars
            for (int i = 0; i < 5; ++i) {
                *stream >> data;
                charStack.append(data);
            }

            if (charStack.compare(QLatin1String("PUBLIC"), Qt::CaseInsensitive) == 0) {
                state = HTMLTokenizer::AfterDocTypePublicKeywordState;
                stateFn = &HTMLTokenizerPrivate::afterDocTypePublicKeywordState;
                return true;
            } else if (charStack.compare(QLatin1String("SYSTEM"), Qt::CaseInsensitive) == 0) {
                state = HTMLTokenizer::AfterDocTypeSystemKeywordState;
                stateFn = &HTMLTokenizerPrivate::afterDocTypeSystemKeywordState;
                return true;
            }
        }

        Q_EMIT q->parserError(QStringLiteral("expected-space-or-right-bracket-in-doctype"));
        state = HTMLTokenizer::BogusDocTypeState;
        stateFn = &HTMLTokenizerPrivate::bogusDocTypeState;
        currentToken->forceQuirks = true;
        stream->seek(initalPos);
    }

    return true;
}

bool HTMLTokenizerPrivate::afterDocTypePublicKeywordState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();

    QChar data;
    *stream >> data;

    if (IS_SPACE_CHARACTER(data)) {
        state = HTMLTokenizer::BeforeDocTypePublicIdentifierState;
        stateFn = &HTMLTokenizerPrivate::beforeDocTypePublicIdentifierState;
    } else if (data == '"') {
        Q_EMIT q->parserError(QStringLiteral("unexpected-double-quote-in-doctype"));
        currentToken->doctypePublicId = "";
        state = HTMLTokenizer::DocTypePublicIdentifierDoubleQuotedState;
        stateFn = &HTMLTokenizerPrivate::docTypePublicIdentifierDoubleQuotedState;
    } else if (data == '\'') {
        Q_EMIT q->parserError(QStringLiteral("unexpected-single-quote-in-doctype"));
        currentToken->doctypePublicId = "";
        state = HTMLTokenizer::DocTypePublicIdentifierSingleQuotedState;
        stateFn = &HTMLTokenizerPrivate::docTypePublicIdentifierSingleQuotedState;
    } else if (data == '>') {
        Q_EMIT q->parserError(QStringLiteral("unexpected-single-quote-in-doctype"));
        currentToken->forceQuirks = true;
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-doctype"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        Q_EMIT q->parserError(QStringLiteral("unexpected-char-in-doctype"));
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        state = HTMLTokenizer::BogusDocTypeState;
        stateFn = &HTMLTokenizerPrivate::bogusDocTypeState;
    }

    return true;
}

bool HTMLTokenizerPrivate::beforeDocTypePublicIdentifierState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    // Ignore all space characters
    while (IS_SPACE_CHARACTER(data)) {
        initalPos = stream->pos();
        *stream >> data;
    }

    if (data == '"') {
        currentToken->doctypePublicId = "";
        state = HTMLTokenizer::DocTypePublicIdentifierDoubleQuotedState;
        stateFn = &HTMLTokenizerPrivate::docTypePublicIdentifierDoubleQuotedState;
    } else if (data == '\'') {
        currentToken->doctypePublicId = "";
        state = HTMLTokenizer::DocTypePublicIdentifierSingleQuotedState;
        stateFn = &HTMLTokenizerPrivate::docTypePublicIdentifierSingleQuotedState;
    } else if (data == '>') {
        Q_EMIT q->parserError(QStringLiteral("unexpected-end-of-doctype"));
        currentToken->forceQuirks = true;
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-doctype"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        Q_EMIT q->parserError(QStringLiteral("unexpected-char-in-doctype"));
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        state = HTMLTokenizer::BogusDocTypeState;
        stateFn = &HTMLTokenizerPrivate::bogusDocTypeState;
    }

    return true;
}

bool HTMLTokenizerPrivate::docTypePublicIdentifierDoubleQuotedState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    if (data == '"') {
        state = HTMLTokenizer::AfterDocTypePublicIdentifierState;
        stateFn = &HTMLTokenizerPrivate::afterDocTypePublicIdentifierState;
    } else if (data.isNull()) {
        Q_EMIT q->parserError(QStringLiteral("invalid-codepoint"));
        currentToken->name.append(QChar::ReplacementCharacter);
    } else if (data == '>') {
        Q_EMIT q->parserError(QStringLiteral("unexpected-end-of-doctype"));
        currentToken->forceQuirks = true;
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-doctype"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        currentToken->doctypePublicId.append(data);
    }

    return true;
}

bool HTMLTokenizerPrivate::docTypePublicIdentifierSingleQuotedState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    if (data == '\'') {
        state = HTMLTokenizer::AfterDocTypePublicIdentifierState;
        stateFn = &HTMLTokenizerPrivate::afterDocTypePublicIdentifierState;
    } else if (data.isNull()) {
        Q_EMIT q->parserError(QStringLiteral("invalid-codepoint"));
        currentToken->name.append(QChar::ReplacementCharacter);
    } else if (data == '>') {
        Q_EMIT q->parserError(QStringLiteral("unexpected-end-of-doctype"));
        currentToken->forceQuirks = true;
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-doctype"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        currentToken->doctypePublicId.append(data);
    }

    return true;
}

bool HTMLTokenizerPrivate::afterDocTypePublicIdentifierState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    if (IS_SPACE_CHARACTER(data)) {
        state = HTMLTokenizer::BetweenDocTypePublicAndSystemIdentifierState;
        stateFn = &HTMLTokenizerPrivate::betweenDocTypePublicAndSystemIdentifierState;
    } else if (data == '>') {
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (data == '"') {
        Q_EMIT q->parserError(QStringLiteral("unexpected-char-in-doctype"));
        currentToken->doctypeSystemId = "";
        state = HTMLTokenizer::DocTypeSystemIdentifierDoubleQuotedState;
        stateFn = &HTMLTokenizerPrivate::docTypeSystemIdentifierDoubleQuotedState;
    } else if (data == '\'') {
        Q_EMIT q->parserError(QStringLiteral("unexpected-char-in-doctype"));
        currentToken->doctypeSystemId = "";
        state = HTMLTokenizer::DocTypeSystemIdentifierSingleQuotedState;
        stateFn = &HTMLTokenizerPrivate::docTypeSystemIdentifierSingleQuotedState;
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-doctype"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        q->parserError(QStringLiteral("unexpected-char-in-doctype"));
        currentToken->forceQuirks = true;
        state = HTMLTokenizer::BogusDocTypeState;
        stateFn = &HTMLTokenizerPrivate::bogusDocTypeState;
    }

    return true;
}

bool HTMLTokenizerPrivate::betweenDocTypePublicAndSystemIdentifierState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    // Ignore all space characters
    while (IS_SPACE_CHARACTER(data)) {
        initalPos = stream->pos();
        *stream >> data;
    }

    if (data == '>') {
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (data == '"') {
        currentToken->doctypeSystemId = "";
        state = HTMLTokenizer::DocTypeSystemIdentifierDoubleQuotedState;
        stateFn = &HTMLTokenizerPrivate::docTypeSystemIdentifierDoubleQuotedState;
    } else if (data == '\'') {
        currentToken->doctypeSystemId = "";
        state = HTMLTokenizer::DocTypeSystemIdentifierSingleQuotedState;
        stateFn = &HTMLTokenizerPrivate::docTypeSystemIdentifierSingleQuotedState;
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-doctype"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        q->parserError(QStringLiteral("unexpected-char-in-doctype"));
        currentToken->forceQuirks = true;
        state = HTMLTokenizer::BogusDocTypeState;
        stateFn = &HTMLTokenizerPrivate::bogusDocTypeState;
    }

    return true;
}

bool HTMLTokenizerPrivate::afterDocTypeSystemKeywordState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    if (IS_SPACE_CHARACTER(data)) {
        state = HTMLTokenizer::BeforeDocTypeSystemIdentifierState;
        stateFn = &HTMLTokenizerPrivate::beforeDocTypeSystemIdentifierState;
    } else if (data == '>') {
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (data == '"') {
        Q_EMIT q->parserError(QStringLiteral("unexpected-char-in-doctype"));
        currentToken->doctypeSystemId = "";
        state = HTMLTokenizer::DocTypeSystemIdentifierDoubleQuotedState;
        stateFn = &HTMLTokenizerPrivate::docTypeSystemIdentifierDoubleQuotedState;
    } else if (data == '\'') {
        Q_EMIT q->parserError(QStringLiteral("unexpected-char-in-doctype"));
        currentToken->doctypeSystemId = "";
        state = HTMLTokenizer::DocTypeSystemIdentifierSingleQuotedState;
        stateFn = &HTMLTokenizerPrivate::docTypeSystemIdentifierSingleQuotedState;
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-doctype"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        q->parserError(QStringLiteral("unexpected-char-in-doctype"));
        currentToken->forceQuirks = true;
        state = HTMLTokenizer::BogusDocTypeState;
        stateFn = &HTMLTokenizerPrivate::bogusDocTypeState;
    }

    return true;
}

bool HTMLTokenizerPrivate::beforeDocTypeSystemIdentifierState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    // Ignore all space characters
    while (IS_SPACE_CHARACTER(data)) {
        initalPos = stream->pos();
        *stream >> data;
    }

    if (data == '"') {
        currentToken->doctypeSystemId = "";
        state = HTMLTokenizer::DocTypeSystemIdentifierDoubleQuotedState;
        stateFn = &HTMLTokenizerPrivate::docTypeSystemIdentifierDoubleQuotedState;
    } else if (data == '\'') {
        currentToken->doctypeSystemId = "";
        state = HTMLTokenizer::DocTypeSystemIdentifierSingleQuotedState;
        stateFn = &HTMLTokenizerPrivate::docTypeSystemIdentifierSingleQuotedState;
    } else if (data == '>') {
        Q_EMIT q->parserError(QStringLiteral("unexpected-char-in-doctype"));
        currentToken->forceQuirks = true;
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-doctype"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        Q_EMIT q->parserError(QStringLiteral("unexpected-char-in-doctype"));
        currentToken->forceQuirks = true;
        state = HTMLTokenizer::BogusDocTypeState;
        stateFn = &HTMLTokenizerPrivate::bogusDocTypeState;
    }

    return true;
}

bool HTMLTokenizerPrivate::docTypeSystemIdentifierDoubleQuotedState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    if (data == '"') {
        state = HTMLTokenizer::AfterDocTypeSystemIdentifierState;
        stateFn = &HTMLTokenizerPrivate::afterDocTypeSystemIdentifierState;
    } else if (data.isNull()) {
        Q_EMIT q->parserError(QStringLiteral("invalid-codepoint"));
        currentToken->doctypeSystemId.append(QChar::ReplacementCharacter);
        state = HTMLTokenizer::BeforeDocTypeSystemIdentifierState;
        stateFn = &HTMLTokenizerPrivate::beforeDocTypeSystemIdentifierState;
    } else if (data == '>') {
        Q_EMIT q->parserError(QStringLiteral("unexpected-end-of-doctype"));
        currentToken->forceQuirks = true;
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-doctype"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        currentToken->doctypeSystemId.append(data);
    }

    return true;
}

bool HTMLTokenizerPrivate::docTypeSystemIdentifierSingleQuotedState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    if (data == '\'') {
        state = HTMLTokenizer::AfterDocTypeSystemIdentifierState;
        stateFn = &HTMLTokenizerPrivate::afterDocTypeSystemIdentifierState;
    } else if (data.isNull()) {
        Q_EMIT q->parserError(QStringLiteral("invalid-codepoint"));
        currentToken->doctypeSystemId.append(QChar::ReplacementCharacter);
        state = HTMLTokenizer::BeforeDocTypeSystemIdentifierState;
        stateFn = &HTMLTokenizerPrivate::beforeDocTypeSystemIdentifierState;
    } else if (data == '>') {
        Q_EMIT q->parserError(QStringLiteral("unexpected-end-of-doctype"));
        currentToken->forceQuirks = true;
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-doctype"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        currentToken->doctypeSystemId.append(data);
    }

    return true;
}

bool HTMLTokenizerPrivate::afterDocTypeSystemIdentifierState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    // Ignore all space characters
    while (IS_SPACE_CHARACTER(data)) {
        initalPos = stream->pos();
        *stream >> data;
    }

    if (data == '>') {
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_STREAM_EOF(stream)) {
        Q_EMIT q->parserError(QStringLiteral("eof-in-doctype"));
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        currentToken->forceQuirks = true;
        emitCurrentTagToken();
        stream->seek(initalPos);
    } else {
        q->parserError(QStringLiteral("unexpected-char-in-doctype"));
        currentToken->forceQuirks = true;
        state = HTMLTokenizer::BogusDocTypeState;
        stateFn = &HTMLTokenizerPrivate::bogusDocTypeState;
    }

    return true;
}

bool HTMLTokenizerPrivate::bogusDocTypeState()
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QChar data;
    *stream >> data;

    if (data == '>') {
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
    } else if (IS_STREAM_EOF(stream)) {
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        emitCurrentTagToken();
        stream->seek(initalPos);
    }

    return true;
}

bool HTMLTokenizerPrivate::cDataSectionState()
{
    return true;
}

// https://html.spec.whatwg.org/multipage/syntax.html#consume-a-character-reference
QString HTMLTokenizerPrivate::consumeEntity(QChar *allowedChar)
{
    Q_Q(HTMLTokenizer);

    qint64 initalPos = stream->pos();
    QString output = QStringLiteral("&");

    QChar data;
    *stream >> data;
    if (IS_SPACE_CHARACTER(data) ||
            data == 0x003C || // Less-Than sign
            data == 0x0026 || // Ampersand
            IS_STREAM_EOF(stream) || // EOF
            (allowedChar && data == *allowedChar)) {
        // Not a character reference. No characters are consumed,
        // and nothing is returned. (This is not an error, either.)
        stream->seek(initalPos);
        return QString();
    } else if (data == 0x0023) { // Number sign (#)
        output.append(data);

        *stream >> data;
        QChar number;
        if (data == 0x0078 || // Latin small letter X
                data == 0x0058) { // Latin capital letter X
            number = consumeNumberEntity(true);
        } else {
            number = consumeNumberEntity(false);
        }

        if (number.isNull()) {
            q->parserError(QStringLiteral("expected-numeric-entity"));
            // unconsume all characters
            stream->seek(initalPos);
            return QString();
        }

        return number;
    } else {

    }
    return QString();
}

QChar HTMLTokenizerPrivate::consumeNumberEntity(bool isHex)
{
    Q_Q(HTMLTokenizer);

    QChar ret;
    QString charStack;
    QChar c;
    qint64 lastPos = stream->pos();
    *stream >> c;
    if (isHex) {
        while (IS_ASCII_HEX_DIGITS(c) &&
               !stream->atEnd()) {
            charStack.append(c); // store the position to rewind for ;
            lastPos = stream->pos();
            *stream >> c;
        }
    } else {
        while (IS_ASCII_DIGITS(c) && // Zero (0) to Nine (9)
               !stream->atEnd()) {
            charStack.append(c);
            lastPos = stream->pos(); // store the position to rewind for ;
            *stream >> c;
        }
    }

    // No char was found return null to unconsume
    if (charStack.isNull()) {
        return QChar::Null;
    }

    // Discard the ; if present. Otherwise, put it back on the queue and
    // invoke parseError on parser.
    if (c != 0x003B) {
        q->parserError(QStringLiteral("numeric-entity-without-semicolon"));
        stream->seek(lastPos);
    }

    // Convert the number using the proper base
    bool ok;
    int charAsInt = charStack.toInt(&ok, isHex ? 16 : 10);
    if (!ok) {
        // TODO error
    }

    // Certain characters get replaced with others
    QMap<int,int>::ConstIterator it = replacementCharacters.constFind(charAsInt);
    if (it != replacementCharacters.constEnd()) {
        ret = it.value();
        q->parserError(QString("illegal-codepoint-for-numeric-entity: %1").arg(charStack));
    } else if ((charAsInt >= 0xD800 && charAsInt <= 0xDFFF) ||
               charAsInt > 0x10FFFF) {
        ret = QChar::ReplacementCharacter;
        q->parserError(QString("illegal-codepoint-for-numeric-entity: %1").arg(charStack));
    } else {
        if ((0x0001 <= charAsInt && charAsInt <= 0x0008) ||
                (0x000E <= charAsInt && charAsInt <= 0x001F) ||
                (0x007F <= charAsInt && charAsInt <= 0x009F) ||
                (0xFDD0 <= charAsInt && charAsInt <= 0xFDEF) ||
                (charAsInt == 0x000B || charAsInt == 0xFFFE || charAsInt == 0xFFFF || charAsInt == 0x1FFFE ||
                 charAsInt == 0x1FFFF || charAsInt == 0x2FFFE || charAsInt == 0x2FFFF || charAsInt == 0x3FFFE ||
                 charAsInt == 0x3FFFF || charAsInt == 0x4FFFE || charAsInt == 0x4FFFF || charAsInt == 0x5FFFE ||
                 charAsInt == 0x5FFFF || charAsInt == 0x6FFFE || charAsInt == 0x6FFFF || charAsInt == 0x7FFFE ||
                 charAsInt == 0x7FFFF || charAsInt == 0x8FFFE || charAsInt == 0x8FFFF || charAsInt == 0x9FFFE ||
                 charAsInt == 0x9FFFF || charAsInt == 0xAFFFE || charAsInt == 0xAFFFF || charAsInt == 0xBFFFE ||
                 charAsInt == 0xBFFFF || charAsInt == 0xCFFFE || charAsInt == 0xCFFFF || charAsInt == 0xDFFFE ||
                 charAsInt == 0xDFFFF || charAsInt == 0xEFFFE || charAsInt == 0xEFFFF || charAsInt == 0xFFFFE ||
                 charAsInt == 0xFFFFF || charAsInt == 0x10FFFE || charAsInt == 0x10FFFF)) {
            q->parserError(QString("illegal-codepoint-for-numeric-entity: %1").arg(charStack));
            ret = charAsInt;
        }
    }

    return ret;
}

void HTMLTokenizerPrivate::emitCurrentTagToken()
{
    qDebug() << "emitCurrentTagToken" << currentToken;
    currentToken = 0;
}
