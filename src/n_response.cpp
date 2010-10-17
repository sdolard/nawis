// Qt
#include <QDateTime>
#include <QUuid>
#include <QNetworkCookie>

// App
#include "n_version.h"
#include "n_log.h"
#include "n_http.h"
#include "n_date.h"
#include "n_config.h"

#include "n_response.h"

NResponse::NResponse()
{
    m_maxFileBufferSize = getConfig().fileBufferSize();
    m_fileFirstBytePos  = 0;
    m_fileLastBytePos   = 0;
    m_fileReadSize      = 0;
    m_compressed        = false;

    m_httpHeader.setValue("Date", NDate_n::toHTMLDateTime(QDateTime::currentDateTime()));
    m_httpHeader.setValue("Server", NVersion_n::namedVersion(true));
    m_httpHeader.setValue("Accept-Ranges", "bytes");
    m_httpHeader.setContentType("application/json");  // Default server language format
    m_httpHeader.setValue("charset","UTF-8");
    m_dataType = NResponse::dtPriorBuffered;
}

void NResponse::removeDefaultCharset()
{
    m_httpHeader.removeValue("charset");
}

NResponse::NResponse(const NResponse& response)
{
    m_dataType = response.m_dataType;
    m_data = response.m_data;
    m_httpHeader = response.m_httpHeader;
    m_fileInfo = response.m_fileInfo;
    m_fileFirstBytePos = response.m_fileFirstBytePos;
    m_fileLastBytePos = response.m_fileLastBytePos;
    m_fileReadSize = response.m_fileReadSize;
    m_compressed = response.m_compressed;
    m_file.close();
    m_file.setFileName("");
}

NResponse & NResponse::operator=(const NResponse& response)
                                {
    m_dataType = response.m_dataType;
    m_data = response.m_data;
    m_httpHeader = response.m_httpHeader;
    m_fileInfo = response.m_fileInfo;
    m_fileFirstBytePos = response.m_fileFirstBytePos;
    m_fileLastBytePos = response.m_fileLastBytePos;
    m_fileReadSize = response.m_fileReadSize;
    m_compressed = response.m_compressed;
    m_file.close();
    m_file.setFileName("");
    return *this;
}

void NResponse::setData(const QByteArray & data)
{
    Q_ASSERT(!m_compressed);
    m_data = data;
}

QByteArray & NResponse::data()
{
    return m_data;
}

void NResponse::setHTTPHeader(const QHttpResponseHeader & httpHeader)
{
    m_httpHeader = httpHeader;
}

QHttpResponseHeader & NResponse::httpHeader()
{
    return m_httpHeader;
}

void NResponse::clear()
{
    m_data.clear();
    m_httpHeader = QHttpResponseHeader();
}

bool NResponse::keepAlive()
{
    return m_httpHeader.value("Connection").compare("keep-alive", Qt::CaseInsensitive) == 0;
}

bool NResponse::isEmpty()
{
    switch (m_dataType) {
    case NResponse::dtPriorBuffered:
        return m_data.isEmpty();
    case NResponse::dtFile:
        return m_fileInfo.fileName().isEmpty();
    default:
        Q_ASSERT(false);
        break;
    }
    return true;
}

void NResponse::setFileInfo(const QFileInfo & fileInfo)
{
    if (fileInfo.exists()) {
        m_dataType = NResponse::dtFile;
        // TODO: test with all browsers
        // Firefox OK
        // Safari OK
        // Internet Explorer OK
        // Opera OK
        // Google Chrome Ok
        // Konqueror // KO
        m_httpHeader.setValue("Content-Disposition",
                              QString("attachment; filename=\"%1\"").arg(fileInfo.fileName()));
        m_fileInfo = fileInfo;
        return;
    }
    m_dataType = NResponse::dtPriorBuffered;
    m_fileInfo = QFileInfo();
    setFileRange("");
}

NResponse::DataType NResponse::dataType()
{
    return m_dataType;
}

int NResponse::updateFileBuffer()
{
    if (m_data.size() != 0)
        return 0;

    if (m_file.fileName().isEmpty()) // file not read
    {
        m_file.setFileName(m_fileInfo.absoluteFilePath());
        if (!m_file.open(QIODevice::ReadOnly))
        {
            return -1;
        } else {
            if (m_fileFirstBytePos != 0)
            {
                if (!m_file.seek(m_fileFirstBytePos))
                    return -1;
            }
        }
    }

    if (!m_file.isOpen())
        return 0;
    if (m_file.atEnd())
        return 0;

    quint64 toRead = m_maxFileBufferSize;
    if (m_fileLastBytePos != 0)
    {
        if ((m_file.pos() + m_maxFileBufferSize) > m_fileLastBytePos + 1)
            toRead = m_fileLastBytePos - m_file.pos() + 1;
    }
    m_data = m_file.read(toRead);
    m_fileReadSize += m_data.size();

    if (m_file.atEnd() ||
        (m_file.pos() == (m_fileLastBytePos + 1)))
    {
        Q_ASSERT(m_fileReadSize = m_fileLastBytePos - m_fileFirstBytePos + 1);
        m_file.close();
    }
    return m_data.size();
}

