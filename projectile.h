#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "gameobject.h"

//子弹类
class Projectile : public GameObject
{
public:
    Projectile(const Vec2& pos, const Vec2& direction, int damage, float speed);
    ~Projectile() override = default;
    
    void update(float deltaTime) override;
    void render(QPainter& painter) override;
    
    // 获取伤害值
    int getDamage() const { return m_damage; }
    
    // 设置穿透次数
    void setPenetration(int count) { m_penetration = count; }
    int getPenetration() const { return m_penetration; }
    
    // 减少穿透次数
    void reducePenetration();
    
    // 设置最大射程
    void setMaxRange(float range) { m_maxRange = range; }

private:
    Vec2 m_direction;
    int m_damage;
    int m_penetration = 1;//穿透次数
    float m_distanceTraveled = 0.0f;
    float m_maxRange = 500.0f;
    QColor m_color;
};

#endif
