#ifndef EXPERIENCEGEM_H
#define EXPERIENCEGEM_H

#include "gameobject.h"

//经验宝石类
class ExperienceGem : public GameObject
{
public:
    ExperienceGem(const Vec2& pos, int expValue);
    ~ExperienceGem() override = default;
    
    void update(float deltaTime) override;
    void render(QPainter& painter) override;
    
    // 找到玩家位置，被玩家吸引
    void setTarget(const Vec2& target) { m_target = target; }
    
    // 吸引范围
    void setAttractRange(float range) { m_attractRange = range; }
    
    // 获取经验值
    int getExperienceValue() const { return m_expValue; }

    //获取回血
    int getHealValue() const { return m_healValue; }
    
    // 是否被吸引
    bool isAttracted() const { return m_attracted; }
    
private:
    int m_expValue;
    Vec2 m_target;
    float m_attractRange = 100.0f;
    bool m_attracted = false;
    float m_bobTimer = 0.0f;// 上下浮动动画

    int m_randomHealNum;
    int m_healValue;
};

#endif
