/*
 *  n_stream_writer.h
 *  nawis
 *
 *  Created by Sébastien Dolard on 12/09/09.
 *  Copyright 2009 Sébastien Dolard. All rights reserved.
 *
 */
#ifndef N_STREAM_WRITER
#define N_STREAM_WRITER

// Qt
#include <QByteArray>
#include <QXmlStreamWriter>
#include <QString>

// app
#include "n_client_session.h"

// TODO delete this
class KsStreamWriter{
public:

    KsStreamWriter(QByteArray *data, const NClientSession & session);
    ~KsStreamWriter();

    void writeStartToken(const QString & token);
    void writeToken(const QString & token, const QString & value = "");
    void writeTokenBool(const QString & token, bool value);
    void writeTokenInt(const QString & token, int value);
    void writeEndToken();

private:
    QByteArray       *m_data;
    QXmlStreamWriter *m_xsw;
};

#endif //N_STREAM_WRITER
