#ifndef N_RESPONSE_H
#define N_RESPONSE_H

// Qt
#include <QByteArray>
#include <QHttpResponseHeader>
#include <QFileInfo>
#include <QFile>

// App
#include "n_mime_type.h"
#include "n_compress.h"

class NResponse
{
public:
    enum DataType {
        dtPriorBuffered,
        dtFile
    };

    NResponse();
    NResponse(const NResponse& response);
    NResponse & operator=(const NResponse& response);

    void setData(const QByteArray & data);
    QByteArray & data();
    void deflateData(bool ieCompat);
    void setGZipped();

    void setHTTPHeader(const QHttpResponseHeader & httpHeader);
    QHttpResponseHeader & httpHeader();

    void clear();
    bool keepAlive();

    // not data, no file to send
    bool isEmpty();

    void setFileInfo(const QFileInfo & fileInfo);
    QFileInfo & fileInfo();
    void setFileRange(const QString & byteRange);

    DataType dataType();

    /*
		return -1 if an error occured
		return 0 if buffer is not empty (it will not be updated)
		return buffer size if it has been updated
	*/
    int updateFileBuffer();

    /*
	* Add expires header 10 years from now
	*/
    void add10yExpiresHttpHeader();

    /*
	* Return true if data are gzipped
	*/
    bool isGZipped() const;

    /*
	* Return true if data are deflated
	*/
    bool isDeflated() const;

    /*
	* Remove default UTF8 charset
	*/
    void removeDefaultCharset();

    /*
	 * Set session id cookie
	 */
    void setSessionCookie(const QString & sessionId);
    void clearSessionCookie();

private:
    DataType            m_dataType;
    QByteArray          m_data;
    QHttpResponseHeader m_httpHeader;
    QFileInfo           m_fileInfo;
    QFile               m_file;
    qint64              m_fileFirstBytePos;
    qint64              m_fileLastBytePos;
    qint64              m_fileReadSize;
    bool                m_compressed;
    qint64              m_maxFileBufferSize;
};

#endif // N_RESPONSE_H
