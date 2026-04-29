#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <QPointF>
#include <QPainter>
#include <QRectF>
#include <set>

//2D向量结构体
struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;
    
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2(const QPointF& p) : x(static_cast<float>(p.x())), y(static_cast<float>(p.y())) {}

    Vec2 operator+(const Vec2& other) const 
    {
        return Vec2(x + other.x, y + other.y);
    }

    Vec2 operator-(const Vec2& other) const 
    {
        return Vec2(x - other.x, y - other.y);
    }
    
    // 标量乘法
    Vec2 operator*(float scalar) const 
    {
        return Vec2(x * scalar, y * scalar);
    }
    
    // 向量长度
    float length() const 
    {
        return std::sqrt(x * x + y * y);
    }
    
    // 归一化向量
    Vec2 normalized() const 
    {
        float len = length();
        if (len > 0.0001f) 
        {
            return Vec2(x / len, y / len);
        }
        return Vec2(0, 0);
    }
    
    // 转换为QPointF
    QPointF toPointF() const 
    {
        return QPointF(static_cast<qreal>(x), static_cast<qreal>(y));
    }
    
    // 计算向量间距离
    float distanceTo(const Vec2& other) const 
    {
        return (*this - other).length();
    }
    
    // 计算到目标的角度（弧度）
    float angleTo(const Vec2& target) const {
        return std::atan2(target.y - y, target.x - x);
    }
};

//游戏对象基类
//所有游戏实体（玩家、敌人、子弹等）的基类
class GameObject
{
public:
    enum class ObjectType
    {
        Player,
        Enemy,
        Projectile,
        ExperienceGem,
        Effect
    };
    
    GameObject(ObjectType type, const Vec2& pos = Vec2(0, 0));
    virtual ~GameObject() = default;
    
    // 核心虚函数 - 子类必须实现
    virtual void update(float deltaTime) = 0;
    virtual void render(QPainter& painter) = 0;
    
    // 获取碰撞半径（用于圆形碰撞检测）
    virtual float getCollisionRadius() const { return m_collisionRadius; }

    // 获取碰撞矩形（用于矩形碰撞检测）
    virtual QRectF getBoundingRect() const;
    
    // 位置相关
    const Vec2& getPosition() const { return m_position; }
    void setPosition(const Vec2& pos) { m_position = pos; }
    
    // 速度相关
    const Vec2& getVelocity() const { return m_velocity; }
    void setVelocity(const Vec2& vel) { m_velocity = vel; }
    
    // 生命值相关
    int getHealth() const { return m_health; }
    void setHealth(int health) { m_health = health; }
    void takeDamage(int damage);
    bool isDead() const { return m_health <= 0; }
    
    // 对象类型
    ObjectType getObjectType() const { return m_type; }
    
    // 是否存活
    bool isAlive() const { return m_alive; }
    void setAlive(bool alive) { m_alive = alive; }
    
protected:
    ObjectType m_type;
    Vec2 m_position;
    Vec2 m_velocity;//速度
    float m_speed = 100.0f;
    float m_collisionRadius = 15.0f;
    int m_health = 100;
    int m_maxHealth = 100;
    bool m_alive = true;
};

//微笑哥方块
class BroBlock : public GameObject
{
public:
    BroBlock(const Vec2& pos, int damage, float waitTime,
             float mapWidth, float mapHeight);
    ~BroBlock() override = default;

    void update(float deltaTime) override;
    void render(QPainter& painter) override;

    // 设置移动方向（在大方块阶段开始时调用）
    void setDirection(const Vec2& dir) { m_direction = dir.normalized(); }

    // 获取伤害值
    int getDamage() const { return m_damage; }

    // 穿透次数
    void setPenetration(int count) { m_penetration = count; }
    void reducePenetration();
    int getPenetration() const { return m_penetration; }

    // 检查是否处于大方块阶段
    bool isBigPhase() const { return m_isBigPhase; }

private:
    // 进入大方块阶段
    void enterBigPhase();

    // 检查是否出地图
    bool isOutOfMap() const;

    int m_damage;
    int m_penetration;
    float m_waitTime;// 小方块等待时间
    float m_elapsedTime;// 已经过时间
    float m_bigSpeed;// 大方块移动速度
    Vec2 m_direction;// 移动方向
    bool m_isBigPhase;// 是否处于大方块阶段
    float m_mapWidth;
    float m_mapHeight;
    float m_rotation;// 旋转角度
    float m_pulsePhase;// 脉冲动画相位
};

//区域效果类
class Pool:public GameObject
{
public:
    Pool(const Vec2& pos,int damage,float radius,float duration);
    ~Pool() override=default;

    void update(float detlaTime) override;
    void render(QPainter& painter) override;

    int getDamage() const {return m_damage;}

    float getPoolRadius() const {return m_poolRadius;}

    bool isEnemyInPool(const Vec2& enemyPos,float enemyRadius) const;

    // 记录伤害过的敌人，防止同一帧重复伤害
    void clearDamagedEnemies() {m_damagedEnemies.clear();}
    bool isDamagedEnemy(GameObject* enemy) {return m_damagedEnemies.count(enemy)>0;}
    void markDamaged(GameObject* enemy) {m_damagedEnemies.insert(enemy);}

private:
    int m_damage;
    float m_poolRadius;
    float m_duration;
    float m_elapsedTime;
    float m_damageTimer;// 伤害计时器
    float m_damageInterval;// 伤害间隔
    float m_flickerPhase;// 闪烁动画相位
    std::set<GameObject*> m_damagedEnemies;
};

#endif