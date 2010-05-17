/*
 * n_config.cpp - configuration file
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

// Qt
#include <QMutexLocker>
//#include <QApplication>
#include <QFile>
#include <QSslKey>
//#include <QDesktopServices>

// App
#include "n_file_category.h"
#include "n_dir.h"
#include "n_log.h"

#include "n_config.h"

// TODO: make auth optionnal, listen on loopback only

#define NCONFIG_GENERAL                           "general"
#define NCONFIG_GENERAL_VERSION_KEY               "version"
#define NCONFIG_GENERAL_VERSION_VALUE             1

#define NCONFIG_SERVER                            "server"
#define NCONFIG_SERVER_PORT_KEY                   "port"
#define NCONFIG_SERVER_PORT_VALUE                 6391
#define NCONFIG_SERVER_SSL_KEY                    "ssl"
#define NCONFIG_SERVER_SSL_VALUE                  "false"
#define NCONFIG_SERVER_SSL_CA_CERTIFICATE_KEY     "ssl_ca_certificate"
#define NCONFIG_SERVER_SSL_PRIVATE_KEY_KEY        "ssl_privare_key"
#define NCONFIG_SERVER_SSL_PRIVATE_KEY_PWD_KEY    "ssl_privare_key_password"
#define NCONFIG_SERVER_SSL_LOCAL_CERTIFICATE_KEY  "ssl_local_certificate"
#define NCONFIG_SERVER_WEB_UI_KEY                 "webui"
#define NCONFIG_SERVER_ADMIN_USER_KEY             "admin_user"
#define NCONFIG_SERVER_ADMIN_USER_VALUE           "admin"
#define NCONFIG_SERVER_ADMIN_PASSWORD_KEY         "admin_password"
#define NCONFIG_SERVER_ADMIN_PASSWORD_VALUE       "admin"


#define NCONFIG_NODE                              "node"
#define NCONFIG_NODE_ADDRESS_KEY                  "address"

#define NCONFIG_SHARED_DIR                        "shared_dir"
#define NCONFIG_SHARED_DIR_PATH_KEY               "path"
#define NCONFIG_SHARED_DIR_RECURSIVE_KEY          "recursive"
#define NCONFIG_SHARED_DIR_SHARED_KEY             "shared"

#define NCONFIG_MOVIE_SUFFIX                      "movie_suffix"
#define NCONFIG_MUSIC_SUFFIX                      "music_suffix"
#define NCONFIG_PICTURE_SUFFIX                    "picture_suffix"
#define NCONFIG_ARCHIVE_SUFFIX                    "archive_suffix"
#define NCONFIG_DOCUMENT_SUFFIX                   "document_suffix"
#define NCONFIG_OTHER_SUFFIX                      "other_suffix"

#define NCONFIG_SUFFIX_NAME_KEY                   "name"
#define NCONFIG_SUFFIX_SHARED_KEY                 "shared"

#define NCONFIG_ADV                               "advanced"
#define NCONFIG_ADV_DIR_UPD_DELAY_KEY             "dir_update_delay"
#define NCONFIG_ADV_DIR_UPD_DELAY_VALUE           1000 * 60 * 60 * 24 // once a day
#define NCONFIG_ADV_LAST_DIR_UPD_KEY              "last_dir_update"
#define NCONFIG_ADV_DIR_FINGER_PRINT_KEY          "dir_finger_print"
#define NCONFIG_ADV_DB_PWD_HASH_KEY               "db_pwd_hash_key"
#define NCONFIG_ADV_DB_PWD_HASH_VALUE             "0x6b697373686172652072756c657a"
#define NCONFIG_ADV_FILE_BUFFER_SIZE_KEY          "file_buffer_size"
#define NCONFIG_ADV_FILE_BUFFER_SIZE_VALUE        1024*1024*10

/*
 server def ssl=true
 server def ssl_ca_certificate=/Users/sebastiend/Documents/dev/nawis/certificate/sebastiend.crt
 server def ssl_local_certificate=/Users/sebastiend/Documents/dev/nawis/certificate/sebastiend.crt
 server def ssl_privare_key=/Users/sebastiend/Documents/dev/nawis/certificate/sebastiend.key
 server def ssl_privare_key_password=test
 */

