/*
 *  n_settings.h
 *  nawis
 *
 *  Created by Sébastien Dolard on 12/01/09.
 *
 * Usage
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
 *
 */

#include <QString>
#include <QVariant>
#include <QHash>
#include <QDateTime>
#include <QTimer>

class NSettingCommand
{
public:
    enum NSettingAction {
        nsDef,
        nsUndef
    };

    // stListItem
    NSettingCommand(const QString & context, const NSettingAction action, const QString & key,
                    const QVariant & value = QVariant(), const QString & comment = "");

    // ctGroupItem
    NSettingCommand(const QString & context, quint32 group, const NSettingAction action, const QString & key,
                    const QVariant & value = QVariant(), const QString & comment = "");

    // stComment
    NSettingCommand(const QString & comment);

    enum SettingType {
        stComment,
        stListItem,
        stGroupItem
    };

    const QString & context() const;

    SettingType type() const;

    NSettingAction action() const;

    const QString & key() const;

    const QVariant & value() const;

    quint32 group() const;

    const QString & comment() const;

    bool isValid() const;

    const QString toString() const;

    static const NSettingCommand toNSettingCommand(const QString & command);


    bool operator<(const NSettingCommand & cmd) const;

private:
    QString   m_context;
    quint32   m_group;
    NSettingAction m_action;
    QString   m_key;
    QVariant  m_value;
    QString   m_comment;
    SettingType   m_type;

    static const QString toString(SettingType type);
    static const QString toString(NSettingAction action);
    static const QString toProtectedFormat(const QString & s);

    static const QString readWord(const QString & command, int * index = 0);
    static const QString readQuotedWord(const QString & command, int * index = 0);
    static const QString readComment(const QString & command, int * index = 0);
};

// With QHash, the items are arbitrarily ordered.
typedef QList<NSettingCommand> NSettingCommandList_t;
class NSettingCommandList: public NSettingCommandList_t
{
public:
    int indexOfListItem(const QString & context, const QString & key) const;
    int indexOfGroupItem(const QString & context, quint32 group, const QString & key) const;
    int indexOfComment(const QString & comment) const;
};

class NSettings: public QObject {
    Q_OBJECT
public:
    NSettings();
    ~NSettings();

    bool loadFromFile();
    bool saveToFile();

    void clear();

    QVariant value(const QString & context, const QString & key,
                   const QVariant & defaultValue = QVariant()) const;
    void setValue(const QString & context, const QString & key,
                  const QVariant & value);
    QVariant value(const QString & context, quint32 group, const QString & key,
                   const QVariant & defaultValue = QVariant()) const;
    int groupSize(const QString & context) const;

    bool groupValueExists(const QString & context, const QString & key,
                          const QVariant & value);
    void beginGroup(const QString & context);
    void endGroup();
    void deleteGroup(const QString & context);

signals:
    /* Emit when config is updated ( loaded or reloaded ) */
    void configFileChanged();

private:
    NSettingCommandList  m_commands;
    QString              m_filePath;
    QTimer               m_fileWatchTimer;
    int                  m_currentGroupIndex;
    QDateTime            m_fileLastModified;

    void setCommand(const NSettingCommand & cmd);
    void createFilePath();

private slots:
    void onFileWatchTimer();
};
