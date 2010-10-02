#include <QScriptEngine>
#include <QFileInfo>
#include <QHostAddress>

#include "n_log.h"
#include "n_json.h"
#include "n_config.h"
#include "n_music_database.h"
#include "n_database.h"
#include "n_convert.h"
#include "n_tcp_server_socket_auth_services.h"
#include "n_http.h"

#include "n_tcp_server_socket_music_services.h"


NTcpServerSocketMusicServices * NTcpServerSocketMusicServices::m_instance = NULL;

NTcpServerSocketMusicServices & NTcpServerSocketMusicServices::instance()
{
    if (m_instance == NULL)
        m_instance = new NTcpServerSocketMusicServices();
    return *m_instance;
}

void NTcpServerSocketMusicServices::deleteInstance()
{
    if (m_instance == NULL)
        return;
    delete m_instance;
    m_instance = NULL;
}

NTcpServerSocketMusicServices::NTcpServerSocketMusicServices()
{
}

NTcpServerSocketMusicServices::~NTcpServerSocketMusicServices()
{
}

NResponse & NTcpServerSocketMusicServices::getAlbum(const NClientSession & session,
                                                       NResponse & response)
{
    bool ok;
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);
    int start = session.url().queryItemValue("start").toInt();
    int limit = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString dir = session.url().queryItemValue("dir");
    QString artist;
    if(session.url().hasQueryItem("artist"))// Cos of NULL test
    {
        artist = session.url().queryItemValue("artist");
        if (artist.isNull())
            artist = "";
        artist = NConvert_n::fromUTF8PercentEncoding(artist);
    }
    QString genre;
    if(session.url().hasQueryItem("genre"))// Cos of NULL test
    {
        genre = session.url().queryItemValue("genre");
        if (genre.isNull())
            genre = "";
        genre = NConvert_n::fromUTF8PercentEncoding(genre);
    }

    int year = session.url().queryItemValue("year").toInt(&ok);
    if (!ok)
        year = -1;

    const NTcpServerAuthSession authSession = getAuthServices().getSession(session.sessionId());
    logMessage(session.socket()->peerAddress().toString(),
          QString("%1 is looking for album: \"%2\"; start: %3; limit: %4, dir:\"%5\"").
          arg(authSession.login()).arg(NConvert_n::fromUTF8PercentEncoding(search)).arg(start).arg(limit).arg(dir));

    int totalCount = NMDB.getAlbumListCount(searches, year, genre, artist);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);
    // TODO: manage limit = -1 to get all album
    bool succeed = NMDB.getAlbumList(se, svData, totalCount, searches, start,
                                         limit, dir, year, genre, artist);
    setJsonRootReponse(svRoot, totalCount, succeed);
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketMusicServices::getArtist(const NClientSession & session,
                                                        NResponse & response)
{
    bool ok;
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);
    int start = session.url().queryItemValue("start").toInt();
    int limit = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString dir = session.url().queryItemValue("dir");
    int year  = session.url().queryItemValue("year").toInt(&ok);
    if (!ok)
        year = -1;

    QString genre;
    if(session.url().hasQueryItem("genre"))// Cos of NULL test
    {
        genre = session.url().queryItemValue("genre");
        if (genre.isNull())
            genre = "";
        genre = NConvert_n::fromUTF8PercentEncoding(genre);
    }

    const NTcpServerAuthSession authSession = getAuthServices().getSession(session.sessionId());
    logMessage(session.socket()->peerAddress().toString(),
          QString("%1 is looking for artist: \"%2\"; start: %3; limit: %4, dir:\"%5\"").
          arg(authSession.login()).arg(NConvert_n::fromUTF8PercentEncoding(search)).arg(start).arg(limit).arg(dir));

    int totalCount = NMDB.getArtistListCount(searches, year, genre);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);
    bool succeed = NMDB.getArtistList(se, svData, totalCount, searches, start, limit, dir, year, genre);
    setJsonRootReponse(svRoot, totalCount, succeed);
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketMusicServices::getGenre(const NClientSession & session,
                                                       NResponse & response)
{
    bool ok;
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);
    int start = session.url().queryItemValue("start").toInt();
    int limit  = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString dir = session.url().queryItemValue("dir");
    int year  = session.url().queryItemValue("year").toInt(&ok);
    if (!ok)
        year = -1;

    const NTcpServerAuthSession & authSession = getAuthServices().getSession(session.sessionId());
    logMessage(session.socket()->peerAddress().toString(),
          QString("%1 is looking for genre: \"%2\"; start: %3; limit: %4, dir:\"%5\"").
          arg(authSession.login()).arg(NConvert_n::fromUTF8PercentEncoding(search)).arg(start).arg(limit).arg(dir));

    int totalCount = NMDB.getGenreListCount(searches, year);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);
    bool succeed = NMDB.getGenreList(se, svData, totalCount, searches, start, limit, dir, year);
    setJsonRootReponse(svRoot, totalCount, succeed);
    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketMusicServices::getID3Picture(int *statusCode,
                                                            const NClientSession & session,
                                                            NResponse & response)
{
    response.removeDefaultCharset();

    QString fileHash = session.resource();
    QFileInfo fileInfo = NDatabase::instance().file(fileHash);

    // Cache
    QString lastModified = NDate_n::toHTMLDateTime(fileInfo.lastModified());
    response.httpHeader().setValue("Last-Modified", lastModified);
    if (session.request().value("If-Modified-Since") == lastModified)
    {
        *statusCode = N_HTTP_NOT_MODIFIED;
        return response;
    }

    // File
    QByteArray ba;
    QString mimeType;
    if (NMetadata::getID3Picture(fileInfo.absoluteFilePath(), ba, mimeType))
    {
        response.setData(ba);
        response.httpHeader().setContentType(mimeType);
        response.add10yExpiresHttpHeader();
    }

    return response;
}