NConfig * NConfig::m_instance = NULL;

NConfig & NConfig::instance()
{
    if (m_instance == NULL)
        m_instance = new NConfig();
    return *m_instance;
}

void NConfig::deleteInstance()
{
    if (m_instance == NULL)
        return;
    delete m_instance;
    m_instance = NULL;
}

NConfig::NConfig()
{
    m_fileLoaded = false;
    connect(&m_settings, SIGNAL(configFileChanged()),
            this, SLOT(onConfigFileChanged()));
    load();
}

NConfig::~NConfig()
{
    if (m_fileLoaded)
    {
        disconnect();
        m_settings.disconnect();
        m_settings.saveToFile();
    }
}

void NConfig::save()
{
    if (!m_fileLoaded)
        return;

    /**
    * Sharing group
    */
    m_settings.deleteGroup(NCONFIG_SHARED_DIR);
    for(int i = 0; i < m_sharedDirectories.count(); i++)
    {
        const NDir & dir = m_sharedDirectories[i];
        m_settings.beginGroup(NCONFIG_SHARED_DIR);
        m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_PATH_KEY,
                            dir.dir().path());
        m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_RECURSIVE_KEY,
                            dir.recursive());
        m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_SHARED_KEY,
                            dir.shared());
        m_settings.endGroup();
    }

    /**
    * Advanced group
    */
    m_settings.setValue(NCONFIG_ADV, NCONFIG_ADV_LAST_DIR_UPD_KEY,
                        m_lastDirUpdate);
    m_settings.setValue(NCONFIG_ADV, NCONFIG_ADV_DIR_FINGER_PRINT_KEY,
                        m_dirFingerPrint);

    m_settings.saveToFile();
}

