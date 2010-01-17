/*
 * n_file_category.cpp - file category definition
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
#include <QtGlobal>
#include <QCoreApplication>

#include "n_file_category.h"

#define	FILE_CATEGORY_ALL         "all"
#define	FILE_CATEGORY_OTHER       "other"	
#define	FILE_CATEGORY_MOVIE       "movie"
#define	FILE_CATEGORY_MUSIC       "music"
#define	FILE_CATEGORY_PICTURE     "picture"
#define	FILE_CATEGORY_ARCHIVE     "archive"
#define	FILE_CATEGORY_DOCUMENT    "document" //Must be last one 

#define	FILE_CATEGORY_ALL_ID         0
#define	FILE_CATEGORY_OTHER_ID       1	
#define	FILE_CATEGORY_MOVIE_ID       2
#define	FILE_CATEGORY_MUSIC_ID       3
#define	FILE_CATEGORY_PICTURE_ID     4
#define	FILE_CATEGORY_ARCHIVE_ID     5
#define	FILE_CATEGORY_DOCUMENT_ID    6 //Must be last one


QString NFileCategory_n::fileCategoryName(NFileCategory_n::FileCategory fc)
{
	switch (fc) {
		case fcAll: return FILE_CATEGORY_ALL;
		case fcOther: return FILE_CATEGORY_OTHER;
		case fcMovie: return FILE_CATEGORY_MOVIE;
		case fcMusic: return FILE_CATEGORY_MUSIC;
		case fcPicture: return FILE_CATEGORY_PICTURE;
		case fcArchive: return FILE_CATEGORY_ARCHIVE;
		case fcDocument: return FILE_CATEGORY_DOCUMENT;
		default:
			Q_ASSERT(false);
			return FILE_CATEGORY_ALL;
	}
} 

int NFileCategory_n::fileCategoryId(NFileCategory_n::FileCategory fc)
{
	switch (fc) {
		case fcAll: return FILE_CATEGORY_ALL_ID;
		case fcOther: return FILE_CATEGORY_OTHER_ID;
		case fcMovie: return FILE_CATEGORY_MOVIE_ID;
		case fcMusic: return FILE_CATEGORY_MUSIC_ID;
		case fcPicture: return FILE_CATEGORY_PICTURE_ID;
		case fcArchive: return FILE_CATEGORY_ARCHIVE_ID;
		case fcDocument: return FILE_CATEGORY_DOCUMENT_ID;
		default:
			Q_ASSERT(false);
			return FILE_CATEGORY_ALL_ID;
	}
} 


NFileCategory_n::FileCategory NFileCategory_n::toFileCategory(int fc,
	NFileCategory_n::FileCategory def)
{
	switch (fc) {
		case FILE_CATEGORY_ALL_ID: return fcAll;
		case FILE_CATEGORY_OTHER_ID: return fcOther;
		case FILE_CATEGORY_MOVIE_ID: return fcMovie;
		case FILE_CATEGORY_MUSIC_ID: return fcMusic;
		case FILE_CATEGORY_PICTURE_ID: return fcPicture;
		case FILE_CATEGORY_ARCHIVE_ID: return fcArchive;
		case FILE_CATEGORY_DOCUMENT_ID: return fcDocument;
		default:
			Q_ASSERT(false);
			return def;
	}
}


NFileCategory_n::FileCategory NFileCategory_n::toFileCategory(const QString & fc,
	NFileCategory_n::FileCategory def)
{
	if (fc.compare(FILE_CATEGORY_ALL, Qt::CaseInsensitive) == 0)
		return NFileCategory_n::fcAll;
	
	if (fc.compare(FILE_CATEGORY_OTHER, Qt::CaseInsensitive) == 0)
		return NFileCategory_n::fcOther;
	
	if (fc.compare(FILE_CATEGORY_MOVIE, Qt::CaseInsensitive) == 0)
		return NFileCategory_n::fcMovie;
	
	if (fc.compare(FILE_CATEGORY_MUSIC, Qt::CaseInsensitive) == 0)
		return NFileCategory_n::fcMusic;
	
	if (fc.compare(FILE_CATEGORY_PICTURE, Qt::CaseInsensitive) == 0)
		return NFileCategory_n::fcPicture;
	
	if (fc.compare(FILE_CATEGORY_ARCHIVE, Qt::CaseInsensitive) == 0)
		return NFileCategory_n::fcArchive;

	if (fc.compare(FILE_CATEGORY_DOCUMENT, Qt::CaseInsensitive) == 0)
		return NFileCategory_n::fcDocument;

	Q_ASSERT(false);
	return def;	
}
