#include "gamewidget.h"
#include "projectile.h"
#include "spritemanager.h"
#include <QPainter>//画笔
#include <QKeyEvent>//键盘事件
#include <QMouseEvent>//鼠标事件
#include <QRandomGenerator>//随机数
#include <QFont>//字体
#include <QUrl>
#include <QSoundEffect>
#include <QMediaPlayer>

GameWidget::GameWidget(QWidget* parent)
    : QWidget(parent)
{
    // 设置窗口大小
    setFixedSize(m_mapWidth, m_mapHeight);
    setFocusPolicy(Qt::StrongFocus);
    
    // 初始化游戏定时器（60 FPS）
    m_gameTimer = new QTimer(this);
    connect(m_gameTimer, &QTimer::timeout, this, &GameWidget::gameLoop);
    
    // 初始化开始界面背景轮播定时器
    m_menuBgTimer = new QTimer(this);
    connect(m_menuBgTimer, &QTimer::timeout, this, &GameWidget::updateMenuBackground);
    
    // 初始化开始界面
    initializeMenu();
    
    // 设置按钮区域
    m_startButtonRect = QRect(500, 400, 333, 50);
    m_settingsButtonRect = QRect(500, 480, 333, 50);

    setWindowIcon(QIcon(":/sprites/zhazhashi.ico"));
    QPixmap Cursorsprite = SpriteManager::instance().getSprite("kuotu");
    Cursorsprite = Cursorsprite.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    setCursor(QCursor(Cursorsprite));
}