bool NConfig::load()
{
    QMutexLocker locker(&m_dataMutex);
    NLOGD("NConfig", "m_dataMutex NConfig::load");
    if (!m_settings.loadFromFile())
    {
        m_fileLoaded = false;
        return false;
    }
    m_fileLoaded = true; // must be done there cose of configuration file update

    m_version = m_settings.value(NCONFIG_GENERAL, NCONFIG_GENERAL_VERSION_KEY, 0).toInt();
    if (m_version < NCONFIG_GENERAL_VERSION_VALUE)
        writeDefaultConfigFile();

    /* Server
	 */
    m_serverPort = m_settings.value(NCONFIG_SERVER, NCONFIG_SERVER_PORT_KEY,
                                    NCONFIG_SERVER_PORT_VALUE).toInt();
    m_serverPub.setPath(m_settings.value(NCONFIG_SERVER, NCONFIG_SERVER_WEB_UI_KEY).toString());
    m_serverAdminUser =  m_settings.value(NCONFIG_SERVER, NCONFIG_SERVER_ADMIN_USER_KEY,
                                          NCONFIG_SERVER_ADMIN_USER_VALUE).toString();
    m_serverAdminPassword =  m_settings.value(NCONFIG_SERVER, NCONFIG_SERVER_ADMIN_PASSWORD_KEY,
                                              NCONFIG_SERVER_ADMIN_PASSWORD_VALUE).toString();

    // SSL
    m_serverSsl = m_settings.value(NCONFIG_SERVER, NCONFIG_SERVER_SSL_KEY,
                                   NCONFIG_SERVER_SSL_VALUE).toBool();
    m_serverSslCaCertificate = m_settings.value(NCONFIG_SERVER,
                                                NCONFIG_SERVER_SSL_CA_CERTIFICATE_KEY, "To define").toString();
    m_serverSslLocalCertificate = m_settings.value(NCONFIG_SERVER,
                                                   NCONFIG_SERVER_SSL_LOCAL_CERTIFICATE_KEY, "To define").toString();
    m_serverSslPrivateKey = m_settings.value(NCONFIG_SERVER,
                                             NCONFIG_SERVER_SSL_PRIVATE_KEY_KEY, "To define").toString();
    m_serverSslPrivateKeyPwd = m_settings.value(NCONFIG_SERVER,
                                                NCONFIG_SERVER_SSL_PRIVATE_KEY_PWD_KEY, "To define").toString();

    /* Node
	 */
    m_referenceServer = m_settings.value(NCONFIG_NODE, NCONFIG_NODE_ADDRESS_KEY).toString();
    if (m_referenceServer.isEmpty())
        m_referenceServer = "127.0.0.1";


    /* Sharing group
	 */
    m_sharedDirectories.clear();
    int size = m_settings.groupSize(NCONFIG_SHARED_DIR);
    for (int i = 0; i < size; ++i) {
        QString d = m_settings.value(NCONFIG_SHARED_DIR, i, NCONFIG_SHARED_DIR_PATH_KEY,
                                     tr("To define")).toString();
        bool r = m_settings.value(NCONFIG_SHARED_DIR, i,
                                  NCONFIG_SHARED_DIR_RECURSIVE_KEY).toBool();
        bool s = m_settings.value(NCONFIG_SHARED_DIR, i,
                                  NCONFIG_SHARED_DIR_SHARED_KEY).toBool();
        NDir dir (d, r, s);
        m_sharedDirectories.append(dir);
    }

    m_fileSuffixes.clear();
    /* Movie group
	 */
    size = m_settings.groupSize(NCONFIG_MOVIE_SUFFIX);
    for (int i = 0; i < size; ++i) {
        NFileSuffix fs;
        fs.setName(m_settings.value(NCONFIG_MOVIE_SUFFIX, i,
                                    NCONFIG_SUFFIX_NAME_KEY).toString());
        fs.setShared(m_settings.value(NCONFIG_MOVIE_SUFFIX, i,
                                      NCONFIG_SUFFIX_SHARED_KEY, true).toBool());
        fs.setCategory(NFileCategory_n::fcMovie);
        m_fileSuffixes.insert(fs.name(), fs);
    }

    /* Music group
	 */
    size = m_settings.groupSize(NCONFIG_MUSIC_SUFFIX);
    for (int i = 0; i < size; ++i) {
        NFileSuffix fs;
        fs.setName(m_settings.value(NCONFIG_MUSIC_SUFFIX, i,
                                    NCONFIG_SUFFIX_NAME_KEY).toString());
        fs.setShared(m_settings.value(NCONFIG_MUSIC_SUFFIX, i,
                                      NCONFIG_SUFFIX_SHARED_KEY, true).toBool());
        fs.setCategory(NFileCategory_n::fcMusic);
        m_fileSuffixes.insert(fs.name(), fs);
    }

    /* Picture group
	 */
    size = m_settings.groupSize(NCONFIG_PICTURE_SUFFIX);
    for (int i = 0; i < size; ++i) {
        NFileSuffix fs;
        fs.setName(m_settings.value(NCONFIG_PICTURE_SUFFIX, i,
                                    NCONFIG_SUFFIX_NAME_KEY).toString());
        fs.setShared(m_settings.value(NCONFIG_PICTURE_SUFFIX, i,
                                      NCONFIG_SUFFIX_SHARED_KEY, true).toBool());
        fs.setCategory(NFileCategory_n::fcPicture);
        m_fileSuffixes.insert(fs.name(), fs);
    }

    /* Archive group
	 */
    size = m_settings.groupSize(NCONFIG_ARCHIVE_SUFFIX);
    for (int i = 0; i < size; ++i) {
        NFileSuffix fs;
        fs.setName(m_settings.value(NCONFIG_ARCHIVE_SUFFIX, i,
                                    NCONFIG_SUFFIX_NAME_KEY).toString());
        fs.setShared(m_settings.value(NCONFIG_ARCHIVE_SUFFIX, i,
                                      NCONFIG_SUFFIX_SHARED_KEY, true).toBool());
        fs.setCategory(NFileCategory_n::fcArchive);
        m_fileSuffixes.insert(fs.name(), fs);
    }

    /* Document group
	 */
    size = m_settings.groupSize(NCONFIG_DOCUMENT_SUFFIX);
    for (int i = 0; i < size; ++i) {
        NFileSuffix fs;
        fs.setName(m_settings.value(NCONFIG_DOCUMENT_SUFFIX, i,
                                    NCONFIG_SUFFIX_NAME_KEY).toString());
        fs.setShared(m_settings.value(NCONFIG_DOCUMENT_SUFFIX, i,
                                      NCONFIG_SUFFIX_SHARED_KEY, true).toBool());
        fs.setCategory(NFileCategory_n::fcDocument);
        m_fileSuffixes.insert(fs.name(), fs);
    }


    /* Other group
	 */
    size = m_settings.groupSize(NCONFIG_OTHER_SUFFIX);
    for (int i = 0; i < size; ++i) {
        NFileSuffix fs;
        fs.setName(m_settings.value(NCONFIG_OTHER_SUFFIX, i,
                                    NCONFIG_SUFFIX_NAME_KEY).toString());
        fs.setShared(m_settings.value(NCONFIG_OTHER_SUFFIX, i,
                                      NCONFIG_SUFFIX_SHARED_KEY, true).toBool());
        fs.setCategory(NFileCategory_n::fcOther);
        m_fileSuffixes.insert(fs.name(), fs);
    }

    // Advanced
    m_dirUpdateDelay = m_settings.value(NCONFIG_ADV, NCONFIG_ADV_DIR_UPD_DELAY_KEY,
                                        NCONFIG_ADV_DIR_UPD_DELAY_VALUE).toInt();
    m_lastDirUpdate = m_settings.value(NCONFIG_ADV, NCONFIG_ADV_LAST_DIR_UPD_KEY).toDateTime();
    m_dirFingerPrint = m_settings.value(NCONFIG_ADV, NCONFIG_ADV_DIR_FINGER_PRINT_KEY).toString();
    m_dbPwdHashKey = m_settings.value(NCONFIG_ADV, NCONFIG_ADV_DB_PWD_HASH_KEY).toByteArray();
    m_fileBufferSize = m_settings.value(NCONFIG_ADV, NCONFIG_ADV_FILE_BUFFER_SIZE_KEY,
                                        NCONFIG_ADV_FILE_BUFFER_SIZE_VALUE).toLongLong();

    //m_fileSuffixes.dump();

    genSslCfg();

    return true;
}

