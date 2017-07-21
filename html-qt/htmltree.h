#ifndef HTMLTREE_H
#define HTMLTREE_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <QVector>

class HTMLToken;
class HTMLTreeNode
{
public:
    HTMLTreeNode(const QString &name = QString());
    virtual ~HTMLTreeNode();

    QString name;
    HTMLTreeNode *parent = nullptr;
    QVector<HTMLTreeNode *> children;
    QMap<QString, QString> attributes;
    HTMLToken *token;
    QStringRef type;
    QString text;
    bool end = false;
    bool plainText = true;

    /*!
     * Insert node as a child of the current node
     */
    virtual void appendChild(HTMLTreeNode *node);

    /*!
     * Insert data as text in the current node,
     * TODO positioned before the
     * start of node insertBefore or to the end of the node's text.
     */
    virtual void insertText(const QString &data);

    /*!
     * Remove node from the children of the current node
     */
    virtual void removeChild(HTMLTreeNode *node);

    /*!
     * Move all the children of the current node to newParent.
     * This is needed so that trees that don't store text as nodes move the
     * text in the correct way
     */
    virtual void reparentChildren(HTMLTreeNode *node);

    /*!
     * Return true if the node has children or text, false otherwise
     */
    virtual bool hasContent() const;

    QString asText() const;
};

class HTMLTree
{
public:
    HTMLTree(const QString &namespaceHTMLElements = QString());
    virtual ~HTMLTree();

    void reset();

    HTMLTreeNode *document();

    void insertText(QChar c, HTMLTreeNode *parent = nullptr);

    void inserRoot(HTMLToken *token);

    void insertDoctype(HTMLToken *token);

    void insertComment(HTMLToken *token, HTMLTreeNode *parent = nullptr);

    HTMLTreeNode *createElement(HTMLToken *token);

    void dump();

    QVector<HTMLTreeNode*> openElements() const;

private:
    HTMLTreeNode *createNode(int &pos, int lastPos, bool plainText, HTMLTreeNode *parent);
    void dumpTree(HTMLTreeNode *root, int level = 0);

    QString m_defaultNamespace;
    bool m_useAllowed;
    bool m_insertFromTable = false;
    QStringList m_allowed;
    QString m_content;
    int m_pos = 0;
    QList<HTMLTreeNode*> m_nodes;
    HTMLTreeNode *m_document = nullptr;
    QVector<HTMLTreeNode*> m_openElements;
};

#endif // HTMLTREE_H
