#include "htmltree.h"

#include "htmltokenizer_p.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(HTML_TREE, "htmlqt.tree")

HTMLTree::HTMLTree(const QString &namespaceHTMLElements)
{
    if (namespaceHTMLElements.isEmpty()) {
        m_defaultNamespace = QStringLiteral("http://www.w3.org/1999/xhtml");
    } else {
        m_defaultNamespace = namespaceHTMLElements;
    }

    reset();
}

HTMLTree::~HTMLTree()
{

}

void HTMLTree::reset()
{
    m_openElements.clear();

    delete m_document;
    m_document = new HTMLTreeNode;
}

HTMLTreeNode *HTMLTree::document()
{
    return m_document;
}

void HTMLTree::inserText()
{
    qCDebug(HTML_TREE) << Q_FUNC_INFO;
}

void HTMLTree::inserRoot(HTMLToken *token)
{
    HTMLTreeNode *node = createElement(token);
    m_openElements.push_back(node);
    m_document->appendChild(node);
}

void HTMLTree::insertDoctype(HTMLToken *token)
{
    qCDebug(HTML_TREE) << Q_FUNC_INFO << token;
    m_document->token = token;
}

void HTMLTree::insertComment(const QString &comment, HTMLTreeNode *parent)
{
    qCDebug(HTML_TREE) << Q_FUNC_INFO;
}

HTMLTreeNode *HTMLTree::createElement(HTMLToken *token)
{
    auto ret = new HTMLTreeNode(token->name);
    ret->attributes = token->data;
    return ret;
}

void HTMLTree::dump()
{
    dumpTree(m_document);
}

QVector<HTMLTreeNode *> HTMLTree::openElements() const
{
    return m_openElements;
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

HTMLTreeNode::HTMLTreeNode(const QString &name)
{
    this->name = name;
}

void HTMLTreeNode::appendChild(HTMLTreeNode *node)
{
    children.push_back(node);
}

void HTMLTreeNode::insertText(const QString &data)
{
    text.append(data);
}

void HTMLTreeNode::removeChild(HTMLTreeNode *node)
{
    children.removeOne(node);
}

void HTMLTreeNode::reparentChildren(HTMLTreeNode *node)
{
    for (HTMLTreeNode *child : children) {
        node->appendChild(child);
    }
    children.clear();
}

bool HTMLTreeNode::hasContent() const
{
    return !text.isEmpty() || !children.isEmpty();
}

QString HTMLTreeNode::asText() const
{
    QString attributesStr;
    for (const std::pair<QString,QString> &pair : attributes) {
        if (pair.second.isEmpty()) {
            attributesStr += QLatin1Char(' ') + pair.first;
        } else {
            attributesStr += QLatin1Char(' ') + pair.first + QLatin1String("=\"") + pair.second + QLatin1Char('"');
        }
    }

    return QLatin1Char('<') + name + attributesStr + QLatin1Char('>');
}
