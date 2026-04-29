#ifndef SPRITEMANAGER_H
#define SPRITEMANAGER_H

#include <QPixmap>
#include <QMap>
#include <QString>
#include <QSoundEffect>

//精灵图片管理器
class SpriteManager
{
public:
    // 获取单例实例
    static SpriteManager& instance();
    
    // 获取图片
    const QPixmap& getSprite(const QString& name);
    
private:
    SpriteManager();
    ~SpriteManager() = default;
    
    // 禁止拷贝
    SpriteManager(const SpriteManager&) = delete;
    SpriteManager& operator=(const SpriteManager&) = delete;

    QMap<QString, QPixmap> m_sprites;// 图片缓存
    QString m_resourcePrefix;// 资源路径前缀
};

//音效管理器
class SoundManager
{
public:
    // 获取单例实例
    static SoundManager& instance();

    // 获取图片
    const QSoundEffect& getSound(const QString& name);

private:
    SoundManager();
    ~SoundManager() = default;

    // 禁止拷贝
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    QMap<QString, QPixmap> m_sprites;// 图片缓存
    QString m_resourcePrefix;// 资源路径前缀
};

#endif