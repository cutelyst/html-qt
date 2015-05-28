#include "htmltokenizer_p.h"

#include <QDebug>

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

HTMLTokenizer::HTMLTokenizer() : d_ptr(new HTMLTokenizerPrivate)
{
    d_ptr->q_ptr = this;
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



bool HTMLTokenizerPrivate::dataState()
{
    Q_Q(HTMLTokenizer);

    QChar data;
    *stream >> data;
    if (data == QLatin1Char('&')) {
        state = HTMLTokenizer::CharacterReferenceInDataState;
        stateFn = &HTMLTokenizerPrivate::entityDataState;
    } else if (data == QLatin1Char('<')) {
        state = HTMLTokenizer::TagOpenState;
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

bool HTMLTokenizerPrivate::entityDataState()
{

    state = HTMLTokenizer::DataState;
    stateFn = &HTMLTokenizerPrivate::dataState;
    return true;
}

void HTMLTokenizerPrivate::consumeEntity(QChar *allowedChar)
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
    } else if (data == 0x0023) { // Number sign (#)
        output.append(data);

        *stream >> data;
        QString number;
        if (data == 0x0078 || // Latin small letter X
                data == 0x0058) { // Latin capital letter X
            number = consumeHexDigits();
        } else {
            number = consumeDigits();
        }

        if (number.isNull()) {
            q->parserError(QStringLiteral("expected-numeric-entity"));
            // unconsume all characters
            stream->seek(origPos);
            return;
        }

        *stream >> data;
        if (data != 0x003B) { //Semicolon
            q->parserError(QStringLiteral("expected-numeric-entity"));
            // unconsume all characters
            stream->seek(origPos);
            return;
        }

    } else {

    }


}

QChar HTMLTokenizerPrivate::consumeNumberEntity(bool isHex)
{
    Q_Q(HTMLTokenizer);

    QChar ret;
    QString charStack;
    QChar c;
    if (isHex) {
        do {
            *stream >> c;
            // Zero (0) to Nine (9)
            if ((c >= 0x0030 && c <= 0x39) ||
                    (c >= 0x0041 && c <= 0x0046) || // Latin A to Latin F
                    (c >= 0x0061 && c <= 0x0066)) { // Latin a to Latin f
                charStack.append(c);
            } else {

            }
        } while (!stream->atEnd());
    } else {
        do {
            *stream >> c;
            // Zero (0) to Nine (9)
            if ((c >= 0x0030 && c <= 0x39)) {
                charStack.append(c);
            } else {

            }
        } while (!stream->atEnd());
    }
    qint64 lastPos = stream->pos();

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

    // Discard the ; if present. Otherwise, put it back on the queue and
    // invoke parseError on parser.
    if (c != 0x003B) {
        q->parserError(QStringLiteral("numeric-entity-without-semicolon"));
        stream->seek(lastPos);
    }

    return ret;
}

QString HTMLTokenizerPrivate::consumeHexDigits()
{
    qint64 originalPos = stream->pos();

    QString ret;
    QChar data;
    do {
        *stream >> data;
        // Zero (0) to Nine (9)
        if ((data >= 0x0030 && data <= 0x39) ||
                (data >= 0x0041 && data <= 0x0046) || // Latin A to Latin F
                (data >= 0x0061 && data <= 0x0066)) { // Latin a to Latin f
            ret.append(data);
        } else {

        }
    } while (!stream->atEnd());
    return ret;
}

QString HTMLTokenizerPrivate::consumeDigits()
{
    qint64 originalPos = stream->pos();

    QString ret;
    QChar data;
    do {
        *stream >> data;
        // Zero (0) to Nine (9)
        if ((data >= 0x0030 && data <= 0x39)) {
            ret.append(data);
        } else {

        }
    } while (!stream->atEnd());
    return ret;
}