void NConfig::writeDefaultConfigFile()
{
    if (!m_fileLoaded)
        return;

    // Version
    m_settings.setValue(NCONFIG_GENERAL, NCONFIG_GENERAL_VERSION_KEY,
                        NCONFIG_GENERAL_VERSION_VALUE);

    /* Server group
	 */
    // port
    if (m_settings.value(NCONFIG_SERVER, NCONFIG_SERVER_PORT_KEY, -1).toInt() == -1)
        m_settings.setValue(NCONFIG_SERVER, NCONFIG_SERVER_PORT_KEY, NCONFIG_SERVER_PORT_VALUE);
    if (m_settings.value(NCONFIG_SERVER, NCONFIG_SERVER_WEB_UI_KEY).toString().isEmpty())
        m_settings.setValue(NCONFIG_SERVER, NCONFIG_SERVER_WEB_UI_KEY, "");
    //Login & pwd
    if (m_settings.value(NCONFIG_SERVER, NCONFIG_SERVER_ADMIN_USER_KEY, "").toString().isEmpty() )
        m_settings.setValue(NCONFIG_SERVER, NCONFIG_SERVER_ADMIN_USER_KEY, NCONFIG_SERVER_ADMIN_USER_VALUE);
    if (m_settings.value(NCONFIG_SERVER, NCONFIG_SERVER_ADMIN_PASSWORD_KEY, "").toString().isEmpty() )
        m_settings.setValue(NCONFIG_SERVER, NCONFIG_SERVER_ADMIN_PASSWORD_KEY, NCONFIG_SERVER_ADMIN_PASSWORD_VALUE);
    // SSL
    if (m_settings.value(NCONFIG_SERVER, NCONFIG_SERVER_SSL_KEY).toString().isEmpty())
        m_settings.setValue(NCONFIG_SERVER, NCONFIG_SERVER_SSL_KEY, false);
    if (m_settings.value(NCONFIG_SERVER, NCONFIG_SERVER_SSL_CA_CERTIFICATE_KEY).toString().isEmpty())
        m_settings.setValue(NCONFIG_SERVER, NCONFIG_SERVER_SSL_CA_CERTIFICATE_KEY, "To define");
    if (m_settings.value(NCONFIG_SERVER, NCONFIG_SERVER_SSL_LOCAL_CERTIFICATE_KEY).toString().isEmpty())
        m_settings.setValue(NCONFIG_SERVER, NCONFIG_SERVER_SSL_LOCAL_CERTIFICATE_KEY, "To define");
    if (m_settings.value(NCONFIG_SERVER, NCONFIG_SERVER_SSL_PRIVATE_KEY_KEY).toString().isEmpty())
        m_settings.setValue(NCONFIG_SERVER, NCONFIG_SERVER_SSL_PRIVATE_KEY_KEY, "To define");
    if (m_settings.value(NCONFIG_SERVER, NCONFIG_SERVER_SSL_PRIVATE_KEY_PWD_KEY).toString().isEmpty())
        m_settings.setValue(NCONFIG_SERVER, NCONFIG_SERVER_SSL_PRIVATE_KEY_PWD_KEY, "To define");

    /* Node group
	 */
    // Node address
    if (!m_settings.groupValueExists(NCONFIG_NODE, NCONFIG_NODE_ADDRESS_KEY,
                                     ""))
    {
        m_settings.beginGroup(NCONFIG_NODE);
        m_settings.setValue(NCONFIG_NODE, NCONFIG_NODE_ADDRESS_KEY, "");
        m_settings.endGroup();
    }

    /* Sharing group
	 */
    if (m_settings.groupSize(NCONFIG_SHARED_DIR) == 0)
    {
        m_settings.beginGroup(NCONFIG_SHARED_DIR);
        // TODO: review this, cos visibility of those directories is fx account that run service...
        //		m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_PATH_KEY,
        //							QDesktopServices::storageLocation ( QDesktopServices::DocumentsLocation ));
        //		m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_RECURSIVE_KEY,
        //							true);
        //		m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_SHARED_KEY,
        //							true);
        //		m_settings.endGroup();

        m_settings.beginGroup(NCONFIG_SHARED_DIR);
        //		m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_PATH_KEY,
        //							QDesktopServices::storageLocation ( QDesktopServices::MusicLocation ));
        //		m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_RECURSIVE_KEY,
        //							true);
        //		m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_SHARED_KEY,
        //							true);
        m_settings.endGroup();

        m_settings.beginGroup(NCONFIG_SHARED_DIR);
        //		m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_PATH_KEY,
        //							QDesktopServices::storageLocation ( QDesktopServices::MoviesLocation ));
        //		m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_RECURSIVE_KEY,
        //							true);
        //		m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_SHARED_KEY,
        //							true);
        m_settings.endGroup();

        m_settings.beginGroup(NCONFIG_SHARED_DIR);
        //		m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_PATH_KEY,
        //							QDesktopServices::storageLocation ( QDesktopServices::PicturesLocation ));
        //		m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_RECURSIVE_KEY,
        //							true);
        //		m_settings.setValue(NCONFIG_SHARED_DIR, NCONFIG_SHARED_DIR_SHARED_KEY,
        //							true);
        m_settings.endGroup();
    }

    /* Movie group
	 */
    QStringList suffixList;
    suffixList << "avi" << "srt" << "mpg" << "mkv" << "divx" << "xvid" << "mpeg";
    QString suffix;
    foreach (suffix, suffixList)
    {
        if (!m_settings.groupValueExists(NCONFIG_MOVIE_SUFFIX, NCONFIG_SUFFIX_NAME_KEY,
                                         suffix))
        {
            m_settings.beginGroup(NCONFIG_MOVIE_SUFFIX);
            m_settings.setValue(NCONFIG_MOVIE_SUFFIX, NCONFIG_SUFFIX_NAME_KEY, suffix);
            m_settings.setValue(NCONFIG_MOVIE_SUFFIX, NCONFIG_SUFFIX_SHARED_KEY , true);
            m_settings.endGroup();
        }
    }

    /* Music group
	 */
    suffixList.clear();
    suffixList << "mp3" << "mpc" << "flac" << "ogg";
    foreach (suffix, suffixList)
    {
        if (!m_settings.groupValueExists(NCONFIG_MUSIC_SUFFIX, NCONFIG_SUFFIX_NAME_KEY,
                                         suffix))
        {
            m_settings.beginGroup(NCONFIG_MUSIC_SUFFIX);
            m_settings.setValue(NCONFIG_MUSIC_SUFFIX, NCONFIG_SUFFIX_NAME_KEY, suffix);
            m_settings.setValue(NCONFIG_MUSIC_SUFFIX, NCONFIG_SUFFIX_SHARED_KEY , true);
            m_settings.endGroup();
        }
    }

    /* Picture group
	 */
    suffixList.clear();
    suffixList << "jpg" << "png" << "jpeg" << "bmp" << "tiff";
    foreach (suffix, suffixList)
    {
        if (!m_settings.groupValueExists(NCONFIG_PICTURE_SUFFIX, NCONFIG_SUFFIX_NAME_KEY,
                                         suffix))
        {
            m_settings.beginGroup(NCONFIG_PICTURE_SUFFIX);
            m_settings.setValue(NCONFIG_PICTURE_SUFFIX, NCONFIG_SUFFIX_NAME_KEY, suffix);
            m_settings.setValue(NCONFIG_PICTURE_SUFFIX, NCONFIG_SUFFIX_SHARED_KEY , true);
            m_settings.endGroup();
        }
    }

    /* Archive group
	 */
    suffixList.clear();
    suffixList << "rar" << "zip" << "7z" << "gz" << "bz2" << "tar" << "iso";
    foreach (suffix, suffixList)
    {
        if (!m_settings.groupValueExists(NCONFIG_ARCHIVE_SUFFIX, NCONFIG_SUFFIX_NAME_KEY,
                                         suffix))
        {
            m_settings.beginGroup(NCONFIG_ARCHIVE_SUFFIX);
            m_settings.setValue(NCONFIG_ARCHIVE_SUFFIX, NCONFIG_SUFFIX_NAME_KEY, suffix);
            m_settings.setValue(NCONFIG_ARCHIVE_SUFFIX, NCONFIG_SUFFIX_SHARED_KEY , true);
            m_settings.endGroup();
        }
    }


    /* Document group
	 */
    suffixList.clear();
    suffixList << "txt" << "nfo" << "doc" << "pdf" << "odt" << "xls" << "ods" << "ppt" << "pptx";
    foreach (suffix, suffixList)
    {
        if (!m_settings.groupValueExists(NCONFIG_DOCUMENT_SUFFIX, NCONFIG_SUFFIX_NAME_KEY,
                                         suffix))
        {
            m_settings.beginGroup(NCONFIG_DOCUMENT_SUFFIX);
            m_settings.setValue(NCONFIG_DOCUMENT_SUFFIX, NCONFIG_SUFFIX_NAME_KEY, suffix);
            m_settings.setValue(NCONFIG_DOCUMENT_SUFFIX, NCONFIG_SUFFIX_SHARED_KEY , true);
            m_settings.endGroup();
        }
    }

    /* Other group
	 */
    suffixList.clear();
    suffixList << tr("To define");
    foreach (suffix, suffixList)
    {
        if (!m_settings.groupValueExists(NCONFIG_OTHER_SUFFIX, NCONFIG_SUFFIX_NAME_KEY,
                                         suffix))
        {
            m_settings.beginGroup(NCONFIG_OTHER_SUFFIX);
            m_settings.setValue(NCONFIG_OTHER_SUFFIX, NCONFIG_SUFFIX_NAME_KEY, suffix);
            m_settings.setValue(NCONFIG_OTHER_SUFFIX, NCONFIG_SUFFIX_SHARED_KEY , true);
            m_settings.endGroup();
        }
    }

    // Advanced group
    if (m_settings.value(NCONFIG_ADV, NCONFIG_ADV_DIR_UPD_DELAY_KEY, -1).toInt() == -1)
        m_settings.setValue(NCONFIG_ADV, NCONFIG_ADV_DIR_UPD_DELAY_KEY,
                            NCONFIG_ADV_DIR_UPD_DELAY_VALUE);
    m_settings.setValue(NCONFIG_ADV, NCONFIG_ADV_LAST_DIR_UPD_KEY,
                        QDateTime());
    m_settings.setValue(NCONFIG_ADV, NCONFIG_ADV_DB_PWD_HASH_KEY,
                        NCONFIG_ADV_DB_PWD_HASH_VALUE);
    m_settings.setValue(NCONFIG_ADV, NCONFIG_ADV_FILE_BUFFER_SIZE_KEY,
                        NCONFIG_ADV_FILE_BUFFER_SIZE_VALUE);
    m_settings.saveToFile();
}