NResponse & NTcpServerSocketMusicServices::getYear(const NClientSession & session,
                                                      NResponse & response)
{
    bool ok;
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);
    int start = session.url().queryItemValue("start").toInt();
    int limit  = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString dir = session.url().queryItemValue("dir");

    const NTcpServerAuthSession authSession = getAuthServices().getSession(session.sessionId());
    logMessage(session.socket()->peerAddress().toString(),
          QString("%1 is looking for year: \"%2\"; start: %3; limit: %4, dir:\"%5\"").
          arg(authSession.login()).arg(NConvert_n::fromUTF8PercentEncoding(search)).arg(start).arg(limit).arg(dir));


    int totalCount = NMDB.getYearListCount(searches);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);

    bool succeed = NMDB.getYearList(se, svData, totalCount, searches, start,
                                        limit, dir);
    setJsonRootReponse(svRoot, totalCount, succeed);

    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}

NResponse & NTcpServerSocketMusicServices::getTitle(const NClientSession & session,
                                                       NResponse & response)
{
    bool ok;
    int start = session.url().queryItemValue("start").toInt();
    if (!ok)
        start = 0;
    int limit  = session.url().queryItemValue("limit").toInt(&ok);
    if (!ok)
        limit = 25;
    QString dir = session.url().queryItemValue("dir");
    QString sort = session.url().queryItemValue("sort");
    QString search = session.url().queryItemValue("search");
    QStringList searches = search.split("+", QString::SkipEmptyParts);
    searches = NConvert_n::fromUTF8PercentEncoding(searches);

    QString album;
    if(session.url().hasQueryItem("album")) // Cos of NULL test
    {
        album = session.url().queryItemValue("album");
        if (album.isNull())
            album = "";
        album = NConvert_n::fromUTF8PercentEncoding(album);

    }
    QString artist;
    if(session.url().hasQueryItem("artist"))// Cos of NULL test
    {
        artist = session.url().queryItemValue("artist");
        if (artist.isNull())
            artist = "";
        artist = NConvert_n::fromUTF8PercentEncoding(artist);
    }
    QString genre;
    if(session.url().hasQueryItem("genre"))// Cos of NULL test
    {	genre = session.url().queryItemValue("genre");
        if (genre.isNull())
            genre = "";
        genre = NConvert_n::fromUTF8PercentEncoding(genre);
    }

    int year  = session.url().queryItemValue("year").toInt(&ok);
    if (!ok)
        year = -1;

    const NTcpServerAuthSession authSession = getAuthServices().getSession(session.sessionId());
    logMessage(session.socket()->peerAddress().toString(),
          QString("%1 is looking for music search:\"%2\"; album:\"%3\"; artist:\"%4\";"\
             "genre:\"%5\"; year:\"%6\"; start:\"%7\"; "\
             "limit:\"%8\"; sort:\"%9\"; dir:\"%10\"").
          arg(authSession.login()). // 1
          arg(NConvert_n::fromUTF8PercentEncoding(search)).// 2
          arg(album).// 3
          arg(artist).// 4
          arg(genre).// 5
          arg(year).// 6
          arg(start).// 7
          arg(limit).// 8
          arg(sort).// 9
          arg(dir));// 10

    int totalCount = NMDB.getTitleListCount(searches, album, artist, genre, year);
    QScriptEngine se;
    QScriptValue svRoot = se.newObject();
    QScriptValue svData = se.newArray(totalCount);
    svRoot.setProperty(RSP_DATA, svData);

    bool succeed = NMDB.getTitleList(se, svData, searches, album, artist, genre, year,
                                         start, limit, sort, dir);

    setJsonRootReponse(svRoot, totalCount, succeed);

    response.setData(NJson::serializeToQByteArray(svRoot));
    return response;
}
