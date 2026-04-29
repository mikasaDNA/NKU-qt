#include "enemy.h"
#include "spritemanager.h"
#include <QPainter>
#include <QtMath>
#include <QRandomGenerator>
#include <QtMath>
#include <QRect>

Enemy::Enemy(EnemyType type, const Vec2& pos)
    : GameObject(ObjectType::Enemy, pos)
    , m_enemyType(type)
{
    // 设置属性
    switch (type) {
        case EnemyType::jiuliumi:
            m_speed = 80.0f;
            m_health = 20;
            m_maxHealth = 20;
            m_collisionRadius = 20.0f;
            m_expReward = 1;
            m_collisionDamage = 10;
            break;
            
        case EnemyType::xizhiye:
            m_speed = 150.0f;
            m_health = 10;
            m_maxHealth = 10;
            m_collisionRadius = 12.0f;
            m_expReward = 2;
            m_collisionDamage = 5;
            break;
            
        case EnemyType::lanbingtuan:
            m_speed = 40.0f;
            m_health = 100;
            m_maxHealth = 100;
            m_collisionRadius = 33.0f;
            m_expReward = 5;
            m_collisionDamage = 20;
            break;

        case EnemyType::bahongyiyu:
            m_speed = 40.0f;
            m_health=400.0f;
            m_maxHealth=400.0f;
            m_collisionRadius = 50.0f;
            m_expReward = 50;
            m_collisionDamage = 50;
            break;
    }
}

void Enemy::update(float deltaTime)
{
    if (!m_alive) return;
    
    // 计算朝向目标的方向
    Vec2 direction = m_target - m_position;
    float distance = direction.length();
    
    if (distance > 1.0f)
    {
        direction = direction.normalized();
        m_velocity = direction * m_speed;
        m_position = m_position + m_velocity * deltaTime;
    }
}

void Enemy::render(QPainter& painter)
{
    if (!m_alive) return;

    QPixmap jiuliumi = SpriteManager::instance().getSprite("jiuliumi");
    QPixmap xizhiye = SpriteManager::instance().getSprite("xizhiye");
    QPixmap lanbingtuan = SpriteManager::instance().getSprite("lanbingtuan");
    QPixmap bahongyiyu = SpriteManager::instance().getSprite("bahong");
    jiuliumi = jiuliumi.scaled(m_collisionRadius*3.0f, m_collisionRadius*3.0f, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    xizhiye = xizhiye.scaled(m_collisionRadius*3.0f, m_collisionRadius*3.0f, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    lanbingtuan = lanbingtuan.scaled(m_collisionRadius*3.0f, m_collisionRadius*3.0f, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    bahongyiyu = bahongyiyu.scaled(m_collisionRadius*3.0f, m_collisionRadius*3.0f, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    switch (m_enemyType)
    {
        case EnemyType::jiuliumi:
            painter.drawPixmap(m_position.toPointF().x()-m_collisionRadius*1.5f,
                               m_position.toPointF().y()-m_collisionRadius*1.5f,jiuliumi);
            break;
            
        case EnemyType::xizhiye:
            painter.drawPixmap(m_position.toPointF().x()-m_collisionRadius*1.5f,
                               m_position.toPointF().y()-m_collisionRadius*1.5f,xizhiye);
            break;
            
        case EnemyType::lanbingtuan:
            painter.drawPixmap(m_position.toPointF().x()-m_collisionRadius*1.5f,
                               m_position.toPointF().y()-m_collisionRadius*1.5f,lanbingtuan);
            break;
        case EnemyType::bahongyiyu:
            painter.drawPixmap(m_position.toPointF().x()-m_collisionRadius*1.5f,
                               m_position.toPointF().y()-m_collisionRadius*1.5f,bahongyiyu);
            break;
    }
    
    // 绘制生命值条（只有受伤时显示）
    if (m_health < m_maxHealth)
    {
        float healthPercent = static_cast<float>(m_health) / m_maxHealth;
        if(m_enemyType != EnemyType::bahongyiyu)
        {
            QRectF healthBar(m_position.x - 15, m_position.y - m_collisionRadius - 8, 30, 4);
            painter.setBrush(Qt::darkGray);
            painter.setPen(Qt::NoPen);
            painter.drawRect(healthBar);
            painter.setBrush(Qt::red);
            painter.drawRect(QRectF(healthBar.left(), healthBar.top(),
                                    healthBar.width() * healthPercent, healthBar.height()));
        }
        else
        {
            QRectF healthBar(0, 30, 900, 30);
            painter.setBrush(Qt::darkGray);
            painter.setPen(Qt::NoPen);
            painter.drawRect(healthBar);
            painter.setBrush(Qt::red);
            painter.drawRect(QRectF(healthBar.left(), healthBar.top(),
                                    healthBar.width() * healthPercent, healthBar.height()));
        }
    }
}

EnemySpawner::EnemySpawner()
{
}

void EnemySpawner::update(float deltaTime, const Vec2& playerPos,
                          std::vector<std::unique_ptr<Enemy>>& enemies)
{
    m_gameTime += deltaTime;

    // 更新难度（每分钟难度+1）
    m_difficulty = 1.0f + m_gameTime / 60.0f;

    // 更新生成间隔（随难度降低）
    m_spawnInterval = std::max(m_minSpawnInterval, 2.0f - m_difficulty * 0.2f);

    m_spawnTimer += deltaTime;

    // 检查是否需要生成敌人
    if (m_spawnTimer >= m_spawnInterval && enemies.size() < static_cast<size_t>(m_maxEnemies))
    {
        m_spawnTimer = 0.0f;

        // 随难度增加，每次可能生成多个敌人
        int spawnCount = static_cast<int>(1 + m_difficulty * 0.5f);
        for (int i = 0; i < spawnCount; i++) spawnEnemy(playerPos, enemies);
    }
}

void EnemySpawner::spawnEnemy(const Vec2& playerPos,
                              std::vector<std::unique_ptr<Enemy>>& enemies)
{
    // 在玩家周围随机角度生成
    float angle = QRandomGenerator::global()->bounded(2.0 * M_PI);
    float distance = m_spawnDistance + QRandomGenerator::global()->bounded(200.0);

    Vec2 spawnPos(
        playerPos.x + static_cast<float>(std::cos(angle) * distance),
        playerPos.y + static_cast<float>(std::sin(angle) * distance)
        );

    EnemyType type = selectEnemyType();
    auto enemy = std::make_unique<Enemy>(type, spawnPos);
    enemy->setTarget(playerPos);

    enemies.push_back(std::move(enemy));
}

EnemyType EnemySpawner::selectEnemyType()
{
    // 根据难度和随机值选择敌人类型
    double roll = QRandomGenerator::global()->bounded(1.0);

    // 难度越高，出现高级敌人的概率越大
    double fastChance = std::min(0.3, m_difficulty * 0.05);
    double tankChance = std::min(0.15, (m_difficulty - 2) * 0.03);

    if (roll < tankChance && m_difficulty > 2) {
        return EnemyType::lanbingtuan;
    } else if (roll < tankChance + fastChance) {
        return EnemyType::xizhiye;
    } else {
        return EnemyType::jiuliumi;
    }
}
