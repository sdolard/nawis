/*
 *  n_settings.cpp
 *  nawis
 *
 *  Created by Sébastien Dolard on 12/01/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

// Qt
#include <QDir>
#include <QCoreApplication>
#include <QTextStream>

// App
#include "n_path.h"
#ifndef UNIT_TEST
#include "n_log.h"
#endif
#include "n_string.h"

#include "n_settings.h"


/*
 * Reserved key work
 */
const QString N_EQUAL             = "=";
const QString N_SEPARATOR         = " ";
const QString N_SPACE             = " ";
const QString N_GROUP             = "group";
const QString N_DEFINE            = "def";
const QString N_UNDEFINE          = "undef";
const QString N_DBQUOTE           = "\"";
const QString N_COMMENT_SEPARATOR = "#";
const QString N_PROTECTED         = "\\";


NSettingCommand::NSettingCommand(const QString & context, const NSettingAction action, const QString & key,
                                 const QVariant & value, const QString & comment)
                                     :m_context(context), m_action(action), m_key(key), m_value(value),
                                     m_comment(comment)
{
    m_type = NSettingCommand::stListItem;
}


NSettingCommand::NSettingCommand(const QString & context, quint32 group, const NSettingAction action,
                                 const QString & key, const QVariant & value, const QString & comment)
                                     :m_context(context), m_group(group), m_action(action), m_key(key), m_value(value),
                                     m_comment(comment)
{
    m_type = NSettingCommand::stGroupItem;
}

NSettingCommand::NSettingCommand(const QString & comment)
    :m_comment(comment)
{
    m_type = NSettingCommand::stComment;
}

bool NSettingCommand::operator<(const NSettingCommand & cmd) const
{		
    return NString_n::naturalCompare(toString(), cmd.toString()) < 0;
}

const QString & NSettingCommand::context() const
{
    return m_context;
}

NSettingCommand::SettingType NSettingCommand::type() const
{
    return m_type;
}

NSettingCommand::NSettingAction NSettingCommand::action() const
{
    return m_action;
}

const QString & NSettingCommand::key() const
{
    return m_key;
}

const QVariant & NSettingCommand::value() const
{	
    return m_value;
}

quint32 NSettingCommand::group() const
{
    return m_group;
}

const QString & NSettingCommand::comment() const
{
    return m_comment;
}

bool NSettingCommand::isValid() const
{
    if (m_context.isEmpty() &&
        (m_type == stGroupItem ||  m_type == stListItem))
        return false;

    switch (m_type) {
    case stListItem:
        if (m_action == nsDef)
            return !m_key.trimmed().isEmpty();
        break;

    case stGroupItem:
        if (m_action == nsDef)
            return !m_key.trimmed().isEmpty(); //group is unsigned int, no test needed

        break;

    case stComment:
        return !m_comment.isEmpty();
    }
    return true;
}

const QString NSettingCommand::toString() const
{
    if (!isValid()) {
#ifndef UNIT_TEST
        NLOGD("NSettingCommand::toString(): invalid command",
              QString("context='%1' type='%2' group='%3' action='%4' key='%5' value='%6' comment='%7'").
              arg(m_context).
              arg(toString(m_type)).
              arg(m_group).
              arg(toString(m_action)).
              arg(m_key).
              arg(m_value.toString()).
              arg(m_comment)
              );
#endif
        return "";
    }

    QString command(toProtectedFormat(m_context));
    switch (m_type) {
    case stListItem:
        {
            switch (m_action) {
            case nsDef:
                {
                    command += QString(" %1 %2").arg(toString(nsDef)).
                               arg(toProtectedFormat(m_key));

                    if (!m_value.isNull())
                        command += QString("%1%2").arg(N_EQUAL).
                                   arg(toProtectedFormat(m_value.toString()));
                    break;
                }

            case nsUndef:
                {
                    command += QString(" %1").arg(toString(nsUndef));

                    if (!m_key.isNull())
                        command += QString(" %1").arg(toProtectedFormat(m_key));

                    if (!m_value.isNull())
                        command += QString("%1%2").arg(N_EQUAL).
                                   arg(toProtectedFormat(m_value.toString()));

                    break;
                }
            }
            break;
        }

        // <context> group <groupid> add|delete|clear <key>=<value>
    case stGroupItem:
        {
            switch (m_action) {
            case nsDef:
                {
                    command += QString(" %1 %2 %3 %4").arg(N_GROUP).
                               arg(m_group).
                               arg(toString(nsDef)).
                               arg(toProtectedFormat(m_key));

                    if (!m_value.isNull())
                        command += QString("%1%2").arg(N_EQUAL).
                                   arg(toProtectedFormat(m_value.toString()));
                    break;
                }

            case nsUndef:
                {
                    command += QString(" %1 %2 %3").arg(N_GROUP).arg(m_group).
                               arg(toString(nsUndef));

                    if (!m_key.isNull())
                        command += QString(" %1").arg(toProtectedFormat(m_key));

                    if (!m_value.isNull())
                        command += QString("%1%2").arg(N_EQUAL).
                                   arg(toProtectedFormat(m_value.toString()));

                    break;
                }
            }
            break;
        }

    case stComment:
        return QString("# %1").arg(m_comment);
    }
    if (!m_comment.isEmpty())
        command += QString(" %1 %2").arg(N_COMMENT_SEPARATOR).arg(m_comment);
    return command;
}

