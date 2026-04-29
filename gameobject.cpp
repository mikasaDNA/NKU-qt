#include "gameobject.h"
#include "spritemanager.h"
#include <QPainter>
#include <QtMath>
#include <QPainterPath>

GameObject::GameObject(ObjectType type, const Vec2& pos)
    : m_type(type)
    , m_position(pos)
    , m_velocity(0, 0)
{
}

QRectF GameObject::getBoundingRect() const
{
    return QRectF(
        m_position.x - m_collisionRadius,
        m_position.y - m_collisionRadius,
        m_collisionRadius * 2,
        m_collisionRadius * 2
    );
}

void GameObject::takeDamage(int damage)
{
    m_health -= damage;
    if (m_health <= 0) {
        m_health = 0;
        m_alive = false;
    }
}

//----------哥方块----------
BroBlock::BroBlock(const Vec2& pos, int damage, float waitTime,
                   float mapWidth, float mapHeight)
    : GameObject(ObjectType::Projectile, pos)
    , m_damage(damage)
    , m_penetration(20)
    , m_waitTime(waitTime)
    , m_elapsedTime(0.0f)
    , m_bigSpeed(600.0f)
    , m_direction(1.0f, 0.0f)
    , m_isBigPhase(false)
    , m_mapWidth(mapWidth)
    , m_mapHeight(mapHeight)
    , m_rotation(0.0f)
    , m_pulsePhase(0.0f)
{
    m_collisionRadius = 15.0f;// 一开始是小方块碰撞半径
}

void BroBlock::update(float deltaTime)
{
    if (!m_alive) return;

    m_elapsedTime += deltaTime;
    m_rotation += deltaTime * 180.0f;// 旋转
    m_pulsePhase += deltaTime * 5.0f;

    if (!m_isBigPhase)
    {
        // 小方块阶段：等待
        if (m_elapsedTime >= m_waitTime)
        {
            enterBigPhase();
        }
    } else
    {
        // 大方块阶段：移动
        m_position = m_position + m_direction * m_bigSpeed * deltaTime;

        // 检查是否出地图
        if (isOutOfMap()) m_alive = false;
    }
}

void BroBlock::render(QPainter& painter)
{
    if (!m_alive) return;

    painter.save();
    painter.translate(m_position.toPointF());
    painter.rotate(m_rotation);

    float pulse = 1.0f + 0.1f * std::sin(m_pulsePhase);

    if (!m_isBigPhase)
    {
        // 小方块 - 黄色笑脸方块
        QPixmap sprite = SpriteManager::instance().getSprite("smilebro");
        sprite = sprite.scaled(m_collisionRadius*2.0f, m_collisionRadius*2.0f, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        float size = m_collisionRadius * 2.0f * pulse;
        painter.drawPixmap(QPointF(-size/2, -size/2),sprite);
    }
    else
    {
        // 大方块 - 红色愤怒方块
        QPixmap sprite = SpriteManager::instance().getSprite("IS-2");
        sprite = sprite.scaled(m_collisionRadius*2.0f, m_collisionRadius*2.0f, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        float size = m_collisionRadius * 2.0f * pulse;
        painter.drawPixmap(QPointF(-size/2, -size/2),sprite);
    }

    painter.restore();
}

void BroBlock::enterBigPhase()
{
    m_isBigPhase = true;
    m_collisionRadius = 30.0f;
    m_elapsedTime = 0.0f;
}

void BroBlock::reducePenetration()
{
    m_penetration--;
    if (m_penetration <= 0)
    {
        m_alive = false;
    }
}

bool BroBlock::isOutOfMap() const
{
    float margin = m_collisionRadius * 2;
    return m_position.x < -margin || m_position.x > m_mapWidth + margin ||
           m_position.y < -margin || m_position.y > m_mapHeight + margin;
}

//----------燃烧区域----------
Pool::Pool(const Vec2& pos,int damage,float radius,float duration)
    :GameObject(ObjectType::Effect,pos)
    ,m_damage(damage)
    ,m_poolRadius(radius)
    ,m_duration(duration)
    ,m_elapsedTime(0.0f)
    ,m_damageTimer(0.0f)
    ,m_damageInterval(0.3f)
    ,m_flickerPhase(0.0f)
{
    m_collisionRadius=radius;
}

void Pool::update(float detlaTime)
{
    if(!m_alive) return;

    m_elapsedTime+=detlaTime;
    m_damageTimer+=detlaTime;
    m_flickerPhase+=detlaTime*8.0f;

    clearDamagedEnemies();

    if(m_elapsedTime>=m_duration) {m_alive=false;}
}

void Pool::render(QPainter& painter)
{
    if(!m_alive) return;

    painter.save();

    // 计算透明度
    float alpha = 1.0f;
    float fadeStart = m_duration * 0.7f;  // 最后30%时间开始淡出
    if (m_elapsedTime > fadeStart)
    {
        alpha = 1.0f - (m_elapsedTime - fadeStart) / (m_duration - fadeStart);
    }

    float flicker = 0.7f + 0.1f * std::sin(m_flickerPhase);
    alpha *= flicker;

    // 绘制燃烧区域外圈（银白色光晕）----并没有做得很好
    QRadialGradient outerGlow(m_position.toPointF(), m_poolRadius * 1.2f);
    outerGlow.setColorAt(0, QColor(224,224,224, static_cast<int>(100 * alpha)));
    outerGlow.setColorAt(0.5, QColor(224,224,224, static_cast<int>(60 * alpha)));
    outerGlow.setColorAt(1, QColor(255, 0, 0, 0));
    painter.setBrush(outerGlow);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(m_position.toPointF(), m_poolRadius * 1.2f, m_poolRadius * 1.2f);

    QPixmap sprite = SpriteManager::instance().getSprite("molotov");
    sprite = sprite.scaled(2.0f*m_poolRadius,2.0f*m_poolRadius);
    QPainterPath circlePath;
    circlePath.addEllipse(m_position.toPointF(),m_poolRadius,m_poolRadius);
    painter.setClipPath(circlePath);
    painter.drawPixmap(m_position.toPointF().x()-m_poolRadius,
                       m_position.toPointF().y()-m_poolRadius,
                       sprite);

    painter.restore();
}

bool Pool::isEnemyInPool(const Vec2& enemyPos,float enemyRadius) const
{
    float distance=m_position.distanceTo(enemyPos);
    return distance<(m_poolRadius+enemyRadius);
}