QFileInfo & NResponse::fileInfo()
{
    return m_fileInfo;
}

void NResponse::setFileRange(const QString & byteRange)
{
    m_fileFirstBytePos = 0;
    m_fileLastBytePos = 0;
    m_fileReadSize = 0;
    m_httpHeader.removeValue("Content-Range");
    m_httpHeader.setValue("Content-Length", QString("%1").arg(m_fileInfo.size()));

    QString range = byteRange;
    range.remove("bytes=");
    QStringList list = range.split("-");
    if (list.count() != 2)
        return;
    QString firstBytePos = list.at(0);
    QString lastBytePos = list.at(1);

    // for a file length = 10000
    if (!firstBytePos.isEmpty()) // 9500-x
    {
        m_fileFirstBytePos = firstBytePos.toUInt();
        if (lastBytePos.isEmpty())
            m_fileLastBytePos = m_fileInfo.size() - 1; // 9500-
        else
            m_fileLastBytePos = lastBytePos.toUInt(); // 9500-9800
    } else {
        if (!lastBytePos.isEmpty()) // -500
        {
            m_fileFirstBytePos = m_fileInfo.size() - 1 - lastBytePos.toUInt();
            m_fileLastBytePos = m_fileInfo.size() - 1;
        }
    }
    Q_ASSERT(m_fileLastBytePos >= m_fileFirstBytePos);

    if (m_fileLastBytePos < m_fileFirstBytePos)
        m_fileLastBytePos = m_fileFirstBytePos;

#ifdef DEBUG
    logDebug("NResponse::setFileRange", byteRange);
    logDebug("NResponse::setFileRange", QString("Content-Range: bytes %1-%2/%3").
          arg(m_fileFirstBytePos).arg(m_fileLastBytePos).arg(m_fileInfo.size()));
    logDebug("NResponse::setFileRange", QString("Content-Length: %1").
          arg(m_fileLastBytePos - m_fileFirstBytePos + 1));
#endif
    m_httpHeader.setStatusLine(N_HTTP_PARTIAL_CONTENT, NHttp_n::statusCodeToString(N_HTTP_PARTIAL_CONTENT));
    m_httpHeader.setValue("Content-Range", QString("bytes %1-%2/%3").
                          arg(m_fileFirstBytePos).arg(m_fileLastBytePos).arg(m_fileInfo.size()));
    m_httpHeader.setValue("Content-Length", QString("%1").arg(m_fileLastBytePos - m_fileFirstBytePos + 1));
}


void NResponse::deflateData(bool ieCompat)
{
    Q_ASSERT(!m_compressed);
    if (m_compressed)
        return;

    if (!NMimeType_n::contentTypeNeedCompression(m_httpHeader.contentType()))
        return;

    m_data = NCompress_n::deflateData(m_data, ieCompat);
    m_httpHeader.setValue("Content-Encoding", "deflate");
    m_compressed = true;
}

void NResponse::setGZipped()
{
    Q_ASSERT(!m_compressed);
    if (m_compressed)
        return;

    m_httpHeader.setValue("Content-Encoding", "gzip");
    m_compressed = true;
}

bool NResponse::isGZipped() const
{
    return m_httpHeader.value("Content-Encoding") == "gzip";
}

bool NResponse::isDeflated() const
{
    return m_httpHeader.value("Content-Encoding") == "deflate";
}

void NResponse::add10yExpiresHttpHeader(){
    //Client cache management
    QDateTime TenYearsFromNow = QDateTime::currentDateTime().addYears(10);
    m_httpHeader.setValue("Expires", NDate_n::toHTMLDateTime(TenYearsFromNow));
}

void NResponse::setSessionCookie(const QString & sessionId, bool secured)
{
    QNetworkCookie sessionCookie;
    sessionCookie.setHttpOnly (true);
    sessionCookie.setName ("nawis_sessionId");
    sessionCookie.setPath ("/");
    sessionCookie.setSecure(secured);
    sessionCookie.setValue(sessionId.toAscii());

    m_httpHeader.setValue("Set-Cookie", sessionCookie.toRawForm(QNetworkCookie::Full));
}

void NResponse::clearSessionCookie()
{
    setSessionCookie("");
}

