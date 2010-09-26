/*
 *  n_metadata.h
 *  nawis
 *
 *  Created by Sébastien Dolard on 05/09/09.
 *  Copyright 2009 Sébastien Dolard. All rights reserved.
 *
 */

#ifndef N_METADATA_H
#define N_METADATA_H

// Qt
#include <QString>
#include <QDateTime>
#include <QByteArray>
#include <QFileInfo>


/* TODO: seperate in At least 3 classes
*   - NMetadata
*   - NPictureMetadata
*   - NMPegMetadata
*/

class NMetadata {
public:
    NMetadata(); //ctor

    void setFileName(const QString & fileName);
    void clear();

    // Id3 and Exiv2
    const QString & artist() const;

    // Id3
    const QString & album() const;
    const QString & title() const;
    const QString & genre() const;
    const QString & comment() const;
    uint year() const;
    uint trackNumber() const;
    // return duraton in ms
    uint duration() const;
    bool hasID3Picture() const;

    // Exiv2
    const QDateTime & dateTimeOriginal() const;
    const QString & copyright() const;
    const QString & userComment() const;
    qint64 width() const;
    qint64 height() const;
    const QString & make() const;
    const QString & model() const;

    // GPS Exiv2
    const QString longitude() const;
    const QString latitude() const;
    int altitude() const;

    // IPTC
    const QString & city() const;
    const QString & provinceState() const;
    const QString & country() const;

    /* return an empty QByteArray if failed
	*/
    static bool getID3Picture(const QString & fileName, QByteArray & ba, QString & mimeType);

    static bool isTaglibCompat(QFileInfo &fi);
    static bool isExiv2Compat(QFileInfo &fi);
    static bool isIptcCompat(QFileInfo &fi);

private:
    QString   m_fileName;

    // ID3 and Exiv2
    QString   m_artist;
    QString   m_comment;
    uint      m_year;

    // ID3
    QString   m_album;
    QString   m_title;
    QString   m_genre;
    uint      m_trackNumber;
    uint      m_duration;
    bool      m_hasID3Picture;

    // Exiv2
    QDateTime m_dateTimeOriginal;
    QString   m_copyright;
    qint64    m_width;
    qint64    m_height;
    QString   m_make;
    QString   m_model;
    // GPS
    QString   m_GPSVersion;
    // N (north) | S (south)
    QString   m_GPSLatitudeRef;
    // Latitude: 33°  52'  31,66"       N
    //           33/1 52/1 129675/4096
    QString   m_GPSLatitude;
    // E (east) | W (west)
    QString   m_GPSLongitudeRef;
    // Longitude: 116°  18'  5,83"      O
    //	          116/1 18/1 23882/4096
    QString   m_GPSLongitude;

    QString   m_GPSAltitudeRef;
    // Real altitude = m_GPSAltitude - m_GPSAltitudeRef
    QString   m_GPSAltitude;
    // Geodesic system
    QString   m_GPSMapDatum;

    // IPTC
    QString   m_city;
    QString   m_provinceState;
    QString   m_countryName;

    /*
	 TagLib is a library for reading and editing the meta-data of several popular 
	 audio formats. Currently it supports both ID3v1 and ID3v2 for MP3 files, 
	 Ogg Vorbis comments and ID3 tags and Vorbis comments in FLAC, MPC, Speex, 
	 WavPack and TrueAudio files. 
	 */
    void getTaglibData();
    void printTaglibData();

    void getExiv2Data();
    void printExiv2Data();

    void printIptcData();
    void getIptcData();

    bool hasAttachedPictureFrame(const QString & fileName);
};

#endif //N_METADATA_H