void GameWidget::initializeMenu()
{
    // 加载开始界面背景图片
    m_menuBackgrounds.clear();
    for (int i = 0; i < 5; i++)
    {
        QString path = QString(":/sprites/menu_bg_%1.png").arg(i);
        QPixmap bg(path);
        bg = bg.scaled(width(), height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_menuBackgrounds.push_back(bg);
    }
    
    m_menuBgm = new QMediaPlayer(this);
    m_menuAudioOutput = new QAudioOutput(this);
    m_menuAudioOutput->setVolume(0.3f);
    m_menuBgm->setAudioOutput(m_menuAudioOutput);
    m_menuBgm->setSource(QUrl("qrc:/sounds/menu_bgm.mp3"));
    m_menuBgm->setLoops(QMediaPlayer::Infinite);
    m_menuBgm->play();

    m_currentMenuBgIndex = 0;
    
    // 启动背景轮播定时器（每3秒切换）
    m_menuBgTimer->start(3000);

    //加载武器图鉴
    m_weaponCatalog.clear();
    m_weaponCatalog.push_back(std::make_unique<No10Commando>());
    m_weaponCatalog.push_back(std::make_unique<NightRaid>());
    m_weaponCatalog.push_back(std::make_unique<SmileBro>());
    m_weaponCatalog.push_back(std::make_unique<Molotov>());
}

void GameWidget::updateMenuBackground()
{
    if (!m_menuBackgrounds.empty())
    {
        m_currentMenuBgIndex = (m_currentMenuBgIndex + 1) % m_menuBackgrounds.size();
        update();  // 触发重绘
    }
}

void GameWidget::initializeGame()
{
    m_menuBgm->stop();

    QStringList availableBgs = {"bg_0", "bg_1", "bg_2", "bg_3", "bg_4"};
    QStringList availableBgms = {"bgm_0", "bgm_1", "bgm_2", "bgm_3", "bgm_4", "bgm_5"};
    
    int randomBgIndex = QRandomGenerator::global()->bounded(availableBgs.size());
    QString selectedBg = availableBgs[randomBgIndex];
    m_currentBackground = SpriteManager::instance().getSprite(selectedBg);
    m_currentBackground = m_currentBackground.scaled(m_tileSize, m_tileSize,Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    int randomBgmIndex = QRandomGenerator::global()->bounded(availableBgms.size());
    QString selectedBgm = availableBgms[randomBgmIndex];
    m_currentBgm = new QMediaPlayer(this);
    m_bgmAudioOutPut = new QAudioOutput(this);
    m_bgmAudioOutPut->setVolume(0.1f);
    m_currentBgm->setAudioOutput(m_bgmAudioOutPut);
    m_currentBgm->setSource(QUrl(QString("qrc:/sounds/%1.ogg").arg(selectedBgm)));
    m_currentBgm->setLoops(QMediaPlayer::Infinite);
    m_currentBgm->play();

    // 在地图中心创建玩家
    float mapCenterX = m_mapWidth / 2.0f;
    float mapCenterY = m_mapHeight / 2.0f;
    m_player = std::make_unique<Player>(Vec2(mapCenterX, mapCenterY));
    
    connect(m_player.get(), &Player::levelUp, this, [this](int level) 
        {
        Q_UNUSED(level);
        if(level%2 == 0)
        {
            m_currentUpgradeOptions = m_player->getUpgradeOptions();
            if(!m_currentUpgradeOptions.empty())
            {
                m_state = GameState::LevelUp;
                m_levelUp = true;
                QSoundEffect* m_levelUpBgm = new QSoundEffect(this);
                m_levelUpBgm->setVolume(0.3f);
                m_levelUpBgm->setSource(QUrl(QString("qrc:/sounds/kardsjjc.wav")));
                m_levelUpBgm->play();
            }
        }
    });
    
    // 创建敌人生成器
    m_spawner = std::make_unique<EnemySpawner>();
    
    // 清空实体列表
    m_enemies.clear();
    m_projectiles.clear();
    m_gems.clear();
    
    // 重置游戏状态
    m_gameTime = 0.0f;
    m_score = 0;
    m_gameOver = false;
    m_paused = false;
    m_levelUp = false;
    m_state = GameState::Playing;
    m_randomNum = QRandomGenerator::global()->bounded(5);
    
    m_elapsedTimer.restart();
}

void GameWidget::gameLoop()
{
    float deltaTime = m_elapsedTimer.elapsed() / 1000.0f;
    m_elapsedTimer.restart();
    
    // 限制deltaTime防止跳帧
    deltaTime = std::min(deltaTime, 0.1f);
    
    if (m_state == GameState::Playing && !m_paused)
    {
        updateGame(deltaTime);
    }
    
    update();
}

void GameWidget::updateGame(float deltaTime)
{
    m_menuBgm->stop();

    // 处理输入
    handleInput(deltaTime);
    
    // 限制玩家位置在地图边界内
    clampPlayerPosition();

    if (m_player)//更新玩家
    {
        m_player->update(deltaTime);
    }
    for (auto& enemy : m_enemies)//更新敌人
    {
        if (enemy && enemy->isAlive())
        {
            enemy->setTarget(m_player->getPosition());
            enemy->update(deltaTime);
        }
    }
    for (auto& proj : m_projectiles)//更新子弹
    {
        if (proj && proj->isAlive())
        {
            proj->update(deltaTime);
        }
    }
    for (auto& gem : m_gems)//更新经验宝石
    {
        if (gem && gem->isAlive())
        {
            gem->setTarget(m_player->getPosition());
            gem->update(deltaTime);
        }
    }
    if (m_player)//更新玩家武器
    {
        std::vector<GameObject*> enemyPtrs;
        for (auto& e : m_enemies)
        {
            if (e && e->isAlive()) enemyPtrs.push_back(e.get());
        }
        m_player->updateWeapons(deltaTime, enemyPtrs, m_projectiles);
    }
    
    //碰撞检测
    checkCollisions();
    
    //清理死亡实体
    cleanupDeadEntities();
    
    //生成经验宝石
    spawnGems();
    
    //生成敌人
    if (m_spawner && m_player) 
    {
        m_spawner->update(deltaTime, m_player->getPosition(), m_enemies);
    }

    m_gameTime += deltaTime;
}

void GameWidget::handleInput(float deltaTime)
{
    if (!m_player) return;
    m_player->handleInput(m_keysPressed, deltaTime);
}

void GameWidget::clampPlayerPosition()
{
    if (!m_player) return;
    
    Vec2 pos = m_player->getPosition();
    float radius = m_player->getCollisionRadius();

    //限制在边界内（考虑碰撞半径）
    pos.x = std::max(radius, std::min(m_mapWidth - radius, pos.x));
    pos.y = std::max(radius, std::min(m_mapHeight - radius, pos.y));
    
    m_player->setPosition(pos);
}

void GameWidget::checkCollisions()
{
    if (!m_player) return;
    
    Vec2 playerPos = m_player->getPosition();
    float playerRadius = m_player->getCollisionRadius();
    
    //子弹与敌人碰撞
    for (auto& proj : m_projectiles) 
    {
        //检查子弹
        Projectile* p = dynamic_cast<Projectile*>(proj.get());
        BroBlock* block = dynamic_cast<BroBlock*>(proj.get());
        Pool* pool=dynamic_cast<Pool*>(proj.get());
        if (!p&&!block&&!pool) continue;
        if(p)//子弹
        {
            for (auto& enemy : m_enemies)
            {
                if (!enemy || !enemy->isAlive()) continue;

                if (checkCircleCollision(p->getPosition(), p->getCollisionRadius(),
                                         enemy->getPosition(), enemy->getCollisionRadius())) {
                    enemy->takeDamage(p->getDamage());
                    p->reducePenetration();

                    if (enemy->isDead())
                    {
                        m_score += 10;
                        m_pendingGems.push_back({enemy->getPosition(), enemy->getExperienceReward()});
                    }
                    break;
                }
            }
        }
        if(block)//方块
        {
            if(block&&!block->isBigPhase()) continue;

            for(auto& enemy:m_enemies)
            {
                if (!enemy || !enemy->isAlive()) continue;

                if (checkCircleCollision(block->getPosition(), block->getCollisionRadius(),
                                         enemy->getPosition(), enemy->getCollisionRadius())) {
                    enemy->takeDamage(block->getDamage());
                    block->reducePenetration();

                    if (enemy->isDead())
                    {
                        m_score += 10;
                        m_pendingGems.push_back({enemy->getPosition(), enemy->getExperienceReward()});
                    }
                    break;
                }
            }
        }
        if(pool)//区域
        {
            for(auto& enemy:m_enemies)
            {
                if(!enemy || !enemy->isAlive()) continue;

                if(pool->isEnemyInPool(enemy->getPosition(),enemy->getCollisionRadius()))
                {
                    if(!pool->isDamagedEnemy(enemy.get()))
                    {
                        enemy->takeDamage(pool->getDamage());
                        pool->markDamaged(enemy.get());

                        if (enemy->isDead())
                        {
                            m_score += 10;
                            m_pendingGems.push_back({enemy->getPosition(), enemy->getExperienceReward()});
                        }
                        break;
                    }
                }
            }
        }
    }
    
    // 玩家与敌人碰撞
    if (!m_player->isInvincible()) 
    {
        for (auto& enemy : m_enemies)
        {
            if (!enemy || !enemy->isAlive()) continue;
            
            if (checkCircleCollision(playerPos, playerRadius,
                                     enemy->getPosition(), enemy->getCollisionRadius())) {
                m_player->takeDamage(enemy->getCollisionDamage());
                m_player->setInvincible(0.5f);// 0.5秒无敌时间
                
                if (m_player->isDead())
                {
                    m_gameOver = true;
                    m_state = GameState::GameOver;
                }
                break;
            }
        }
    }
    
    // 玩家与经验宝石碰撞
    for (auto& gem : m_gems) 
    {
        if (checkCircleCollision(playerPos, playerRadius,
                                 gem->getPosition(), gem->getCollisionRadius())) 
        {
            m_player->addExperience(gem->getExperienceValue());
            m_player->heal(gem->getHealValue());
            gem->setAlive(false);
        }
    }
}

void GameWidget::cleanupDeadEntities()
{
    // 清理死亡的敌人
    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const std::unique_ptr<Enemy>& e) { return !e || !e->isAlive(); }),
        m_enemies.end()
    );
    
    // 清理死亡的子弹
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [](const std::unique_ptr<GameObject>& p) { return !p || !p->isAlive(); }),
        m_projectiles.end()
    );
    
    // 清理已收集的经验宝石
    m_gems.erase(
        std::remove_if(m_gems.begin(), m_gems.end(),
            [](const std::unique_ptr<ExperienceGem>& g) { return !g || !g->isAlive(); }),
        m_gems.end()
    );
}

