#include "htmltree.h"

#include "htmltokenizer_p.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(HTML_TREE, "htmlqt.tree")

HTMLTree::HTMLTree()
{
    m_root = new HTMLTreeNode;
}

HTMLTree::~HTMLTree()
{

}

HTMLTreeNode *HTMLTree::document()
{
    return m_root;
}

void HTMLTree::inserText()
{
    qCDebug(HTML_TREE) << Q_FUNC_INFO;
}

void HTMLTree::insertDoctype(HTMLToken *token)
{
    qCDebug(HTML_TREE) << Q_FUNC_INFO << token;
    m_root->token = token;
}

void HTMLTree::insertComment(const QString &comment, HTMLTreeNode *parent)
{
    qCDebug(HTML_TREE) << Q_FUNC_INFO;
}

void HTMLTree::dump()
{
    dumpTree(m_root);
}

HTMLTreeNode *HTMLTree::createNode(int &pos, int lastPos, bool plainText, HTMLTreeNode *parent)
{
    qCDebug(HTML_TREE) << Q_FUNC_INFO;
    return 0;
}

void HTMLTree::dumpTree(HTMLTreeNode *root, int level)
{
    qDebug() << QByteArray("-").repeated(level).data() << ">" << root->token->name;
    Q_FOREACH (HTMLTreeNode *node, root->children) {
        dumpTree(node, level + 1);
    }
}