const NSettingCommand NSettingCommand::toNSettingCommand(const QString & command)
{
    /*
     * <context> def|undef <key>[=<value>]
     * ex:  server def port=6391 # Define a key and it's value as an integer value
     *      server def name=TotoServer # Define a key and it's value as a string value
	 *      server def pub="/Users/sebastien/Documents/seb/dev/nawis/ui" # Define a key and it's value as a protected string value
     *      server def A # Define a key
     *      server undef port # Undefine a key
     *      server undef # Undefine an entire context
     *
     * <context> group <groupid> def|undef <key>[=<value>]
     * ex:   shared_dirs group 0 def path="/Users/sebastien/Music" # Define a key and it's value as a protected string value in group 0
     *       shared_dirs group 0 def shared=true # Define a key and it's value as a boolean value in group 0
     *       shared_dirs group 0 def limit=300 # Define a key and it's value as an int value in group 0
     *       shared_dirs group 1 def path="/Users/sebastien/Downloads" # Define a key and it's value as a protected string value in group 1
     *       shared_dirs group 1 def shared=true # Define a key and it's value as a boolean value in group 1
     *       shared_dirs group 0 undef recursive # Undefine a key and it's value group 0
     *       shared_dirs group 0 undef # Undefine group 0
     */

    QString   context;
    int       group = -1;
    NSettingAction action;
    QString   key;
    QVariant  value;
    QString   comment;
    SettingType   type;

    QString cmd = command.trimmed();
    //qDebug(qPrintable(QString("cmd: %1").arg(cmd)));
    int index = 0;
    QString nextWord = readWord(command, &index);
    if(nextWord.isEmpty())
        return NSettingCommand("");

    if (nextWord.compare(N_COMMENT_SEPARATOR, Qt::CaseInsensitive) == 0)
    {
        // Comment only
        comment = readComment(command, &index);
        //qDebug(qPrintable(QString("comment: %1").arg(comment)));
        return NSettingCommand(comment);
    } else {
        // Context
        context = nextWord;
        //qDebug(qPrintable(QString("context: %1").arg(context)));
    }

    nextWord = readWord(command, &index);
    if(nextWord.isEmpty())
        return NSettingCommand("");

    if (nextWord.compare(N_GROUP, Qt::CaseInsensitive) == 0)
    {
        // group
        group = readWord(command, &index).toUInt();
        //qDebug(qPrintable(QString("group: %1").arg(group)));
        nextWord = readWord(command, &index);
        if(nextWord.isEmpty())
            return NSettingCommand("");

        type = stGroupItem;
    } else {
        // list
        type = stListItem;
    }

    // action
    if (nextWord.compare(N_DEFINE, Qt::CaseInsensitive) == 0)
        action = nsDef;
    else if (nextWord.compare(N_UNDEFINE, Qt::CaseInsensitive) == 0)
        action = nsUndef;
    else
        return NSettingCommand("");
    //qDebug(qPrintable(QString("action: %1").arg(toString(action))));


    nextWord = readWord(command, &index);
    //qDebug(qPrintable(QString("nextWord: %1").arg(nextWord)));
    if (nextWord.isEmpty())
    {
        if (action == nsDef) // Action def without key > invalid cmd
            return NSettingCommand("");

        // Action Undef without key > undef context cmd
        if (group != -1)
            return NSettingCommand(context, group, action, key, value, comment);
        return NSettingCommand(context, action, key, value, comment);
    }

    if (nextWord.compare(N_COMMENT_SEPARATOR, Qt::CaseInsensitive) == 0)
    {
        // Comment
        comment = readComment(command, &index);
        //qDebug(qPrintable(QString("comment: %1").arg(comment)));
        if (group != -1)
            return NSettingCommand(context, group, action, key, value, comment);
        return NSettingCommand(context, action, key, value, comment);
    }

    // key
    key = nextWord;
    //qDebug(qPrintable(QString("key: %1").arg(key)));

    nextWord = readWord(command, &index);
    if (nextWord.isEmpty())
    {
        if (group != -1)
            return NSettingCommand(context, group, action, key, value, comment);
        return NSettingCommand(context, action, key, value, comment);
    }

    //qDebug(qPrintable(QString("nextWord: %1").arg(nextWord)));

    if (nextWord.compare(N_COMMENT_SEPARATOR, Qt::CaseInsensitive) == 0)
    {
        // Comment
        comment = readComment(command, &index);
        if (group != -1)
            return NSettingCommand(context, group, action, key, value, comment);
        return NSettingCommand(context, action, key, value, comment);
    }

    // value
    value = nextWord;
    //qDebug(qPrintable(QString("value: %1").arg(value.toString())));
    nextWord = readWord(command, &index);
    if (nextWord.isEmpty())
    {
        if (group != -1)
            return NSettingCommand(context, group, action, key, value, comment);
        return NSettingCommand(context, action, key, value, comment);

    }
    if (nextWord.compare(N_COMMENT_SEPARATOR, Qt::CaseInsensitive) == 0)
    {
        // Comment
        comment = readComment(command, &index);
    }
    if (group != -1)
        return NSettingCommand(context, group, action, key, value, comment);
    return NSettingCommand(context, action, key, value, comment);
}

