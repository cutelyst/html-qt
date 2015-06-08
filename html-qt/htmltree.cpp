#include "htmltree.h"

#include <QDebug>

class HTMLTreeNode
{
public:
    HTMLTreeNode *parent = 0;
    QList<HTMLTreeNode *> children;
    QStringRef type;
    QStringRef text;
    bool end = false;
    bool plainText = true;
};


HTMLTree::HTMLTree()
{

}

HTMLTree::~HTMLTree()
{

}

HTMLTreeNode *HTMLTree::createNode(int &pos, int lastPos, bool plainText, HTMLTreeNode *parent)
{
    return 0;
}

void HTMLTree::dumpTree(HTMLTreeNode *root, int level)
{
    qDebug() << QByteArray("-").repeated(level).data() << ">" << root->text;
    Q_FOREACH (HTMLTreeNode *node, root->children) {
        dumpTree(node, level + 1);
    }
}