int NConfig::serverPort()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig", "m_dataMutex NConfig::serverPort");
    return m_serverPort;
}

const QDir NConfig::serverPub()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig", "m_dataMutex NConfig::serverPub");
    return m_serverPub;
}

bool NConfig::isSslServer()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig", "NConfig::isSslServer");
    return m_serverSsl;
}

const QString NConfig::AdminUser()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig", "m_dataMutex NConfig::AdminUser");
    return m_serverAdminUser;
}

const QString NConfig::AdminPassword()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig", "m_dataMutex NConfig::AdminPassword");
    return m_serverAdminPassword;
}

const NDirList NConfig::sharedDirectories()
{
    QMutexLocker locker(&m_dataMutex);
    return m_sharedDirectories;
}

int NConfig::addSharedDirectory(const NDir & dir)
{
    QMutexLocker locker(&m_dataMutex);
    m_sharedDirectories.append(dir);
    save();
    return m_sharedDirectories.count() - 1;
}

const NDir NConfig::modifySharedDirectory(int id, const NDir & dir)
{
    QMutexLocker locker(&m_dataMutex);
    //NDir & d = m_sharedDirectories.at(dir);
    m_sharedDirectories.replace(id, dir);
    save();
    return dir;
}

void NConfig::removeSharedDirectory(int id)
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig", "m_dataMutex NConfig::removeSharedDirectory");
    if (id < m_sharedDirectories.count())
    {
        m_sharedDirectories.removeAt(id);
        save();
    }
}

