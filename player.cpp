#include "player.h"
#include"spritemanager.h"
#include <QPainter>
#include <QtMath>
#include <QString>
#include <algorithm>//std::shuffle
#include <random>

Player::Player(const Vec2& pos)
    : GameObject(ObjectType::Player, pos)
{
    m_speed = 200.0f;
    m_health = 100;
    m_maxHealth = 100;
    m_collisionRadius = 15.0f;
    
    // 添加基础武器
    m_weapons.push_back(std::make_unique<No10Commando>());
    m_weapons.push_back(std::make_unique<NightRaid>());
    m_weapons.push_back(std::make_unique<SmileBro>());
    m_weapons.push_back(std::make_unique<Molotov>());
}

void Player::update(float deltaTime)
{
    // 更新无敌时间
    if (m_invincibleTimer > 0) m_invincibleTimer -= deltaTime;
    
    // 应用速度到位置
    m_position = m_position + m_velocity * deltaTime;
}

void Player::render(QPainter& painter)
{
    // 无敌时闪烁效果
    if (isInvincible())
    {
        int alpha = static_cast<int>(128 + 127 * std::sin(m_invincibleTimer * 20));
        painter.setOpacity(alpha / 255.0);
    }
    
    // 绘制玩家
    QPixmap sprite = SpriteManager::instance().getSprite("player");
    sprite = sprite.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    float drawX = m_position.x - 32;
    float drawY = m_position.y - 32;
    painter.drawPixmap(QPointF(drawX, drawY), sprite);
    painter.setOpacity(1.0);
    
    // 绘制生命值条
    float healthPercent = static_cast<float>(m_health) / m_maxHealth;
    QRectF healthBar(m_position.x - 20, m_position.y - 40, 40, 5);
    painter.setBrush(Qt::darkGray);
    painter.setPen(Qt::NoPen);
    painter.drawRect(healthBar);
    painter.setBrush(healthPercent > 0.3 ? Qt::green : Qt::red);
    painter.drawRect(QRectF(healthBar.left(), healthBar.top(), 
                            healthBar.width() * healthPercent, healthBar.height()));
}

void Player::handleInput(const QSet<int>& keys, float deltaTime)
{
    Vec2 direction(0, 0);

    if (keys.contains(Qt::Key_W) || keys.contains(Qt::Key_Up)) direction.y -= 1;
    if (keys.contains(Qt::Key_S) || keys.contains(Qt::Key_Down)) direction.y += 1;
    if (keys.contains(Qt::Key_A) || keys.contains(Qt::Key_Left)) direction.x -= 1;
    if (keys.contains(Qt::Key_D) || keys.contains(Qt::Key_Right)) direction.x += 1;
    
    // 归一化方向并应用速度
    if (direction.length() > 0)
    {
        direction = direction.normalized();
        m_velocity = direction * m_speed;
    }
    else
    {
        m_velocity = Vec2(0, 0);
    }
}

void Player::addExperience(int amount)
{
    m_experience += amount;
    int expNeeded = getExperienceToNextLevel();
    
    while (m_experience >= expNeeded)
    {
        m_experience -= expNeeded;
        m_level++;
        expNeeded = getExperienceToNextLevel();
        emit levelUp(m_level);
    }
    
    emit experienceChanged(m_experience, expNeeded);
}

int Player::getExperienceToNextLevel() const
{
    int linear=10+m_level*5;
    int square=10+m_level*m_level*5;
    if(m_level<=10) return linear;
    else return square;
}

void Player::addWeapon(std::unique_ptr<Weapon> weapon)
{
    m_weapons.push_back(std::move(weapon));
}

void Player::updateWeapons(float deltaTime, const std::vector<GameObject*>& enemies,
                           std::vector<std::unique_ptr<GameObject>>& projectiles)
{
    for (auto& weapon : m_weapons)
    {
        weapon->update(deltaTime, this, enemies, projectiles);
    }
}

void Player::heal(int amount)//治愈
{
    m_health = std::min(m_health + amount, m_maxHealth);
    emit healthChanged(m_health, m_maxHealth);
}

void Player::setInvincible(float duration)
{
    m_invincibleTimer = duration;
}

// ----------武器升级----------
std::vector<UpgradeOption> Player::getUpgradeOptions() const
{
    std::vector<UpgradeOption> options;

    // 1. 收集可升级的已有武器
    for (const auto& w : m_weapons)
    {
        if (!w->isMaxLevel())
        {
            options.push_back(UpgradeOption::CreateUpgrade(w.get()));
        }
    }

    // 收集未获得的武器
    QSet<QString> ownedNames;
    for (const auto& w : m_weapons) ownedNames.insert(QString::fromStdString(w->getName()));

    if (!ownedNames.contains(QStringLiteral("突击队突击")))
    {
        options.push_back(UpgradeOption::CreateNew(
            "突击队突击", "向四周发射一群较低伤害的第二突击队"));
    }
    if (!ownedNames.contains(QStringLiteral("莫洛托夫鸡尾酒")))
    {
        options.push_back(UpgradeOption::CreateNew(
            "莫洛托夫鸡尾酒", "地上随机出现莫洛托夫鸡尾酒，造成范围伤害，回合结束时弃掉"));
    }
    if (!ownedNames.contains(QStringLiteral("微笑哥和他的大运")))
    {
        options.push_back(UpgradeOption::CreateNew(
            "微笑哥和他的大运", "随机生成微笑哥，消失后生出一辆IS-2"));
    }

    // 随机打乱并抽取3个
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(options.begin(),options.end(),g);
    options.resize(3);

    return options;
}
