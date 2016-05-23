#ifndef OL_PACKAGE_H
#define OL_PACKAGE_H

#include "package.h"
#include "card.h"

class OLPackage : public Package
{
    Q_OBJECT

public:
    OLPackage();
};


class AocaiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE AocaiCard();

    bool targetFixed() const;
    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    const Card *validateInResponse(ServerPlayer *user) const;
    const Card *validate(CardUseStruct &cardUse) const;
};

class DuwuCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE DuwuCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};

class BushiCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE BushiCard();
    void onUse(Room *, const CardUseStruct &card_use) const;
};

class MidaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MidaoCard();

    void onUse(Room *, const CardUseStruct &card_use) const;
};

#endif
