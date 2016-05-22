#ifndef _BGM_H
#define _BGM_H

#include "package.h"
#include "card.h"
#include "standard.h"

class BGMPackage : public Package
{
    Q_OBJECT

public:
    BGMPackage();
};

class LihunCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE LihunCard();

    bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    void onEffect(const CardEffectStruct &effect) const;
};
#endif