void GameWidget::spawnGems()
{
    for (auto& [pos, value] : m_pendingGems)
    {
        auto gem = std::make_unique<ExperienceGem>(pos, value);
        gem->setTarget(m_player->getPosition());
        m_gems.push_back(std::move(gem));
    }
    m_pendingGems.clear();
}

void GameWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    switch (m_state)
    {
        case GameState::MainMenu:
            renderMainMenu(painter);
            break;
        case GameState::Playing:
            renderGame(painter);
            break;
        case GameState::LevelUp:
            renderGame(painter);
            renderLevelUp(painter);
            break;
        case GameState::GameOver:
            renderGame(painter);
            renderGameOver(painter);
            break;
        case GameState::Settings:
            renderSettings(painter);
        default:
            break;
    }
}

void GameWidget::renderMainMenu(QPainter& painter)
{
    // 绘制背景
    painter.drawPixmap(0, 0, m_menuBackgrounds[m_currentMenuBgIndex]);
    
    // 绘制游戏名称
    painter.setPen(Qt::white);
    painter.setFont(QFont("GOUDY STOUT", 40, QFont::Bold));
    painter.drawText(QRect(0, 100, width(), 100), Qt::AlignCenter, "Zhazhashi Survivor");
    
    painter.setFont(QFont("FZShuTi", 20));
    painter.drawText(QRect(0, 180, width(), 40), Qt::AlignCenter, "渣渣帅，吴迪。？！");
    
    // 绘制"开始游戏"按钮
    painter.setBrush(QColor(80, 150, 80));
    painter.setPen(QPen(QColor(100, 200, 100), 2));
    painter.drawRoundedRect(m_startButtonRect, 10, 10);
    painter.setPen(Qt::yellow);
    painter.setFont(QFont("Arial", 18, QFont::Bold));
    painter.drawText(m_startButtonRect, Qt::AlignCenter, "开始游戏");
    
    // 绘制"图鉴"按钮
    painter.setBrush(QColor(80, 80, 150));
    painter.setPen(QPen(QColor(100, 100, 200), 2));
    painter.drawRoundedRect(m_settingsButtonRect, 10, 10);
    painter.setPen(Qt::yellow);
    painter.drawText(m_settingsButtonRect, Qt::AlignCenter, "图鉴");

    // 绘制提示
    painter.setPen(Qt::gray);
    painter.setFont(QFont("Arial", 12));
    painter.drawText(QRect(0, 560, width(), 30), Qt::AlignCenter, "每一张卡牌众生平等，扎扎师是个例外");
 }

