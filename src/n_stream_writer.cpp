/*
 *  n_stream_writer.cpp
 *  nawis
 *
 *  Created by Sébastien Dolard on 12/09/09.
 *  Copyright 2009 Sébastien Dolard. All rights reserved.
 *
 */
// Qt
#include <QVariant>

#include "n_stream_writer.h"

KsStreamWriter::KsStreamWriter(QByteArray *data, const NClientSession &)
	:m_data(data)
{
	Q_ASSERT(m_data);
	
	m_xsw = new QXmlStreamWriter(m_data);
	m_xsw->writeStartDocument("1.0", true);
	writeStartToken("dataset");
	writeStartToken("response");
	///writeToken("request", session.urlPath());
}

KsStreamWriter::~KsStreamWriter()
{
	m_xsw->writeEndDocument();
	delete m_xsw;
}

void KsStreamWriter::writeStartToken(const QString & token)
{
	m_xsw->writeStartElement(token);
}

void KsStreamWriter::writeToken(const QString & token, const QString & value)
{
	writeStartToken(token);
	m_xsw->writeCharacters(value);
	writeEndToken();
}

void KsStreamWriter::writeTokenBool(const QString & token, bool value)
{
	writeToken(token, QVariant(value).toString());
}

void KsStreamWriter::writeTokenInt(const QString & token, int value)
{
	writeToken(token, QString::number(value));
}

void KsStreamWriter::writeEndToken()
{
	m_xsw->writeEndElement();	
}
