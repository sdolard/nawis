/*
 *  n_string.h
 *  nawis
 *
 *  Created by Sébastien Dolard on 04/02/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include <QString>

namespace NString_n {
	int naturalCompare(const QString & a, const QString & b, 
		Qt::CaseSensitivity cs = Qt::CaseSensitive);
};