void GameWidget::renderGame(QPainter& painter)
{
    renderBackground(painter);// 渲染背景

    for (auto& gem : m_gems)// 渲染经验宝石
    {
        if (gem && gem->isAlive())
        {
            gem->render(painter);
        }
    }

    for (auto& enemy : m_enemies)// 渲染敌人
    {
        if (enemy && enemy->isAlive())
        {
            enemy->render(painter);
        }
    }

    if (m_player)// 渲染玩家
    {
        m_player->render(painter);
    }

    renderHUD(painter);// 渲染HUD

    for (auto& proj : m_projectiles)// 渲染子弹
    {
        if (proj && proj->isAlive())
        {
            proj->render(painter);
        }
    }

    if (m_paused) // 渲染暂停界面
    {
        painter.fillRect(rect(), QColor(0, 0, 0, 150));
        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 36, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, "暂停");
        painter.setFont(QFont("Arial", 16));
        painter.drawText(rect().adjusted(0, 100, 0, 0), Qt::AlignCenter, "按 ESC 继续");
        painter.setPen(Qt::gray);
        painter.setFont(QFont("Arial", 10));
        QString m_tips[6];
        m_tips[0] = "沙漠之狐向你致敬";
        m_tips[1] = "天佑吾王！";
        m_tips[2] = "你可以跑，但无处藏身";
        m_tips[3] = "众人之上，强者之列";
        m_tips[4] = "你的功绩永世长存";
        m_tips[5] = "沙场秋点兵";
        painter.drawText(rect().adjusted(0, 600, 0, 0), Qt::AlignCenter, m_tips[m_randomNum]);
    }
}

