#include "experiencegem.h"
#include <QPainter>
#include <QtMath>
#include <QRandomGenerator>

ExperienceGem::ExperienceGem(const Vec2& pos, int expValue)
    : GameObject(ObjectType::ExperienceGem, pos)
    , m_expValue(expValue)
    , m_healValue(0)
    , m_randomHealNum(0)
{
    m_speed = 300.0f;  // 被吸引时的速度
    m_collisionRadius = 8.0f;
    m_randomHealNum = QRandomGenerator::global()->bounded(9);
    if(m_randomHealNum == 0) { m_healValue = m_expValue * 2; }
}

void ExperienceGem::update(float deltaTime)
{
    if (!m_alive) return;
    
    m_bobTimer += deltaTime * 3.0f;
    
    // 检查是否在吸引范围内
    float distToTarget = m_position.distanceTo(m_target);
    
    if (distToTarget <= m_attractRange)  m_attracted = true;
    
    // 如果被吸引，向目标移动
    if (m_attracted)
    {
        Vec2 direction = (m_target - m_position).normalized();
        m_position = m_position + direction * m_speed * deltaTime;
    }
}

void ExperienceGem::render(QPainter& painter)
{
    if (!m_alive) return;
    
    // 浮动偏移
    float bobOffset = std::sin(m_bobTimer) * 3.0f;
    Vec2 renderPos = m_position + Vec2(0, bobOffset);
    
    // 绘制光晕
    QRadialGradient gradient(renderPos.toPointF(), m_collisionRadius * 2);
    if(m_randomHealNum == 0)
    {
        gradient.setColorAt(0, QColor(255, 100, 100, 150));
    }
    else
    {
        gradient.setColorAt(0, QColor(100, 200, 255, 150));
    }
    gradient.setColorAt(1, QColor(50, 150, 255, 0));
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(renderPos.toPointF(), m_collisionRadius * 2, m_collisionRadius * 2);
    
    // 绘制宝石
    if(m_randomHealNum == 0)
    {
        painter.setBrush(QColor(255, 50, 50));
        painter.setPen(QPen(QColor(255, 100, 100), 1));
    }
    else
    {
        painter.setBrush(QColor(50, 150, 255));
        painter.setPen(QPen(QColor(100, 200, 255), 1));
    }
    
    // 菱形
    QPointF points[4];
    points[0] = QPointF(renderPos.x, renderPos.y - m_collisionRadius);
    points[1] = QPointF(renderPos.x + m_collisionRadius, renderPos.y);
    points[2] = QPointF(renderPos.x, renderPos.y + m_collisionRadius);
    points[3] = QPointF(renderPos.x - m_collisionRadius, renderPos.y);
    painter.drawPolygon(points, 4);
    
    // 高光
    painter.setBrush(QColor(200, 230, 255));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(renderPos.x - 2, renderPos.y - 3), 2, 2);
}