const QString NSettingCommand::toString(NSettingAction action)
{
    switch (action) {
    case nsDef:
        return N_DEFINE;

    case nsUndef:
        return N_UNDEFINE;
    }
    Q_ASSERT(false);
    return "";
}

const QString NSettingCommand::toString(SettingType type)
{
    switch (type) {
    case stComment:
        return "Comment";
    case stListItem:
        return "ListItem";
    case stGroupItem:
        return "GroupItem";
    }
    Q_ASSERT(false);
    return "";
}


const QString NSettingCommand::toProtectedFormat(const QString & s)
{
    if (s.contains(N_SPACE, Qt::CaseInsensitive))
        return QString("%1%2%3").arg(N_DBQUOTE).arg(s).arg(N_DBQUOTE);
    return s;
}

const QString NSettingCommand::readWord(const QString & command, int * index)
{
    QString aWord;
    int i;
    for(i = *index; i < command.size(); ++i)
    {
        if (N_SEPARATOR.compare(command[i], Qt::CaseInsensitive) == 0){
            if (aWord.isEmpty())
                continue;
            *index = i;
            break;
        }

        if (N_EQUAL.compare(command[i], Qt::CaseInsensitive) == 0){
            if (aWord.isEmpty())
                continue;
            *index = ++i;
            break;
        }

        if (N_SPACE.compare(command[i], Qt::CaseInsensitive) == 0 && aWord.isEmpty())
            continue;

        if (N_DBQUOTE.compare(command[i], Qt::CaseInsensitive) == 0 && aWord.isEmpty())
        {
            aWord =  readQuotedWord(command, &i);
            *index = i;
            return aWord;
        }

        aWord += command[i];

        if (N_COMMENT_SEPARATOR.compare(aWord, Qt::CaseInsensitive) == 0)
        {
            *index = ++i;
            break;
        }
    }
    if (i == command.size())
        *index = i;
    return aWord;
}

const QString NSettingCommand::readQuotedWord(const QString & command, int * index)
{
    QString aWord;
    int i;
    for(i = *index; i < command.size(); ++i)
    {
        if (N_DBQUOTE.compare(command[i], Qt::CaseInsensitive) == 0){
            if (aWord.isEmpty())
                continue;
            *index = i++ + 1;
            break;
        }

        aWord += command[i];
    }
    if (i == command.size())
        *index = i;
    return aWord;
}

