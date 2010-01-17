/*
 * n_convert.h - convertion methods
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
#ifndef N_CONVERT_H
#define N_CONVERT_H

//Qt
#include <QString>

namespace NConvert_n
{
	// Max unit is MB  		
	const QString byteToHuman(qint64 bytes);

	// Max unit is minute 	
	const QString durationToHuman(qint64 ms);

	// Return rate for 1 second
	const QString rateForOneSecToHuman(qint64 bytes, qint64 ms);

};

#endif
