#include "spritemanager.h"
#include <QDebug>

SpriteManager& SpriteManager::instance()
{
    static SpriteManager mgr;
    return mgr;
}

SpriteManager::SpriteManager()
    : m_resourcePrefix(":/sprites")
{
}

const QPixmap& SpriteManager::getSprite(const QString& name)
{
    // 如果已经缓存，直接返回
    if (m_sprites.contains(name))
    {
        return m_sprites[name];
    }
    
    // 构建资源路径
    QString path = QString("%1/%2.png").arg(m_resourcePrefix).arg(name);
    
    // 加载图片
    QPixmap pixmap(path);
    
    // 缓存并返回
    m_sprites[name] = pixmap;
    return m_sprites[name];
}