const QString NSettingCommand::readComment(const QString & command, int * index)
{
    Q_ASSERT(index != NULL);
    const QString & comment = command.right(command.length() - *index).trimmed();
    /*qDebug(qPrintable(QString("command: %1").arg(command)));
     qDebug(qPrintable(QString("command.length() - index: %1").arg(command.length() - *index)));*/
    //qDebug(qPrintable(QString("comment: %1").arg(comment)));
    return comment;
}

/*******************************************************************************
 * NSettingCommandList
 *******************************************************************************/
int  NSettingCommandList::indexOfListItem(const QString & context, const QString & key) const
{
    for (int i = 0; i < count(); ++i)
    {
        const NSettingCommand & cmd = at(i);
        if (cmd.context().compare(context) == 0 &&
            cmd.key().compare(key) == 0)
            return i;
    }
    return -1;
}

int NSettingCommandList::indexOfGroupItem(const QString & context, quint32 group, const QString & key) const
{
    for (int i = 0; i < count(); ++i)
    {
        const NSettingCommand & cmd = at(i);
        if (cmd.context().compare(context) == 0 &&
            cmd.key().compare(key) == 0 &&
            cmd.group() == group)
            return i;
    }
    return -1;
}

int NSettingCommandList::indexOfComment(const QString & comment) const
{
    for (int i = 0; i < count(); ++i)
    {
        const NSettingCommand & cmd = at(i);
        if (cmd.comment().compare(comment) == 0)
            return i;
    }
    return -1;
}

/*******************************************************************************
 * NSettings
 *******************************************************************************/
NSettings::NSettings() {
    m_filePath = QDir::toNativeSeparators(QString("%1%2.ini").
                                          arg(NPath_n::config()).
                                          arg(qApp->applicationName()));
    createFilePath();

    QFileInfo fi(m_filePath);
    m_fileLastModified = fi.lastModified();
    connect(&m_fileWatchTimer, SIGNAL(timeout()), this, SLOT(onFileWatchTimer()));
    m_fileWatchTimer.start(5000);
    m_currentGroupIndex = -1;
}

NSettings::~NSettings()
{
}

void NSettings::createFilePath()
{
    QFileInfo fi(m_filePath);

    if (!fi.absoluteDir().exists())
        fi.absoluteDir().mkpath(fi.absolutePath());
    if (fi.exists())
        return;

    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) 
    {
        Q_ASSERT_X(false, "NSettings::createFilePath", "!file.open(QIODevice::WriteOnly | QIODevice::Text)");
#ifndef UNIT_TEST  
        NLOGM("Config file creation failed", "file.open");
#endif
    }
}

void NSettings::onFileWatchTimer()
{
    QFileInfo fi(m_filePath);
    if (fi.lastModified() == m_fileLastModified)
        return;
    m_fileLastModified = fi.lastModified();

    emit configFileChanged();
}

bool NSettings::loadFromFile()
{
    QFile file(m_filePath);
    clear();

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        NLOGM("NSettings::loadFromFile()", "Unable to open configuration file.");
        return false;
    }

    QTextStream in(&file);
    in.setAutoDetectUnicode(true);
    in.setCodec("UTF-8");

    int lineNumber = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        ++lineNumber;
        if (line.isEmpty())
            continue;
        NSettingCommand cmd = NSettingCommand::toNSettingCommand(line);
        //NLOGD("NSettings::loadFromFile()", cmd.toString());
        if (cmd.isValid()){
            /*
             #ifndef UNIT_TEST
			 NLOGD("NSettings::loadFromFile(), new command context", cmd.context());
			 NLOGD("NSettings::loadFromFile(), new command", cmd.toString());
             #endif
             */
            setCommand(cmd);
        } else {
#ifndef UNIT_TEST
            NLOGM("NSettings::loadFromFile(), invalid command:",
                  QString("%1 (line %2)").arg(line).arg(lineNumber));
#endif
        }
    }

    QFileInfo fi(m_filePath);
    m_fileLastModified = fi.lastModified();

    return true;
}

