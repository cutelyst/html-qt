#ifndef HTMLTREE_H
#define HTMLTREE_H

#include <QObject>
#include <QStringList>
#include <QVector>

class HTMLToken;
class HTMLTreeNode
{
public:
    HTMLTreeNode *parent = nullptr;
    QVector<HTMLTreeNode *> children;
    HTMLToken *token;
    QStringRef type;
    QStringRef text;
    bool end = false;
    bool plainText = true;
};

class HTMLTree
{
public:
    HTMLTree();
    ~HTMLTree();

    HTMLTreeNode *document();

    void inserText();

    void insertDoctype(HTMLToken *token);

    void insertComment(const QString &comment, HTMLTreeNode *parent);

    void dump();

private:
    HTMLTreeNode *createNode(int &pos, int lastPos, bool plainText, HTMLTreeNode *parent);
    void dumpTree(HTMLTreeNode *root, int level = 0);

    bool m_useAllowed;
    QStringList m_allowed;
    QString m_content;
    int m_pos = 0;
    HTMLTreeNode *m_root = 0;
    QList<HTMLTreeNode*> m_nodes;
};

#endif // HTMLTREE_H