void GameWidget::renderBackground(QPainter& painter)
{
    Vec2 offset(0.0f,0.0f);

    // 计算可见的瓦片范围
    int endTileX = static_cast<int>(width() / m_tileSize) + 1;
    int endTileY = static_cast<int>(height() / m_tileSize) + 1;

    // 限制在地图范围内
    endTileX = std::min(m_mapGridWidth, endTileX);
    endTileY = std::min(m_mapGridHeight, endTileY);

    // 绘制背景瓦片
    for (int y = 0; y < endTileY; y++)
    {
        for (int x = 0; x < endTileX; x++)
        {
            float screenX = x * m_tileSize;
            float screenY = y * m_tileSize;
            painter.drawPixmap(static_cast<int>(screenX), static_cast<int>(screenY), m_currentBackground);
        }
    }
    
    // 绘制地图边界指示
    painter.setPen(QPen(QColor(255, 0, 0, 100), 4));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(0, 0, 0, m_mapHeight);//左边界
    painter.drawLine(m_mapWidth, 0, m_mapWidth, m_mapHeight);//右边界
    painter.drawLine(0, 0, m_mapWidth, 0);//上边界
    painter.drawLine(0, m_mapHeight, m_mapWidth, m_mapHeight);//下边界
}

void GameWidget::renderLevelUp(QPainter& painter)
{
    // 半透明黑色背景
    painter.fillRect(rect(), QColor(0, 0, 0, 180));

    // 标题
    painter.setPen(Qt::yellow);
    painter.setFont(QFont("Arial", 36, QFont::Bold));
    painter.drawText(QRect(0, 50, width(), 60), Qt::AlignCenter, "Choose Choose Below");

    // 清空按钮区域
    m_upgradeButtomRects.clear();

    // 计算卡片布局
    int cardWidth = 300;
    int cardHeight = 420;
    int cardSpacing = 40;
    int totalWidth = m_currentUpgradeOptions.size() * cardWidth + (m_currentUpgradeOptions.size() - 1) * cardSpacing;
    int startX = (width() - totalWidth) / 2;
    int startY = 150;

    for (size_t i = 0; i < m_currentUpgradeOptions.size(); i++) {
        const auto& option = m_currentUpgradeOptions[i];
        int cardX = startX + i * (cardWidth + cardSpacing);

        // 保存按钮区域
        m_upgradeButtomRects.push_back(QRect(cardX, startY, cardWidth, cardHeight));

        QPixmap sprite = (option.type == UpgradeOption::Type::UpgradeExisting)
                                          ? SpriteManager::instance().getSprite("upgradeexisting") :
                                            SpriteManager::instance().getSprite("addnewweapon");
        painter.drawPixmap(cardX, startY, cardWidth, cardHeight, sprite);

        // 绘制类型标签
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        QString typeText = (option.type == UpgradeOption::Type::UpgradeExisting)
                               ? QString("升级 Lv.%1 → Lv.%2").arg(option.currentLevel).arg(option.currentLevel + 1)
                               : QString("新武器");
        painter.setPen(Qt::white);
        painter.drawText(QRect(cardX, startY + cardHeight + 15, cardWidth, 30), Qt::AlignCenter, typeText);

        // 绘制武器名称
        painter.setFont(QFont("黑体", 20, QFont::Bold));
        painter.setPen(Qt::black);
        painter.drawText(QRect(cardX, startY + 297, cardWidth, 40), Qt::AlignCenter, option.weaponName);

        // 绘制描述
        painter.setFont(QFont("黑体", 14));
        painter.setPen(Qt::black);
        // 自动换行绘制描述
        QRect descRect(cardX + 20, startY + 340, cardWidth - 40, 150);
        painter.drawText(descRect, Qt::AlignTop | Qt::TextWordWrap, option.description);
    }

    // 底部提示
    painter.setPen(QColor(150, 150, 150));
    painter.setFont(QFont("Arial", 18));
    QString m_tips[6];
    m_tips[0] = "一步也不许后退";
    m_tips[1] = "如此少数英雄竟能拯救芸芸众生";
    m_tips[2] = "再累也不能倒下";
    m_tips[3] = "让他们继续飞翔";
    m_tips[4] = "为何拼杀，为何赴死？";
    m_tips[5] = "胜利的果实在变质";
    painter.drawText(QRect(0, m_mapWidth, m_mapHeight - 100, 30), Qt::AlignCenter,m_tips[m_randomNum]);
}

