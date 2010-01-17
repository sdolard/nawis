/*
 *  n_metadata.cpp
 *  nawis
 *
 *  Created by Sébastien Dolard on 05/09/09.
 *  Copyright 2009 Sébastien Dolard. All rights reserved.
 *
 */

// Qt
#include <QFile>
#include <QtDebug>
#include <QStringList>
#include <QFileInfo>

// tatglib
#include <taglib/fileref.h>
#include <taglib/tag.h>

// included header files
#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/iptc.hpp>
#include <iostream>
#include <iomanip>

// App
#include "n_log.h"

#include "n_metadata.h"

NMetadata::NMetadata(){
}

void NMetadata::setFileName(const QString & fileName)
{
	clear();
	
	m_fileName = fileName;	
	QFileInfo fi(m_fileName);
	/*
	 TagLib is a library for reading and editing the meta-data of several popular 
	 audio formats. Currently it supports both ID3v1 and ID3v2 for MP3 files, 
	 Ogg Vorbis comments and ID3 tags and Vorbis comments in 
	 FLAC, MPC, Speex, WavPack and TrueAudio files.
	 */
	if ((fi.suffix().compare("mp3", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("mp4", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("wma", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("ogg", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("flac", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("mpc", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("wav", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("Wv", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("TTA", Qt::CaseInsensitive) == 0))
	{
		getTaglibData();
	}
	
	/*
	 Supported_image_formats
	 http://dev.exiv2.org/wiki/exiv2/Supported_image_formats
	 Type  	 Exif mode  	 IPTC mode  	 XMP mode  	 Comment mode
	 JPEG 	ReadWrite 	ReadWrite 	ReadWrite 	ReadWrite
	 EXV 	ReadWrite 	ReadWrite 	ReadWrite 	ReadWrite
	 CR2 	Read 	Read 	Read 	-
	 CRW 	ReadWrite 	- 	- 	ReadWrite
	 MRW 	Read 	Read 	Read 	-
	 TIFF 	ReadWrite 	ReadWrite 	ReadWrite 	-
	 DNG 	ReadWrite 	ReadWrite 	ReadWrite 	-
	 NEF 	ReadWrite 	ReadWrite 	ReadWrite 	-
	 PEF 	ReadWrite 	ReadWrite 	ReadWrite 	-
	 ARW 	Read 	Read 	Read 	-
	 SR2 	Read 	Read 	Read 	-
	 ORF 	Read 	Read 	Read 	-
	 RW2 	Read 	Read 	Read 	-
	 PNG 	ReadWrite 	ReadWrite 	ReadWrite 	ReadWrite
	 RAF 	Read 	Read 	Read 	-
	 XMP 	- 	- 	ReadWrite 	-
	 GIF 	- 	- 	- 	-
	 PSD 	ReadWrite 	ReadWrite 	ReadWrite 	-
	 TGA 	- 	- 	- 	-
	 BMP 	- 	- 	- 	-
	 JP2 	ReadWrite 	ReadWrite 	ReadWrite 	- 
	 */
	
	if ((fi.suffix().compare("JPG", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("JPEG", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("EXV", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("CR2", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("CRW", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("MRW", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("TIFF", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("DNG", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("NEF", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("PEF", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("ARW", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("SR2", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("ORF", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("RW2", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("PNG", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("RAF", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("PSD", Qt::CaseInsensitive) == 0) ||
		(fi.suffix().compare("JP2", Qt::CaseInsensitive) == 0))
	{
		getExiv2Data();
		getIptcData();
		
		/*printExiv2Data();
		printIptcData();*/
	}
}

void NMetadata::getTaglibData()
{	
	TagLib::FileRef f(QFile::encodeName(m_fileName));
	
	if(!f.isNull() && f.tag()) {
		
		TagLib::Tag *tag = f.tag();
		m_title = TStringToQString(tag->title()).trimmed();
		m_artist = TStringToQString(tag->artist()).trimmed();
		m_album = TStringToQString(tag->album()).trimmed();
		m_comment = TStringToQString(tag->comment()).trimmed();		
		m_genre = TStringToQString(tag->genre()).trimmed();
		
		m_year = tag->year();
		uint tmp = 0; tmp -=1; // tagLibBug ?
		if (m_year == tmp)
			m_year = 0;
		m_trackNumber = tag->track();
	}
	
	if(!f.isNull() && f.audioProperties()) {
		TagLib::AudioProperties *properties = f.audioProperties();
		m_duration = properties->length();
	}
}

void NMetadata::printTaglibData()
{
	TagLib::FileRef f(QFile::encodeName(m_fileName));
	char secondsString[3];
	
	if(!f.isNull() && f.tag()) {
		
		TagLib::Tag *tag = f.tag();
		
		std::cout << "-- TAG --" << std::endl;
		std::cout << "title   - \"" << tag->title()   << "\"" << std::endl;
		std::cout << "artist  - \"" << tag->artist()  << "\"" << std::endl;
		std::cout << "album   - \"" << tag->album()   << "\"" << std::endl;
		std::cout << "year    - \"" << tag->year()    << "\"" << std::endl;
		std::cout << "comment - \"" << tag->comment() << "\"" << std::endl;
		std::cout << "track   - \"" << tag->track()   << "\"" << std::endl;
		std::cout << "genre   - \"" << tag->genre()   << "\"" << std::endl;
	}
	
	if(!f.isNull() && f.audioProperties()) {
		
		TagLib::AudioProperties *properties = f.audioProperties();
		
		int seconds = properties->length() % 60;
		int minutes = (properties->length() - seconds) / 60;
		sprintf(secondsString, "%02i", seconds);
		
		std::cout << "-- AUDIO --" << std::endl;
		std::cout << "bitrate     - " << properties->bitrate() << std::endl;
		std::cout << "sample rate - " << properties->sampleRate() << std::endl;
		std::cout << "channels    - " << properties->channels() << std::endl;
		std::cout << "length      - " << minutes << ":" << secondsString << std::endl;
	}
}

void NMetadata::getExiv2Data()
{	
	try
	{
		Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)QFile::encodeName(m_fileName));
		Q_ASSERT(image.get() != 0);
		image->readMetadata();
		
		Exiv2::ExifData &exifData = image->exifData();
		if (exifData.empty()) {
			NLOGD("NMetadata::getExiv2Data", QString("%1: No Exif data found").arg(m_fileName));
			return;
		}
		
		Exiv2::ExifData::const_iterator end = exifData.end();
		for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
			
			/* http://www.exiv2.org/tags.html
			 */
			switch (i->tag()) {
   case 0x013b:
				// This tag records the name of the camera owner, photographer or
				// image creator. The detailed format is not specified, but it is
				// recommended that the information be written as in the example
				// below for ease of Interoperability. When the field is left blank,
				// it is treated as unknown. Ex.) "Camera owner, John Smith;
				// Photographer, Michael Brown; Image creator, Ken James"
				// Exif.Image.Artist Ascii
				m_artist = QString::fromStdString(i->value().toString()).trimmed();
				break;

   case 0x8298:
				// Copyright information. In this standard the tag is used to
				// indicate both the photographer and editor copyrights. It is
				// the copyright notice of the person or organization claiming
				// rights to the image. The Interoperability copyright statement
				// including date and rights should be written in this field;
				// e.g., "Copyright, John Smith, 19xx. All rights reserved.".
				// In this standard the field records both the photographer and
				// editor copyrights, with each recorded in a separate part of
				// the statement. When there is a clear distinction between the
				// photographer and editor copyrights, these are to be written in
				// the order of photographer followed by editor copyright,
				// separated by NULL (in this case since the statement also ends
				// with a NULL, there are two NULL codes). When only the
				// photographer copyright is given, it is terminated by one NULL
				// code . When only the editor copyright is given, the photographer
				// copyright part consists of one space followed by a terminating
				// NULL code, then the editor copyright is given. When the field
				// is left blank, it is treated as unknown.
				// Exif.Image.Copyright Ascii
				m_copyright = QString::fromStdString(i->value().toString()).trimmed();
				break;

   case 0x9003:
				// The date and time when the original image data was generated.
				// For a digital still camera the date and time the picture was
				// taken are recorded.
				// Exif.Photo.DateTimeOriginal 0x9003 Ascii 20  2008:07:18 18:58:02
				m_dateTimeOriginal = QDateTime::fromString(QString::fromStdString(i->value().toString()), "yyyy:MM:dd hh:mm:ss");
				if (m_dateTimeOriginal.isValid())
					m_year = m_dateTimeOriginal.date().year();
				break;

   case 0xa002:
				// Information specific to compressed data. When a compressed
				// file is recorded, the valid width of the meaningful image must
				// be recorded in this tag, whether or not there is padding data
				// or a restart marker. This tag should not exist in an uncompressed file.
				// Exif.Photo.PixelXDimension 0xa002 Long        1  2304
				m_width = i->value().toLong();
				break;

   case 0xa003:
				// Information specific to compressed data. When a compressed
				// file is recorded, the valid height of the meaningful image
				// must be recorded in this tag, whether or not there is padding
				// data or a restart marker. This tag should not exist in an
				// uncompressed file. Since data padding is unnecessary in the
				// vertical direction, the number of lines recorded in this valid
				// image height tag will in fact be the same as that recorded in the SOF.
				// Exif.Photo.PixelYDimension 0xa003 Long        1  3072
				m_height = i->value().toLong();
				break;

   case 0x010f:
				// The manufacturer of the recording equipment. This is the
				// manufacturer of the DSC, scanner, video digitizer or other
				// equipment that generated the image. When the field is left blank,
				// it is treated as unknown.
				// Exif.Image.Make 0x010f Ascii       6  Canon
				m_make = QString::fromStdString(i->value().toString()).trimmed();
				break;

   case 0x0110:
				// The model name or model number of the equipment. This is the
				// model name or number of the DSC, scanner, video digitizer or
				// other equipment that generated the image. When the field is
				// left blank, it is treated as unknown.
				// Exif.Image.Model 0x0110 Ascii      26  Canon DIGITAL IXUS 850 IS
				m_model = QString::fromStdString(i->value().toString()).trimmed();
				break;

   case 0x0000:
				// Exif.GPSInfo.GPSVersionID                    0x0000 Byte        4  2 0 0 0
				m_GPSVersion = QString::fromStdString(i->value().toString()).trimmed();
				break;

   case 0x0001:
				// Exif.GPSInfo.GPSLatitudeRef                  0x0001 Ascii       2  N
				// Indicates whether the latitude is north or south latitude.
				// The ASCII value 'N' indicates north latitude, and 'S' is south latitude.
				m_GPSLatitudeRef = QString::fromStdString(i->value().toString()).trimmed();
				break;

   case 0x0002:
				// Exif.GPSInfo.GPSLatitude                     0x0002 Rational    3  33/1 52/1 129675/4096
				// Indicates the latitude. The latitude is expressed as three
				// RATIONAL values giving the degrees, minutes, and seconds,
				// respectively. When degrees, minutes and seconds are expressed,
				// the format is dd/1,mm/1,ss/1. When degrees and minutes are used
				// and, for example, fractions of minutes are given up to two
				// decimal places, the format is dd/1,mmmm/100,0/1.
				m_GPSLatitude = QString::fromStdString(i->value().toString()).trimmed();
				break;

   case 0x0003:
				// Exif.GPSInfo.GPSLongitudeRef                 0x0003 Ascii       2  W
				// Indicates whether the longitude is east or west longitude.
				// ASCII 'E' indicates east longitude, and 'W' is west longitude.
				m_GPSLongitudeRef = QString::fromStdString(i->value().toString()).trimmed();
				break;

   case 0x0004:
				// Exif.GPSInfo.GPSLongitude                    0x0004 Rational    3  116/1 18/1 23882/4096
				// Indicates the longitude. The longitude is expressed as three
				// RATIONAL values giving the degrees, minutes, and seconds,
				// respectively. When degrees, minutes and seconds are expressed,
				// the format is ddd/1,mm/1,ss/1. When degrees and minutes are
				// used and, for example, fractions of minutes are given up to
				// two decimal places, the format is ddd/1,mmmm/100,0/1.
				m_GPSLongitude = QString::fromStdString(i->value().toString()).trimmed();
				break;

   case 0x0005:
				// Exif.GPSInfo.GPSAltitudeRef                  0x0005 Byte        1  0
				// Indicates the altitude used as the reference altitude. If the
				// reference is sea level and the altitude is above sea level,
				// 0 is given. If the altitude is below sea level, a value of 1
				// is given and the altitude is indicated as an absolute value in
				// the GSPAltitude tag. The reference unit is meters. Note that
				// this tag is BYTE type, unlike other reference tags.
				m_GPSAltitudeRef = QString::fromStdString(i->value().toString()).trimmed();
				break;

   case 0x0006:
				// Exif.GPSInfo.GPSAltitude                     0x0006 Rational    1  304/1
				// Indicates the altitude based on the reference in GPSAltitudeRef.
				// Altitude is expressed as one RATIONAL value. The reference unit is meters.
				m_GPSAltitude = QString::fromStdString(i->value().toString()).trimmed();
				break;

   case 0x00012:
				// Exif.GPSInfo.GPSMapDatum                     0x0012 Ascii       7  WGS-84
				m_GPSMapDatum = QString::fromStdString(i->value().toString()).trimmed();
				break;

   default:
				continue;
			};
		}		
	}
	catch (Exiv2::AnyError& e) {
		NLOGD("Caught Exiv2 exception", QString::fromStdString(e.what()));
	}
}

void NMetadata::printExiv2Data()
{
	try
	{
		Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)QFile::encodeName(m_fileName));
		Q_ASSERT(image.get() != 0);
		image->readMetadata();
		
		Exiv2::ExifData &exifData = image->exifData();
		if (exifData.empty()) {
			NLOGD("NMetadata::printExiv2Data", QString("%1: No Exif data found").arg(m_fileName));
			return;
		}
		
		Exiv2::ExifData::const_iterator tend = exifData.end();
		for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != tend; ++i) {
			const char* tn = i->typeName();
			std::cout << std::setw(44) << std::setfill(' ') << std::left
					<< i->key() << " "
					<< "0x" << std::setw(4) << std::setfill('0') << std::right
					<< std::hex << i->tag() << " "
					<< std::setw(9) << std::setfill(' ') << std::left
					<< (tn ? tn : "Unknown") << " "
					<< std::dec << std::setw(3)
					<< std::setfill(' ') << std::right
					<< i->count() << "  "
					<< std::dec << i->value()
					<< "\n";
		}
	}
	catch (Exiv2::AnyError& e) {
		NLOGD("Caught Exiv2 exception", QString::fromStdString(e.what()));
	}
}

void NMetadata::getIptcData()
{
	/*
	 http://owl.phy.queensu.ca/~phil/exiftool/faq.html#Q10
	 IPTC: The value of the IPTC:CodedCharacterSet tag determines how the internal 
	 IPTC string values are interpreted. If CodedCharacterSet exists and has a value of 
	 "UTF8" (or "ESC % G") then string values are assumed to be stored as UTF‑8, 
	 otherwise Windows Latin1 (cp1252) coding is assumed by default, but this can 
	 be changed with "-charset iptc=CHARSET". When reading, these strings are 
	 translated to UTF‑8 by default, or to the character set specified by the -charset 
	 or -L option. When writing, the inverse translations are performed. No translation 
	 is done if the internal (IPTC) and external (ExifTool) character sets are the 
	 same. Note that ISO 2022 character set shifting is not supported. Instead, a 
	 warning is issued and the string is not translated if an ISO 2022 shift code 
	 is found. See the IPTC specification for more information about IPTC character 
	 coding.
	 
	 ExifTool may be used to convert IPTC values to a different internal encoding. 
	 To do this, all IPTC tags must be rewritten along with the desired value of 
	 CodedCharacterSet. For example, the following command changes the internal 
	 IPTC encoding to UTF‑8:
	 
	 exiftool -tagsfromfile @ -iptc:all -codedcharacterset=utf8 a.jpg
	 
	 and this command changes it back from UTF‑8 to Windows Latin1 (cp1252):
	 
	 exiftool -tagsfromfile @ -iptc:all -codedcharacterset= a.jpg
	 
	 or this command changes it back from UTF‑8 to Windows Latin2 (cp1250):
	 
	 exiftool -tagsfromfile @ -iptc:all -codedcharacterset= -charset iptc=latin2 a.jpg
	 
	 But note that unless UTF‑8 is used, applications have no reliable way to determine the IPTC character encoding.
	 
	 */
	
	/*
	 Iptc.Envelope.CharacterSet                   0x005a Undefined   3  27 37 71 
	 Iptc.Application2.RecordVersion              0x0000 Short       1  2
	 Iptc.Application2.Keywords                   0x0019 String      4  test
	 Iptc.Application2.Keywords                   0x0019 String      7  restest
	 Iptc.Application2.Keywords                   0x0019 String      3  2é
	 Iptc.Application2.Keywords                   0x0019 String      9  日本国
	 */
	// TODO: manage other charset than UTF8
	try
	{
		Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)QFile::encodeName(m_fileName));
		Q_ASSERT(image.get() != 0);
		image->readMetadata();
		
		Exiv2::IptcData &iptcData = image->iptcData();
		if (iptcData.empty()) {
			NLOGD("NMetadata::getIptcData", QString("%1: No IPTC data found in the file").arg(m_fileName));
			return;
		}
		
		Exiv2::IptcData::iterator end = iptcData.end();
		for (Exiv2::IptcData::iterator i = iptcData.begin(); i != end; ++i) {
			switch (i->tag()) {
   case 0x0019:{
		   m_comment = m_comment.isEmpty() ? "" : m_comment + " ";
		   QString tmp = QString::fromUtf8(i->value().toString().data(), i->value().toString().length());
		   m_comment += tmp.trimmed();
		   break;
	   }
   case 0x005a:{
		   // Identifies city of object data origin according to guidelines established by the provider.
		   m_city = QString::fromUtf8(i->value().toString().data(), i->value().toString().length());
		   break;
	   }
   case 0x005f:{
		   // Identifies Province/State of origin according to guidelines established by the provider.
		   m_provinceState = QString::fromUtf8(i->value().toString().data(), i->value().toString().length());
		   break;
	   }
   case 0x0065:{
		   // Provides full
		   m_countryName = QString::fromUtf8(i->value().toString().data(), i->value().toString().length());
		   break;
	   }
   default:
	   break;
   }
		}
	}
	catch (Exiv2::AnyError& e) {
		NLOGD("Caught IPTC exception", QString::fromStdString(e.what()));
	}
}

void NMetadata::printIptcData()
{
	try
	{
		Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const char*)QFile::encodeName(m_fileName));
		Q_ASSERT(image.get() != 0);
		image->readMetadata();
		
		Exiv2::IptcData &iptcData = image->iptcData();
		if (iptcData.empty()) {
			NLOGD("NMetadata::printIptcData", QString("%1: No IPTC data found in the file").arg(m_fileName));
			return;
		}
		
		Exiv2::IptcData::iterator end = iptcData.end();
		for (Exiv2::IptcData::iterator md = iptcData.begin(); md != end; ++md) {
			std::cout << std::setw(44) << std::setfill(' ') << std::left
					<< md->key() << " "
					<< "0x" << std::setw(4) << std::setfill('0') << std::right
					<< std::hex << md->tag() << " "
					<< std::setw(9) << std::setfill(' ') << std::left
					<< md->typeName() << " "
					<< std::dec << std::setw(3)
					<< std::setfill(' ') << std::right
					<< md->count() << "  "
					<< std::dec << md->value()
					<< std::endl;
		}
	}
	catch (Exiv2::AnyError& e) {
		NLOGD("Caught IPTC exception", QString::fromStdString(e.what()));
	}
}

const QString & NMetadata::artist() const {
	return m_artist;
}

const QString & NMetadata::album() const {
	return m_album;
}

const QString & NMetadata::title() const {
	return m_title;
}

const QString & NMetadata::genre() const {
	return m_genre;
}

uint NMetadata::trackNumber() const {
	return m_trackNumber;
}

const QString & NMetadata::comment() const {
	return m_comment;
}

uint NMetadata::year() const {
	return m_year;
}

uint NMetadata::duration() const
{
	return m_duration;
}

const QDateTime & NMetadata::dateTimeOriginal() const{
	return m_dateTimeOriginal;
}
const QString & NMetadata::copyright() const
{
	return m_copyright;
}

qint64 NMetadata::width() const{
	return m_width;
}

qint64 NMetadata::height()const{
	return m_height;
}

const QString & NMetadata::make() const{
	return m_make;
}

const QString & NMetadata::model() const{
	return m_model;
}
const QString NMetadata::longitude() const
{
	return QString("%1 %2").arg(m_GPSLongitude).arg(m_GPSLongitudeRef);
}

const QString NMetadata::latitude() const
{
	return QString("%1 %2").arg(m_GPSLatitude).arg(m_GPSLatitudeRef);
}

int NMetadata::altitude() const
{
	return m_GPSAltitudeRef.toInt() + m_GPSAltitude.toInt();
}

const QString & NMetadata::city() const
{
	return m_city;
}
const QString & NMetadata::provinceState() const
{
	return m_provinceState;
}

const QString & NMetadata::country() const
{
	return m_countryName;
}

void NMetadata::clear()
{
	m_fileName.clear();
	
	// Id3 tags
	m_artist.clear();
	m_comment.clear();
	m_year = 0;
	
	// Id3 tags
	m_album.clear();
	m_title.clear();
	m_genre.clear();
	m_trackNumber = 0;
	m_duration = 0;
	
	// Exiv2
	m_dateTimeOriginal = QDateTime();
	m_copyright.clear();
	m_width = 0;
	m_height = 0;
	m_make.clear();
	m_model.clear();
	
	// GPS
	m_GPSVersion.clear();
	m_GPSLatitudeRef.clear();
	m_GPSLatitude.clear();
	m_GPSLongitudeRef.clear();
	m_GPSLongitude.clear();
	m_GPSAltitudeRef.clear();
	m_GPSAltitude.clear();
	m_GPSMapDatum.clear();

	//IPTC
	m_city.clear();
	m_provinceState.clear();
	m_countryName.clear();
}

