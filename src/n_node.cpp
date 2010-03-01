#include "n_node.h"

const QString & NNode::address() const
{
    return m_address;
}

void NNode::setAddress(const QString & address)
{
    m_address  = address;
}

const QString & NNode::publicKey() const
{
    return m_publicKey;
}

void NNode::setPublicKey(const QString & publicKey)
{
    m_publicKey = publicKey;
}
