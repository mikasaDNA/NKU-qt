#ifndef ENEMY_H
#define ENEMY_H

#include "gameobject.h"
#include <vector>
#include <memory>

//敌人种类枚举
enum class EnemyType {
    jiuliumi,// 哈基米联队(普通单位)
    xizhiye,// 习志野(扫码快攻单位)
    lanbingtuan,// 岚兵团(大哥单位)
    bahongyiyu// 八纮一与(未找到。！？)
};

//敌人类
class Enemy : public GameObject
{
public:
    Enemy(EnemyType type, const Vec2& pos);
    ~Enemy() override = default;
    
    void update(float deltaTime) override;
    void render(QPainter& painter) override;
    
    // 设置追踪目标
    void setTarget(const Vec2& target) { m_target = target; }
    
    // 获取敌人类型
    EnemyType getEnemyType() const { return m_enemyType; }
    
    // 获取经验值奖励
    int getExperienceReward() const { return m_expReward; }
    
    // 碰撞伤害
    int getCollisionDamage() const { return m_collisionDamage; }
    
private:
    EnemyType m_enemyType;
    Vec2 m_target;
    int m_expReward;
    int m_healReward;
    int m_collisionDamage = 10;
    QColor m_color;
};

//敌人生成器
class EnemySpawner
{
public:
    EnemySpawner();

    void update(float deltaTime, const Vec2& playerPos,std::vector<std::unique_ptr<Enemy>>& enemies);
    float getDifficulty() const { return m_difficulty; }
    float getGameTime() const { return m_gameTime; }

private:
    // 生成敌人
    void spawnEnemy(const Vec2& playerPos,std::vector<std::unique_ptr<Enemy>>& enemies);

    // 根据难度选择敌人类型
    EnemyType selectEnemyType();

    float m_spawnTimer = 0.0f;
    float m_spawnInterval = 2.0f;// 初始生成间隔
    float m_minSpawnInterval = 0.1f; // 最小生成间隔
    float m_gameTime = 0.0f;
    float m_difficulty = 1.0f;

    // 生成参数
    float m_spawnDistance = 300.0f;// 最小生成距离
    int m_maxEnemies = 250;// 最大敌人数量(据实际体验，难以达到)
};

#endif
