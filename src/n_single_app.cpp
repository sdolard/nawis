/*
 *  n_single_app.cpp - Single application class
 * nawis
 * Created by Sébastien Dolard on 17/05/09.
 * Copyright 2009. All rights reserved.
 *
 * This code derive from SingleApplication class available on 
 * http://wiki.qtcentre.org/index.php?title=SingleApplication
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

#include "n_single_app.h"

KsSingleApplication::KsSingleApplication(){}

bool KsSingleApplication::attach(const QString & appName)
{
	m_sharedMemory.setKey("n_single_application_" + appName);
	if (m_sharedMemory.attach())
	{
		qDebug("Application instance already exists. Stopping...");
		m_sharedMemory.detach();
		return false;
	}
	// create shared memory.
	if (!m_sharedMemory.create(1))
	{
		qDebug("Unable to create single application instance.\nError: %s\nStopping", 
			   qPrintable(m_sharedMemory.errorString()));
		return false;
	}
	return true;
}