const NFileSuffixList & NConfig::fileSuffixes()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig","m_dataMutex NConfig::fileSuffixes");
    return m_fileSuffixes;
}

const QString & NConfig::referenceServer()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig","m_dataMutex NConfig::referenceServer");
    return m_referenceServer;
}

int NConfig::dirUpdateDelay()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig","m_dataMutex NConfig::dirUpdateDelay");
    return m_dirUpdateDelay;
}

const QDateTime NConfig::lastDirUpdate()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig","m_dataMutex NConfig::lastDirUpdate");
    return m_lastDirUpdate;
}
bool NConfig::isLastDirUpdateValid()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig","m_dataMutex NConfig::isLastDirUpdateValid");
    return m_lastDirUpdate.isValid() &&
            m_lastDirUpdate.addMSecs(m_dirUpdateDelay) > QDateTime::currentDateTime();
}

void NConfig::setLastDirUpdateDone()
{
    QMutexLocker locker(&m_dataMutex);
    NLOGD("NConfig","m_dataMutex NConfig::setLastDirUpdateDone");
    m_lastDirUpdate = QDateTime::currentDateTime();
    save();
}
void NConfig::invalidLastDirUpdate()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig","m_dataMutex NConfig::invalidLastDirUpdate");
    m_lastDirUpdate = QDateTime();
    save();
}

