#include "weapon.h"
#include "player.h"
#include "projectile.h"
#include <QtMath>
#include <QRandomGenerator>
#include <QUrl>

Weapon::Weapon(float damage, float cooldown, float range, const std::string& name)
    : m_name(name)
    , m_damage(damage)
    , m_cooldown(cooldown)
    , m_range(range)
{
}

void Weapon::update(float deltaTime, Player* player,
                    const std::vector<GameObject*>& enemies,
                    std::vector<std::unique_ptr<GameObject>>& projectiles)
{
    m_timer += deltaTime;
    
    if (m_timer >= m_cooldown) {
        m_timer = 0.0f;
        attack(player, enemies, projectiles);
    }
}

void Weapon::upgrade()
{
    if(isMaxLevel()) return;
    m_level++;
    m_damage *= 1.2f;
    m_cooldown = std::max(0.2f, m_cooldown*0.9f);

    if(m_level%2==0)
    {
        m_penetration++;
    }
}

GameObject* Weapon::findNearestEnemy(const Vec2& pos,
                                     const std::vector<GameObject*>& enemies,
                                     float maxRange)
{
    GameObject* nearest = nullptr;
    float nearestDist = maxRange;
    
    for (auto* enemy : enemies) {
        if (!enemy || !enemy->isAlive()) continue;
        
        float dist = pos.distanceTo(enemy->getPosition());
        if (dist < nearestDist) {
            nearestDist = dist;
            nearest = enemy;
        }
    }
    
    return nearest;
}

std::vector<GameObject*> Weapon::findEnemiesInRange(const Vec2& pos,
                                                     const std::vector<GameObject*>& enemies,
                                                     float range)
{
    std::vector<GameObject*> result;
    
    for (auto* enemy : enemies) {
        if (!enemy || !enemy->isAlive()) continue;
        
        float dist = pos.distanceTo(enemy->getPosition());
        if (dist <= range) {
            result.push_back(enemy);
        }
    }
    
    return result;
}

// -------------------- 第十突击队 --------------------

No10Commando::No10Commando()
    : Weapon(10.0f, 1.0f, 400.0f, "第十突击队")
{
}

void No10Commando::attack(Player* player,
                                const std::vector<GameObject*>& enemies,
                                std::vector<std::unique_ptr<GameObject>>& projectiles)
{
    Vec2 playerPos = player->getPosition();
    
    for (int i = 0; i < m_projectileCount; i++) {
        GameObject* target = findNearestEnemy(playerPos, enemies, m_range);
        if (!target) return;
        
        Vec2 direction = target->getPosition() - playerPos;
        
        auto proj = std::make_unique<Projectile>(
            playerPos,
            direction,
            static_cast<int>(m_damage),
            400.0f
        );
        proj->setMaxRange(m_range);
        proj->setPenetration(m_penetration);
        
        projectiles.push_back(std::move(proj));
    }
}

void No10Commando::upgrade()
{
    if(isMaxLevel()) return;
    m_level++;
    if(m_level == 2) m_projectileCount++;
    if(m_level == 4) {m_penetration++; return;}
    if(m_level == 5) {m_projectileCount++; return;}
    m_cooldown = std::max(0.2f, m_cooldown*0.9f);
    m_range += 50.0f;
}

// -------------------- 突击队突击 --------------------

NightRaid::NightRaid()
    : Weapon(8.0f, 2.0f, 300.0f, "突击队突击")
{
}

void NightRaid::attack(Player* player,
                          const std::vector<GameObject*>& enemies,
                          std::vector<std::unique_ptr<GameObject>>& projectiles)
{
    Vec2 playerPos = player->getPosition();
    
    for (int i = 0; i < m_projectileCount; i++)
    {
        float angle = (2.0f * M_PI * i) / m_projectileCount;
        Vec2 direction(std::cos(angle), std::sin(angle));
        
        auto proj = std::make_unique<Projectile>(
            playerPos,
            direction,
            static_cast<int>(m_damage),
            300.0f
        );
        proj->setMaxRange(m_range);
        proj->setPenetration(m_penetration);
        
        projectiles.push_back(std::move(proj));
    }
}

