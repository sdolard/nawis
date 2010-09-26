#ifndef N_JSON_H
#define N_JSON_H

#include <QScriptValue>
#include <QString>
#include <QByteArray>

/**
* http://www.json.org
* JSON value are:
*    string
*    number
*    object
*    array
*    true
*    false
*    null
*/

class NJson
{
public:
    static const QString serialize(const QScriptValue & sv);
    static const QByteArray serializeToQByteArray(const QScriptValue & sv);

private:
    static const QString serializeString(const QScriptValue & sv);
    static const QString serializeNumber(const QScriptValue & sv);
    static const QString serializeObject(const QScriptValue & sv);
    static const QString serializeArray(const QScriptValue & sv);
    static const QString serializeBool(const QScriptValue & sv);
    static const QString serializeNull(const QScriptValue & sv);   
};
#endif // N_JSON_H
