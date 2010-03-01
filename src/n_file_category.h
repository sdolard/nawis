/*
 * n_file_category.h - file category definition
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

#ifndef N_FILE_CATEGORY_H
#define N_FILE_CATEGORY_H

#include <QString>

namespace NFileCategory_n {
    enum FileCategory
    {
        fcAll,
        fcOther,
        fcMovie,
        fcMusic,
        fcPicture,
        fcArchive,
        fcDocument //Must be last one or look for it in source code to update "for loop"
    };
    inline FileCategory operator++( FileCategory &fc, int )
    {
        return fc = (NFileCategory_n::FileCategory)(fc + 1);
    }


    QString fileCategoryName(FileCategory fc);
    int fileCategoryId(FileCategory fc);
    FileCategory toFileCategory(int fc, FileCategory def = fcAll);
    FileCategory toFileCategory(const QString & fc, FileCategory def = fcAll);
};


#endif // N_FILE_CATEGORY_H

