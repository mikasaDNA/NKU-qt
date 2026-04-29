#include "projectile.h"
#include "spritemanager.h"
#include <QPainter>
#include <QtMath>
#include <QPainterPath>

Projectile::Projectile(const Vec2& pos, const Vec2& direction, int damage, float speed)
    : GameObject(ObjectType::Projectile, pos)
    , m_direction(direction.normalized())
    , m_damage(damage)
{
    m_speed = speed;
    m_collisionRadius = 5.0f;
}

void Projectile::update(float deltaTime)
{
    if (!m_alive) return;
    
    // 沿方向移动
    Vec2 movement = m_direction * m_speed * deltaTime;
    m_position = m_position + movement;
    m_distanceTraveled += movement.length();
    
    // 检查是否超出射程
    if (m_distanceTraveled > m_maxRange)
    {
        m_alive = false;
    }
}

void Projectile::render(QPainter& painter)
{
    if (!m_alive) return;

    painter.save();
    QPixmap sprite = SpriteManager::instance().getSprite("projectile");
    sprite = sprite.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPainterPath circlePath;
    circlePath.addEllipse(m_position.toPointF().x()-m_collisionRadius*2.0f,
                          m_position.toPointF().y()-m_collisionRadius*2.0f,
                          m_collisionRadius*4.0f, m_collisionRadius*4.0f);
    painter.setClipPath(circlePath);
    painter.drawPixmap(m_position.toPointF().x()-m_collisionRadius*2.0f,
                       m_position.toPointF().y()-m_collisionRadius*2.0f,
                       m_collisionRadius*4.0f, m_collisionRadius*4.0f,sprite);
    painter.restore();
}

void Projectile::reducePenetration()
{
    m_penetration--;
    if (m_penetration <= 0)
    {
        m_alive = false;
    }
}