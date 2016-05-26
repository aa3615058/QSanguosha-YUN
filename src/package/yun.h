#ifndef YUN_H
#define YUN_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "standard.h"

class YunCardPackage : public Package
{
    Q_OBJECT

public:
    YunCardPackage();
};

class LureTiger : public TrickCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LureTiger(Card::Suit suit, int number);

    virtual QString getSubtype() const;

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class YunPackage : public Package
{
    Q_OBJECT

public:
    YunPackage();
};

class QiaopoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QiaopoCard();

    void onEffect(const CardEffectStruct &effect) const;
};

class XingcanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE XingcanCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class MiyuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MiyuCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class QifengCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE QifengCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class YunEXPackage : public Package
{
    Q_OBJECT

public:
    YunEXPackage();
};

class LienvCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LienvCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    void onEffect(const CardEffectStruct &effect) const;

};

class DuanyanCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE DuanyanCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class YigeCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE YigeCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

#endif // YUN_H