void GameWidget::renderHUD(QPainter& painter)
{
    // 左上角：生命值、经验值、等级
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 14));
    
    // 生命值
    if (m_player)
    {
        painter.drawText(10, 25, QString("HP: %1/%2").arg(m_player->getHealth()).arg(m_player->getMaxHealth()));
        painter.drawText(10, 45, QString("LV: %1").arg(m_player->getLevel()));
        painter.drawText(10, 65, QString("EXP: %1/%2").arg(m_player->getExperience()).arg(m_player->getExperienceToNextLevel()));
    }
    
    // 右上角：分数、时间
    painter.drawText(width() - 150, 25, QString("分数: %1").arg(m_score));
    painter.drawText(width() - 150, 45, QString("时间: %1s").arg(static_cast<int>(m_gameTime)));
}

void GameWidget::renderSettings(QPainter& painter)
{
    // 绘制背景:家里得请高人了
    QPixmap settingBg = SpriteManager::instance().getSprite("settings_bg");
    settingBg = settingBg.scaled(m_mapWidth, m_mapHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter.drawPixmap(rect(), settingBg);
    painter.fillRect(rect(), QColor(0, 0, 0, 180));

    painter.setPen(Qt::white);
    painter.setFont(QFont("STCaiyun", 36, QFont::Bold));
    painter.drawText(QRect(0, 20, m_mapWidth, 60), Qt::AlignCenter, "卡牌图鉴");
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(QRect(600, 20, m_mapWidth, 60), Qt::AlignCenter, "按ESC返回主页");

    // 卡片布局
    int cardWidth = 250;
    int cardHeight = 350;
    int cardSpacingX = 30;
    int cardSpacingY = 20;
    int cardsPerRow = 4;//每行放四个
    int startX = 80;
    int startY = 100;

    // 绘制武器卡片
    for (size_t i = 0; i < m_weaponCatalog.size(); i++)
    {
        int row = static_cast<int>(i) / cardsPerRow;
        int col = static_cast<int>(i) % cardsPerRow;
        int cardX = startX + col * (cardWidth + cardSpacingX);
        int cardY = startY + row * (cardHeight + cardSpacingY);

        const auto& weapon = m_weaponCatalog[i];

        // 绘制卡片背景，坏人多多
        QPixmap sprite = SpriteManager::instance().getSprite("catalog");
        sprite=sprite.scaled(cardWidth,cardHeight,Qt::KeepAspectRatio,Qt::SmoothTransformation);
        painter.drawPixmap(cardX, cardY, sprite);

        // 绘制武器名称及属性
        painter.setPen(Qt::black);
        painter.setFont(QFont("黑体", 18, QFont::Bold));
        painter.drawText(QRect(cardX + 10, cardY + 250, cardWidth - 20, 30), Qt::AlignCenter,
                         QString::fromStdString(weapon->getName()));
        painter.setFont(QFont("黑体", 14));
        QString attrText = QString("基础伤害: %1  冷却: %2秒").arg(weapon->getDamage()).arg(weapon->getCooldown());
        painter.drawText(QRect(cardX + 10, cardY + 280, cardWidth - 20, 25), Qt::AlignCenter, attrText);
    }
}

void GameWidget::renderGameOver(QPainter& painter)
{
    painter.fillRect(rect(), QColor(0, 0, 0, 180));
    
    painter.setPen(Qt::red);
    painter.setFont(QFont("Arial", 50, QFont::Bold));
    painter.drawText(QRect(0, 300, 1333, 50), Qt::AlignCenter, "GAME OVER");
    
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 20));
    QString stats = QString("分数: %1\n时间: %2s\n等级: %3")
        .arg(m_score)
        .arg(static_cast<int>(m_gameTime))
        .arg(m_player ? m_player->getLevel() : 1);
    painter.drawText(rect().adjusted(0, 80, 0, 0), Qt::AlignCenter, stats);
    
    painter.setFont(QFont("Arial", 16));
    painter.drawText(rect().adjusted(0, 400, 0, 0), Qt::AlignCenter, "按 R 重新开始");
    painter.drawText(rect().adjusted(0, 450, 0, 0), Qt::AlignCenter, "按 ESC 返回主菜单");

    painter.setPen(Qt::gray);
    QString m_tips[6];
    m_tips[0] = "沙漠之狐向你致敬";
    m_tips[1] = "热血、辛苦、眼泪和汗水";
    m_tips[2] = "生有涯，义无涯";
    m_tips[3] = "我未曾死亡，只是悄然隐去";
    m_tips[4] = "望胜者实至名归";
    m_tips[5] = "沙场秋点兵";
    painter.drawText(rect().adjusted(0, 650, 0, 0), Qt::AlignCenter, m_tips[m_randomNum]);
}

