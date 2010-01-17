#ifndef N_NODE_H
#define N_NODE_H

// Qt
#include <QString>

class NNode
{
public:
	const QString & address() const;
	void setAddress(const QString & address);

	const QString & publicKey() const;
	void setPublicKey(const QString & publicKey);

	//bool isValid();

private:
	QString m_address;
	QString m_publicKey;
};

#endif // N_NODE_H
