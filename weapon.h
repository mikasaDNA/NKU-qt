#ifndef WEAPON_H
#define WEAPON_H

#include "gameobject.h"
#include <memory>
#include <vector>
#include<QString>
#include <QSoundEffect>

class Player;

//武器类
class Weapon
{
public:
    Weapon(float damage, float cooldown, float range, const std::string& name);
    virtual ~Weapon() = default;
    
    // 更新武器冷却时间并尝试攻击
    void update(float deltaTime, Player* player, 
                const std::vector<GameObject*>& enemies,
                std::vector<std::unique_ptr<GameObject>>& projectiles);
    
    // 子类实现的攻击逻辑
    virtual void attack(Player* player, 
                       const std::vector<GameObject*>& enemies,
                       std::vector<std::unique_ptr<GameObject>>& projectiles) = 0;
    
    // 武器属性
    const std::string& getName() const { return m_name; }
    float getDamage() const { return m_damage; }
    float getCooldown() const { return m_cooldown; }
    int getLevel() const { return m_level; }
    
    // 升级武器
    virtual void upgrade();
    bool isMaxLevel() const { return m_level>=m_maxlevel; }

    //升级选择UI
    virtual QString getUpgradeDescription() const
    {
        return QString("伤害+20%，攻速+10%");
    }
    
protected:
    // 找到最近的敌人
    GameObject* findNearestEnemy(const Vec2& pos, 
                                 const std::vector<GameObject*>& enemies,
                                 float maxRange);
    
    // 找到范围内的所有敌人
    std::vector<GameObject*> findEnemiesInRange(const Vec2& pos,
                                                 const std::vector<GameObject*>& enemies,
                                                 float range);
    
    std::string m_name;
    float m_damage;
    float m_cooldown;
    float m_range;
    float m_timer = 0.0f;
    int m_level = 1;
    int m_maxlevel=5;//每个武器最高五级
    int m_penetration=1;
};

// 升级选项的结构体
struct UpgradeOption {
    enum class Type { UpgradeExisting, AddNewWeapon };

    Type type;
    Weapon* targetWeapon = nullptr; // 如果是升级旧武器，指向那个武器
    QString weaponName;
    QString description;
    int currentLevel = 0;

    //生成“升级旧武器”的选项
    static UpgradeOption CreateUpgrade(Weapon* w)
    {
        return {
            Type::UpgradeExisting,
            w,
            QString::fromStdString(w->getName()),
            w->getUpgradeDescription(),
            w->getLevel()
        };
    }

    //生成“获取新武器”的选项
    static UpgradeOption CreateNew(const QString& name, const QString& desc)
    {
        return {
            Type::AddNewWeapon,
            nullptr,
            name,
            desc,
            0
        };
    }
};


//--------------------weapon--------------------

//初始武器：第十突击队
class No10Commando : public Weapon
{
public:
    No10Commando();
    
    void attack(Player* player,
                const std::vector<GameObject*>& enemies,
                std::vector<std::unique_ptr<GameObject>>& projectiles) override;
    
    void upgrade() override;

    QString getUpgradeDescription() const override
    {
        if(isMaxLevel()) return QString("已满级");
        if(m_level + 1 == 2) return QString("子弹+1，攻速加快，射程增大");
        if(m_level + 1 == 4) return QString("穿透+1");
        if(m_level + 1 == 5) return QString("子弹+1");
        return QString("攻速加快，射程增大");
    }
    
private:
    int m_projectileCount = 1;
};

//突击队突击
class NightRaid : public Weapon
{
public:
    NightRaid();
    
    void attack(Player* player,
                const std::vector<GameObject*>& enemies,
                std::vector<std::unique_ptr<GameObject>>& projectiles) override;
    
    void upgrade() override;

    QString getUpgradeDescription() const override
    {
        if(isMaxLevel()) return QString("已满级");
        if(m_level + 1 == 3) return QString("伤害提高，穿透+1");
        return QString("伤害提高，弹药+2");
    }
    
private:
    int m_projectileCount = 8;
};

//卑斯麦号
// class beisimai : public Weapon
// {
// public:
//     beisimai();

//     void attack(Player* player,
//                 const std::vector<GameObject*>& enemies,
//                 std::vector<std::unique_ptr<GameObject>>& projectiles) override;

//     void upgrade() override;

//     QString getUpgradeDescription() const override
//     {
//         if(isMaxLevel()) return QString("已满级");
//         if(m_level + 1 == 3) return QString("伤害提高，穿透+1");
//         return QString("伤害提高，弹药+2");
//     }
// };

//微笑哥
class SmileBro : public Weapon
{
public:
    SmileBro();

    void attack(Player* player,
                const std::vector<GameObject*>& enemies,
                std::vector<std::unique_ptr<GameObject>>& projectiles) override;

    void upgrade() override;

    QString getUpgradeDescription() const override
    {
        if(isMaxLevel()) return QString("已满级");
        if(m_level + 1 == 3) return QString("等待时间减少");
        return QString("伤害增加，冷却缩短");
    }

    // 设置地图尺寸
    void setMapSize(float width, float height) { m_mapWidth = width; m_mapHeight = height; }

private:
    float m_waitTime;// 小方块等待时间
    float m_mapWidth;
    float m_mapHeight;
    QSoundEffect m_sound;
};

//莫洛托夫鸡尾酒
class Molotov : public Weapon
{
public:
    Molotov();

    void attack(Player* player,
                const std::vector<GameObject*>& enemies,
                std::vector<std::unique_ptr<GameObject>>& projectiles) override;

    void upgrade() override;

    QString getUpgradeDescription() const override
    {
        if(isMaxLevel()) return("已满级");
        if(m_level+1<=3) return("瓶子+1，范围变大");
        return QString("瓶子+1");
    }

private:
    float m_poolRadius;//燃烧半径
    float m_poolDuration;//持续时间
    float m_throwRange;//投掷距离
    float m_mapWidth;
    float m_mapHeight;
    int m_poolCount;
};

#endif