bool GameWidget::checkCircleCollision(const Vec2& pos1, float r1,
                                      const Vec2& pos2, float r2)
{
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    return distance < (r1 + r2);
}

void GameWidget::keyPressEvent(QKeyEvent* event)
{
    if (!event->isAutoRepeat()) m_keysPressed.insert(event->key());

    if (event->key() == Qt::Key_Escape)
    {
        if (m_state == GameState::Playing)
        {
            m_paused = !m_paused;
            m_randomNum=QRandomGenerator::global()->bounded(5);
        }
        if (m_state == GameState::GameOver || m_state == GameState::Settings)
        {
            m_state = GameState::MainMenu;
            initializeMenu();
        }
    }
    if (event->key() == Qt::Key_R && m_state == GameState::GameOver)
    {
        initializeGame();
    }
    QWidget::keyPressEvent(event);
}

void GameWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (!event->isAutoRepeat()) m_keysPressed.remove(event->key());
    QWidget::keyReleaseEvent(event);
}

void GameWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_state == GameState::MainMenu)
    {
        QPoint pos = event->pos();
        if (m_startButtonRect.contains(pos))
        {
            m_menuBgTimer->stop();
            initializeGame();
            m_gameTimer->start(16);
        }
        else if (m_settingsButtonRect.contains(pos))
        {
            m_state=GameState::Settings;
        }
    }
    else if(m_state == GameState::LevelUp)
    {
        QPoint pos = event->pos();
        for (size_t i = 0; i < m_upgradeButtomRects.size() && i < m_currentUpgradeOptions.size(); i++)
        {
            if (m_upgradeButtomRects[i].contains(pos))
            {
                applyUpgrade(m_currentUpgradeOptions[i]);
                m_state = GameState::Playing;
                m_levelUp = false;
                m_currentUpgradeOptions.clear();
                m_upgradeButtomRects.clear();
                break;
            }
        }
    }
    
    QWidget::mousePressEvent(event);
}

void GameWidget::applyUpgrade(const UpgradeOption& option)
{
    if (!m_player) return;

    if (option.type == UpgradeOption::Type::UpgradeExisting)
    {
        // 升级已有武器
        if (option.targetWeapon)
        {
            option.targetWeapon->upgrade();
        }
    }
    else if (option.type == UpgradeOption::Type::AddNewWeapon)
    {
        // 添加新武器
        if (option.weaponName == "突击队突击")
        {
            m_player->addWeapon(std::make_unique<NightRaid>());
        }
        else if (option.weaponName == "微笑哥和他的大运")
        {
            m_player->addWeapon(std::make_unique<SmileBro>());
        }
        else if (option.weaponName=="莫洛托夫鸡尾酒")
        {
            m_player->addWeapon(std::make_unique<Molotov>());
        }
    }
}
