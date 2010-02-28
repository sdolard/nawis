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

NSingleApplication::NSingleApplication(const QString & appName)

{
    m_sharedMemory.setKey("n_single_application_" + appName);
}

NSingleApplication::~NSingleApplication()
{
    detach();
}

bool NSingleApplication::attach()
{
    /*if (m_sharedMemory.attach())
    {
        if (detach())
            return
    }*/

    // Create shared memory.
    if (!m_sharedMemory.create(1))
    {
        qDebug("Unable to create single application instance.\nError: %s\nStopping",
               qPrintable(m_sharedMemory.errorString()));
        return false;
    }
    return true;
}

void NSingleApplication::detach()
{
    qDebug("Application instance already exists. Stopping...");
    return m_sharedMemory.detach();
}

bool NSingleApplication::alreadyExistsAnInstance()
{

    if (m_sharedMemory.create(1))
    {
        return false;
    }

    if (m_sharedMemory.isAttached()) // This is the instance, return false
        return false;

    if (m_sharedMemory.attach())
    {
         qDebug("Application instance already exists.");
         m_sharedMemory.detach();
         return false;
    }
}