bool NSettings::saveToFile()
{
    QFile file(m_filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        NLOGM("NSettings::saveToFile()", "Unable to open configuration file.");
        return false;
    }

    QTextStream out(&file);
    out.setAutoDetectUnicode(true);
    out.setCodec("UTF-8");
    qSort(m_commands.begin(), m_commands.end());
    QString previousContext;
    for (int i = 0; i < m_commands.size(); ++i)
    {
        const NSettingCommand & cmd = m_commands.at(i);
        if (!previousContext.isEmpty() &&
            previousContext != cmd.context())
            out << endl;

        out << cmd.toString() << endl;
        previousContext = cmd.context();
    }

    return true;
}

void NSettings::clear()
{
    m_commands.clear();
}

QVariant NSettings::value(const QString & context, const QString & key,
                          const QVariant & defaultValue) const
{
    for (int i = 0; i < m_commands.size(); ++i)
    {
        const NSettingCommand & cmd = m_commands.at(i);
        if (cmd.type() != NSettingCommand::stListItem)
            continue;

        if (cmd.context().compare(context) != 0)
            continue;

        if (cmd.key().compare(key) != 0)
            continue;

        if (cmd.value().isNull())
            return defaultValue;
        else
            return cmd.value();
    }
    return defaultValue;
}

QVariant NSettings::value(const QString & context, quint32 group, const QString & key,
                          const QVariant & defaultValue) const
{
    for (int i = 0; i < m_commands.size(); ++i)
    {
        const NSettingCommand & cmd = m_commands.at(i);
        if (cmd.type() != NSettingCommand::stGroupItem)
            continue;

        if (cmd.context().compare(context) != 0)
            continue;

        if (cmd.group() != group)
            continue;

        if (cmd.key().compare(key) != 0)
            continue;

        if (cmd.value().isNull())
            return defaultValue;
        else
            return cmd.value();
    }
    return defaultValue;
}

void NSettings::setValue(const QString & context, const QString & key,
                         const QVariant & value)
{
    if (m_currentGroupIndex == -1)
        setCommand(NSettingCommand(context, NSettingCommand::nsDef, key, value));
    else
        setCommand(NSettingCommand(context, m_currentGroupIndex, NSettingCommand::nsDef, key, value));
}


int NSettings::groupSize(const QString & context) const
{	
    QHash<QString, int> map;
    for (int i = 0; i < m_commands.size(); ++i)
    {
        const NSettingCommand & cmd = m_commands.at(i);
        if (cmd.type() != NSettingCommand::stGroupItem)
            continue;

        if (cmd.context().compare(context) != 0)
            continue;

        map.insert(QString(cmd.group()), cmd.group());
    }
    return map.size();
}

bool NSettings::groupValueExists(const QString & context, const QString & key,
                                 const QVariant & value)
{
    for (int i = 0; i < groupSize(context); ++i)
    {
        if (this->value(context, i, key).toByteArray() == value.toByteArray())
            return true;
    }
    return false;
}

void NSettings::deleteGroup(const QString & context)
{
    QHash<QString, int> map;
    for (int i = m_commands.size() - 1; i > 0; --i)
    {
        const NSettingCommand & cmd = m_commands.at(i);
        if (cmd.type() != NSettingCommand::stGroupItem)
            continue;

        if (cmd.context().compare(context) != 0)
            continue;
        m_commands.removeAt(i);
    }
}

void NSettings::setCommand(const NSettingCommand & cmd)
{
    int i = 0;
    if (cmd.type() == NSettingCommand::stListItem)
        i = m_commands.indexOfListItem(cmd.context(), cmd.key());
    else if (cmd.type() == NSettingCommand::stGroupItem)
        i = m_commands.indexOfGroupItem(cmd.context(), cmd.group(), cmd.key());

    switch (cmd.type())
    {
    case NSettingCommand::stListItem:
    case NSettingCommand::stGroupItem:
        {
            if (i == -1)
            {
                if (cmd.action() == NSettingCommand::nsDef)
                    m_commands.append(cmd);
                return;
            }

            if (cmd.action() == NSettingCommand::nsDef)
                m_commands.replace(i, cmd);
            else
                m_commands.removeAt(i);

            break;
        }

    case NSettingCommand::stComment:
        m_commands.append(cmd);
        break;
    }
}

void NSettings::beginGroup(const QString & context)
{
    m_currentGroupIndex = groupSize(context);
}

void NSettings::endGroup()
{
    m_currentGroupIndex = -1;
}
