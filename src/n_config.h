/*
 * n_config.h - configuration file
 * Copyright (C) 2008 Sebastien Dolard
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef N_CONFIG_H
#define N_CONFIG_H

// Qt
#include <QStringList>
#include <QMutex>
#include <QDir>
#include <QDateTime>
#include <QSslConfiguration>

// App
#include "n_file_suffix.h"
#include "n_dir.h"
#include "n_settings.h"

class NConfig: public QObject
{
    Q_OBJECT
public:
    static NConfig & instance();
    static void deleteInstance();
    ~NConfig();

    // TODO: set server compression optionnal
    // TODO: disable auth, but listen only on localhost/127.0.0.1
    int serverPort();
    int serverSslPort();
    const QDir serverPub() const;
    bool isSslServerEnabled()const;
    bool isOnlySslServerEnabled() const;
    const QString AdminUser() const;
    const QString AdminPassword() const;
    const NDirList sharedDirectories() const;
    int addSharedDirectory(const NDir & dir) ;
    const NDir modifySharedDirectory(int id, const NDir & dir);
    void removeSharedDirectory(int id);
    void dumpSharedDirectoriesConfig() const;

    const NFileSuffixList & fileSuffixes() const;
    const QString & referenceServer() const;

    int dirUpdateDelay() const;
    const QDateTime lastDirUpdate() const;
    void setLastDirUpdateDone();
    void invalidLastDirUpdate();
    bool isLastDirUpdateValid() const;

    const QString dirFingerPrint() const;
    void setDirFingerPrint(const QString & fp);
    void clearDirUpdateData();

    // Returns a password generated with dbPwdHashKey (cf config file)
    // Return an empty string if password is empty
    const QString toPasswordHash(const QString & password) const;

    const QSslConfiguration & sslCfg() const;
    quint64 fileBufferSize() const;

signals:
    void configFileChanged();

private:
    static NConfig        *m_pInstance;
    NSettings              m_settings;
    int                    m_version;
    int                    m_serverPort;
    int                    m_serverSslPort;
    QDir                   m_serverPub;
    bool                   m_sslServerEnabled;
    bool                   m_onlySslServerEnabled;
    QString                m_serverSslCaCertificate;
    QString                m_serverSslLocalCertificate;
    QString                m_serverSslPrivateKey;
    QString                m_serverSslPrivateKeyPwd;
    QString                m_serverAdminUser;
    QString                m_serverAdminPassword;
    NDirList               m_sharedDirectories;
    NFileSuffixList        m_fileSuffixes;
    mutable QMutex         m_dataMutex;
    QString                m_referenceServer;
    int                    m_dirUpdateDelay;
    int                    m_dirWatchDelay;
    int                    m_dbFileDelay;
    QDateTime              m_lastDirUpdate;
    QString                m_dirFingerPrint;
    bool                   m_fileLoaded;
    QSslConfiguration      m_sslCfg;
    QByteArray             m_dbPwdHashKey;
    qint64                 m_fileBufferSize;

    NConfig();
    bool load();
    void save();
    void writeDefaultConfigFile();
    void genSslCfg();
    const QByteArray & dbPwdHashKey() const;

private slots:
    void onConfigFileChanged();
};

inline NConfig & getConfig() {
    return NConfig::instance();
}


#endif // N_CONFIG_H