const QString NConfig::dirFingerPrint()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig","m_dataMutex NConfig::dirFingerPrint");
    return m_dirFingerPrint;
}

void NConfig::setDirFingerPrint(const QString & fp)
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig","m_dataMutex NConfig::setDirFingerPrint");
    m_dirFingerPrint = fp;
    save();
}

void NConfig::clearDirUpdateData()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig","m_dataMutex NConfig::clearDirUpdateData");
    m_dirFingerPrint.clear();
    m_lastDirUpdate = QDateTime();
    save();
}


quint64 NConfig::fileBufferSize()
{
    QMutexLocker locker(&m_dataMutex);
    return m_fileBufferSize;
}


void NConfig::onConfigFileChanged()
{
    if (!m_fileLoaded)
        return;

    if (load())
        emit configFileChanged();
}

void NConfig::dumpSharedDirectoriesConfig()
{
    QMutexLocker locker(&m_dataMutex);
    NLOGD("NConfig","m_dataMutex NConfig::dumpSharedDirectoriesConfig");
    if (m_sharedDirectories.isEmpty())
    {
        NLOGM("Server", tr("There is no shared directory"));
        return;
    }
    NLOGM("NServer", tr("Shared directories are: "));
    foreach (NDir NDir, m_sharedDirectories)
        NLOGM("Server", tr("  %1, %2, %3, %4").
              arg(NDir.dir().absolutePath()).
              arg(NDir.shared() ? "shared": "not shared").
              arg(NDir.recursive()? "recursive": "not recursive").
              arg(NDir.dir().exists() ? "": "not exists")
              );
}

