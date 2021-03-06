/*
 *  n_single_app.h - Single application class
 * nawis
 * Created by Sébastien Dolard on 17/05/09.
 * Copyright 2009. All rights reserved.
 *
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

#ifndef N_SINGLE_APPLICATION_H
#define N_SINGLE_APPLICATION_H

#include <QSharedMemory>

class NSingleApplication
{
public:
    NSingleApplication(const QString & appName);
    ~NSingleApplication();

    bool alreadyExistsAnInstance();
    bool attach();
    bool detach();

private:
    QSharedMemory m_sharedMemory;
};
#endif // N_SINGLE_APPLICATION_H
