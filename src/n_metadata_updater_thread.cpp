/*
 *  n_metadata_thread_updater.cpp
 *  nawis
 *
 *  Created by Sébastien Dolard on 04/09/09.
 *  Copyright Sébastien Dolard 2009. All rights reserved.
 *
 */
#include <QFileInfo>

#include "n_log.h"
#include "n_database.h"

#include "n_metadata_updater_thread.h"

NMetadataUpdaterThread::NMetadataUpdaterThread(QObject * parent)
:NThread(parent)
{	
}

NMetadataUpdaterThread::~NMetadataUpdaterThread()
{
}

void NMetadataUpdaterThread::run()
{
	NLOGM("NServer", tr("Metadata extraction..."));
	Q_ASSERT(NDB.beginTransaction());
	forever	{
		if (isStopping())
			break;
		
		if (!getFile())
			break;
	}
	NDB.commitTransaction();
	NLOGM("NServer", tr("Metadata extraction done."));
}

bool NMetadataUpdaterThread::getFile()
{
	// Get a file with no metadata in database
	QString absoluteFilePath;
	int fileId;
	
	if (!NDB.getFileWithNoMetadata(absoluteFilePath, fileId))
	{
		emit metadataUpdateDone();
		return false;
	}
	
	const QFileInfo fi(absoluteFilePath);
	if (!fi.exists())
	{
		Q_ASSERT_X(fi.absoluteFilePath().isEmpty(), "getFile", "invalid filename");
		emit metadataUpdateDone();
		return false;
	}
	
	//NLOGD("NMetadataUpdaterThread::getFile", fi.absoluteFilePath());
	m_metadata.setFileName(fi.absoluteFilePath());
	NDB.setMetadata(fileId, m_metadata);
	return true;
}