void NightRaid::upgrade()
{
    if(isMaxLevel()) return;
    m_level++;
    m_damage *= 1.2f;
    if(m_level == 3) {m_penetration++; return;}
    m_cooldown = std::max(0.2f, m_cooldown*0.9f);
    m_projectileCount=std::min(24, m_projectileCount+2);
}

// // --------------------卑斯麦--------------------
// beisimai::beisimai()
//     :Weapon(10.0f,1.0f,20.0f,"卑斯麦")
// {
// }

// void beisimai::attack(Player* player,
//                       const std::vector<GameObject*>& enemies,
//                       std::vector<std::unique_ptr<GameObject>>& projectiles)
// {
//     Vec2 playerPos=player->getPosition();


// }

// -------------------- 微笑哥和他的大运 --------------------

SmileBro::SmileBro()
    : Weapon(25.0f, 3.0f, 0.0f, "微笑哥和他的大运")
    , m_waitTime(2.0f)
    , m_mapWidth(1333.0f)
    , m_mapHeight(750.0f)
{
    m_sound.setSource(QUrl("qrc:/sounds/smilebro.wav"));
    m_sound.setVolume(0.3f);
}

void SmileBro::attack(Player* player,
                            const std::vector<GameObject*>& enemies,
                            std::vector<std::unique_ptr<GameObject>>& projectiles)
{
    // 在地图中随机位置生成方块
    float margin = 100.0f;
    float x = QRandomGenerator::global()->bounded(static_cast<int>(margin),
                                                  static_cast<int>(m_mapWidth - margin));
    float y = QRandomGenerator::global()->bounded(static_cast<int>(margin),
                                                  static_cast<int>(m_mapHeight - margin));
    Vec2 randomPos(x, y);

    // 创建小方块
    auto block = std::make_unique<BroBlock>(
        randomPos,
        static_cast<int>(m_damage),
        m_waitTime,
        m_mapWidth,
        m_mapHeight
        );
    //随机方向射击
    float angle = QRandomGenerator::global()->bounded(2.0f * M_PI);
    block->setDirection(Vec2(std::cos(angle), std::sin(angle)));

    block->setPenetration(20);
    projectiles.push_back(std::move(block));

    m_sound.play();
}

void SmileBro::upgrade()
{
    if(isMaxLevel()) return;
    m_level++;
    if(m_level == 3)
    {
        m_waitTime -= 0.5f;
        return;
    }
    m_damage *= 1.2f;
    m_cooldown = std::max(0.2f, m_cooldown*0.9f);
}

// --------------------莫洛托夫鸡尾酒--------------------
Molotov::Molotov()
    :Weapon(1.0f,2.5f,200.0f,"莫洛托夫鸡尾酒")
    ,m_poolRadius(80.0f)
    ,m_poolDuration(3.0F)
    ,m_throwRange(250.0f)
    ,m_mapWidth(1333.0f)
    ,m_mapHeight(750.0f)
    ,m_poolCount(1)
{
}

void Molotov::attack(Player* player,
                     const std::vector<GameObject*>& enemies,
                     std::vector<std::unique_ptr<GameObject>>& projectiles)
{
    Vec2 playerPos = player->getPosition();

    for(int i=0;i<m_poolCount;i++)
    {
        float angle=QRandomGenerator::global()->bounded(2.0f*M_PI);
        float distance=QRandomGenerator::global()->bounded(m_throwRange);
        float targetX=playerPos.x+std::cos(angle)*distance;
        float targetY=playerPos.y+std::sin(angle)*distance;

        //限制在地图内
        targetX=std::max(0.0f, std::min(m_mapWidth,targetX));
        targetY=std::max(0.0f, std::min(m_mapHeight,targetY));
        Vec2 targetPos(targetX,targetY);

        auto pool=std::make_unique<Pool>(
            targetPos,
            static_cast<int>(m_damage),
            m_poolRadius,
            m_poolDuration
        );

        projectiles.push_back(std::move(pool));
    }
}

void Molotov::upgrade()
{
    if(isMaxLevel()) return;
    m_level++;
    m_poolCount+=1;
    if(m_level<=3) m_poolRadius+=10;
}