const QByteArray & NConfig::dbPwdHashKey()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig", "m_dataMutex NConfig::dbPwdHashKey");
    return m_dbPwdHashKey;
}

const QSslConfiguration & NConfig::sslCfg()
{
    QMutexLocker locker(&m_dataMutex);
    //NLOGD("NConfig","m_dataMutex NConfig::sslCfg");
    return m_sslCfg;
}

void NConfig::genSslCfg()
{
    m_sslCfg = QSslConfiguration();

    if (!m_serverSsl)
        return;
    m_sslCfg.setProtocol(QSsl::AnyProtocol);
    m_sslCfg.setPeerVerifyMode (QSslSocket::VerifyNone);

    // Ca certificate
    // "/Users/sebastiend/Documents/dev/nawis/ca/sebastiend.crt"
    /*NLOGD("genSslCfg", m_serverSslCaCertificate);
	 NLOGD("genSslCfg", m_serverSslLocalCertificate);
	 NLOGD("genSslCfg", m_serverSslPrivateKey);
	 NLOGD("genSslCfg", m_serverSslPrivateKeyPwd);*/
    m_sslCfg.setCaCertificates(QSslCertificate::fromPath(m_serverSslCaCertificate));
    foreach (QSslCertificate cert, m_sslCfg.caCertificates())
    {
        NLOGD("NConfig::genSslCfg",
              QString("Certificat found > Organization: %1").arg(cert.issuerInfo(QSslCertificate::Organization)));
    }
    if (m_sslCfg.caCertificates().count() == 0 ){
        NLOGM("NConfig::genSslCfg",
              QString("No ssl certificat valid found: %1").arg(m_serverSslCaCertificate));
        m_sslCfg = QSslConfiguration();
        return;
    }

    // Private key
    // "/Users/sebastiend/Documents/dev/nawis/ca/sebastiend.key"
    QFile file(m_serverSslPrivateKey);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen()) {
        NLOGM("NConfig::genSslCfg",
              QString("Could not open private key: %1").arg(file.fileName()));
        m_sslCfg = QSslConfiguration();
        return;
    }
    // Private key password
    // "test"
    QSslKey key(&file, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, m_serverSslPrivateKeyPwd.toUtf8());
    if (key.isNull()){
        NLOGM("NConfig::genSslCfg", QString("No ssl private key valid found: %1").arg(m_serverSslPrivateKey));
        m_sslCfg = QSslConfiguration();
        return;
    }
    m_sslCfg.setPrivateKey(key);
    file.close();

    // Local certificate
    // Same as Ca Certificate fo self signed cert
    // "/Users/sebastiend/Documents/dev/nawis/ca/sebastiend.crt"
    file.setFileName(m_serverSslLocalCertificate);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen()) {
        NLOGM("NConfig::genSslCfg",
              QString("Could not open local certificate: %1").arg(file.fileName()));
        m_sslCfg = QSslConfiguration();
        return;
    }
    // Private key password
    // "test"
    QSslCertificate localCertificate(&file);
    if (!localCertificate.isValid()){
        NLOGM("NConfig::genSslCfg", "Local certificate is not valid: %1.");
        m_sslCfg = QSslConfiguration();
        return;
    }
    m_sslCfg.setLocalCertificate(localCertificate);
    file.close();
}


