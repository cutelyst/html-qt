#include "htmltokenizer_p.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDebug>

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

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

    while (CALL_MEMBER_FN(*d, d->stateFn)()) {
        // dunno what to do here :)
    }
    qDebug() << "finished";
}

// https://html.spec.whatwg.org/multipage/syntax.html#data-state
bool HTMLTokenizerPrivate::dataState()
{
    Q_Q(HTMLTokenizer);

    QChar data;
    *stream >> data;
    if (data == QLatin1Char('&')) {
        state = HTMLTokenizer::CharacterReferenceInDataState;
        stateFn = &HTMLTokenizerPrivate::characterReferenceInDataState;
    } else if (data == QLatin1Char('<')) {
        state = HTMLTokenizer::TagOpenState;
        stateFn = &HTMLTokenizerPrivate::tagOpenState;
    } else if (data.isNull()) {
        state = HTMLTokenizer::TagOpenState;
        tokenQueue.append(qMakePair<QString,QString>("ParseError", "invalid-codepoint"));
        tokenQueue.append(qMakePair<QString,QString>("Characters", "\u0000"));
        Q_EMIT q->character(data);
    } else if (stream->status() != QTextStream::Ok) {
        // Tokenization ends.
        return false;
    } else {
        tokenQueue.append(qMakePair<QString,QString>("Characters", data));
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
    if (data == QLatin1Char('!')) {
        state = HTMLTokenizer::MarkupDeclarationOpenState;
        stateFn = &HTMLTokenizerPrivate::markupDeclarationOpenState;
    } else if (data == QLatin1Char('/')) {
        state = HTMLTokenizer::EndTagOpenState;
        stateFn = &HTMLTokenizerPrivate::endTagOpenState;
    } else if (data >= 0x0041 && data <= 0x005A) {
        state = HTMLTokenizer::TagNameState;
        stateFn = &HTMLTokenizerPrivate::tagNameState;
        // We could just add 0x0020
        currentTokenName = data + 0x0020;
        currentTokenSelfClosing = false;
        currentTokenSelfClosingAcknowledged = false;
    } else if (data >= 0x0061 && data <= 0x007A) {
        state = HTMLTokenizer::TagNameState;
        stateFn = &HTMLTokenizerPrivate::tagNameState;
        currentTokenName = data;
        currentTokenSelfClosing = false;
        currentTokenSelfClosingAcknowledged = false;
    } else if (data == QLatin1Char('/')) {
        q->parserError(QStringLiteral("expected-tag-name-but-got-question-mark"));
        state = HTMLTokenizer::BogusCommentState;
        stateFn = &HTMLTokenizerPrivate::bogusCommentState;
    } else {
        state = HTMLTokenizer::DataState;
        stateFn = &HTMLTokenizerPrivate::dataState;
        // LESS-THAN SIGN <
        q->character(0x003C);
        stream->seek(initalPos);
    }

    return true;
}

bool HTMLTokenizerPrivate::tagNameState()
{

}

bool HTMLTokenizerPrivate::markupDeclarationOpenState()
{

}

bool HTMLTokenizerPrivate::endTagOpenState()
{

}

// https://html.spec.whatwg.org/multipage/syntax.html#consume-a-character-reference
QString HTMLTokenizerPrivate::consumeEntity(QChar *allowedChar)
{
    Q_Q(HTMLTokenizer);

    qint64 origPos = stream->pos();
    QString output = QStringLiteral("&");

    QChar data;
    *stream >> data;
    if (data == QChar::Tabulation ||
            data == QChar::LineFeed ||
            data == 0x000C || // Form Feed (FF)
            data == QChar::Space ||
            data == 0x003C || // Less-Than sign
            data == 0x0026 || // Ampersand
            stream->status() != QTextStream::Ok || // EOF
            (allowedChar && data == *allowedChar)) {
        // Not a character reference. No characters are consumed,
        // and nothing is returned. (This is not an error, either.)
        stream->seek(origPos);
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
            stream->seek(origPos);
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
        while (((c >= 0x0030 && c <= 0x39) || // Zero (0) to Nine (9)
                (c >= 0x0041 && c <= 0x0046) || // Latin A to Latin F
                (c >= 0x0061 && c <= 0x0066)) && // Latin a to Latin f
               !stream->atEnd()) {
            charStack.append(c); // store the position to rewind for ;
            lastPos = stream->pos();
            *stream >> c;
        }
    } else {
        while (c >= 0x0030 && c <= 0x39 && // Zero (0) to Nine (9)
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
        ret = 0xFFFD;
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
