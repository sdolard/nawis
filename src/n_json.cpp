#include <QScriptValueIterator>
#include <QVariant>

#include "n_json.h"

const QByteArray NJson::serializeToQByteArray(const QScriptValue & sv)
{
    return serialize(sv).toUtf8();
}

const QString NJson::serialize(const QScriptValue & sv)
{
    if (!sv.isValid())
        return QString();
    /**
    * JSON value are:
    *    string
    *    number
    *    object
    *    array
    *    true
    *    false
    *    null
    */
    // Order is important!
    if (sv.isString())
        return serializeString(sv);
    if (sv.isNumber())
        return serializeNumber(sv);
    if (sv.isArray()) // array IS an object, isArray test must be done before is Objet test
        return serializeArray(sv);
    if (sv.isObject())
        return serializeObject(sv);
    if (sv.isBool())
        return serializeBool(sv);
    if (sv.isNull())
        return serializeNull(sv);

    if (sv.isDate())
        Q_ASSERT_X("NJson::serialize", "sv.isDate()", false);
    if (sv.isError())
        Q_ASSERT_X("NJson::serialize", "sv.isError()", false);
    if (sv.isFunction())
        Q_ASSERT_X("NJson::serialize", "sv.isFunction()", false);
    if (sv.isQMetaObject())
        Q_ASSERT_X("NJson::serialize", "sv.isQMetaObject()", false);
    if (sv.isQObject())
        Q_ASSERT_X("NJson::serialize", "sv.isQObject()", false);
    if (sv.isRegExp())
        Q_ASSERT_X("NJson::serialize", "sv.isRegExp()", false);
    if (sv.isUndefined())
        Q_ASSERT_X("NJson::serialize", "sv.isUndefined()", false);
    if (sv.isVariant())
        Q_ASSERT_X("NJson::serialize", "sv.isVariant()", false);

    Q_ASSERT(false); //?
    return "";
}

const QString NJson::serializeObject(const QScriptValue & sv)
{
    Q_ASSERT(sv.isObject());
    QString json;
    json = "{";
    QScriptValueIterator it(sv);
    while (it.hasNext()) {
        it.next();
        json += QString("\"%1\":").arg(it.name());
        json += serialize(it.value());
        if (it.hasNext())
            json += ",";
    }
    return json += "}";
}

const QString NJson::serializeArray(const QScriptValue & sv)
{
    Q_ASSERT(sv.isArray());
    QString json;
    json = "[";
    QScriptValueIterator it(sv);
    while (it.hasNext()) {
        it.next();
        if (!it.hasNext() && it.name().compare("length") == 0) { // reserved word/property on array
            if (json.endsWith(","))
                json.remove(json.length() - 1, 1);
            break;
        }
        json += serialize(it.value());
        if (it.hasNext())
            json += ",";
    }
    return json += "]";
}

const QString NJson::serializeBool(const QScriptValue & sv)
{
    return sv.toVariant().toString();
}

const QString NJson::serializeNull(const QScriptValue & )
{
    Q_ASSERT(false);
    return "";
}

const QString NJson::serializeNumber(const QScriptValue & sv)
{
    Q_ASSERT(sv.isNumber());
    return QString("%1").arg(sv.toString());
}

const QString NJson::serializeString(const QScriptValue & sv)
{
    // THIS FUNCTION IS NOT FAMOUS... Review it.
    /*
	char
	any-Unicode-character-
		except-"-or-\-or-
		control-character
        \ \  reverse
        \ "  quotation
        \ /  solidus
        \ b  backspace
        \ f  formfeed
        \ n  newline
        \ r  carriage return
        \ t  horizontal tab
        \ u  four-hex-digits
	*/
    Q_ASSERT(sv.isString());
    QString value = sv.toString();
    value.replace(QString("\\"), QString("\\\\"));
    value.replace(QString("\""), QString("\\\""));
    value.replace(QString("//"), QString("\\//"));
    value.replace(QString("\b"), QString("\\b"));
    value.replace(QString("\f"), QString("\\f"));
    value.replace(QString("\n"), QString("\\n"));
    value.replace(QString("\r"), QString("\\r"));
    value.replace(QString("\t"), QString("\\t"));
    //value = value.replace(QString("\u"), QString("\\u"));
    return QString("\"%1\"").arg(value);
}
