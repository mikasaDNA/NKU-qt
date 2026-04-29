#ifndef PLAYER_H
#define PLAYER_H

#include "gameobject.h"
#include "weapon.h"
#include <QSet>
#include <vector>
#include <memory>
#include <QObject>

//玩家类
class Player : public QObject, public GameObject
{
    Q_OBJECT
    
public:
    Player(const Vec2& pos = Vec2(500, 400));
    ~Player() override = default;
    
    // 更新玩家状态
    void update(float deltaTime) override;
    
    // 渲染玩家
    void render(QPainter& painter) override;
    
    // 处理键盘输入
    void handleInput(const QSet<int>& keys, float deltaTime);
    
    // -----经验值系统-----
    void addExperience(int amount);
    int getExperience() const { return m_experience; }
    int getLevel() const { return m_level; }
    int getExperienceToNextLevel() const;
    //--------------------

    // -----武器系统-----
    void addWeapon(std::unique_ptr<Weapon> weapon);
    const std::vector<std::unique_ptr<Weapon>>& getWeapons() const { return m_weapons; }
    std::vector<UpgradeOption> getUpgradeOptions() const;

    // 更新所有武器
    void updateWeapons(float deltaTime, const std::vector<GameObject*>& enemies,
                       std::vector<std::unique_ptr<GameObject>>& projectiles);
    //------------------
    
    // 生命值
    int getMaxHealth() const { return m_maxHealth; }
    void heal(int amount);
    
    // 无敌
    void setInvincible(float duration);
    bool isInvincible() const { return m_invincibleTimer > 0; }
    
signals:
    void levelUp(int newLevel);
    void healthChanged(int current, int max);
    void experienceChanged(int current, int needed);
    
private:
    int m_level = 1;
    int m_experience = 0;
    float m_invincibleTimer = 0.0f;
    std::vector<std::unique_ptr<Weapon>> m_weapons;
};

#endif
