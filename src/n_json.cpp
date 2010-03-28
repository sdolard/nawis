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

    if (sv.isArray())
        return serializeArray(sv);
    if (sv.isObject())
        return serializeObject(sv);
    if (sv.isBool())
        return serializeBool(sv);
    if (sv.isDate())
        return serializeDate(sv);
    if (sv.isNull())
        return serializeNull(sv);
    if (sv.isNumber())
        return serializeNumber(sv);
    if (sv.isString())
        return serializeString(sv);
    if (sv.isUndefined())
        return serializeUndefined(sv);

    Q_ASSERT(false);
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
        if (!it.value().isObject())
            json += QString("\"%1\":").arg(it.name());
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

const QString NJson::serializeDate(const QScriptValue & )
{
    Q_ASSERT(false);
    return "";
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
    value = value.replace(QString("\\"), QString("\\\\"));
    value = value.replace(QString("\""), QString("\\\""));
    value = value.replace(QString("//"), QString("\\//"));
    value = value.replace(QString("\b"), QString("\\b"));
    value = value.replace(QString("\f"), QString("\\f"));
    value = value.replace(QString("\n"), QString("\\n"));
    value = value.replace(QString("\r"), QString("\\r"));
    value = value.replace(QString("\t"), QString("\\t"));
    //value = value.replace(QString("\u"), QString("\\u"));
    return QString("\"%1\"").arg(value);
}

const QString NJson::serializeUndefined(const QScriptValue & )
{
    Q_ASSERT(false);
    return "";
}
